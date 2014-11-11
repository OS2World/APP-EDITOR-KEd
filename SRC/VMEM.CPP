/*****************************************************
File: VMEM.C        Copyright 1989 by Dlugosz Software
   virtual memory system for Editor Toolbox

   rev 3.5 !! August 7, 1994
   The ed now needs 4 pages to start up since I moved all the windowing
   functions to TOW. I add 2 more pages to the initial alloc to allow
   loading small files without additional allocations.

   eliminated the global frame[MAX_FRAMES] and made static to this file

*****************************************************/
#include "usual.h"
#include "dmm.h"
#include "vmem.h"
#include <stdlib.h>

#include "test.h"

const int MAX_FRAMES = 4096; //frames *vmem_pagesize = 32MB mod 10-21-93

struct vmem_struct { USHORT page, offset; };   // MOD 8-2-92
static union {
   struct vmem_struct vs;
   ULONG  vptr1;
};

static USHORT framecount= 6;               /* need this many to start up */
struct frame_t frame[MAX_FRAMES];       
static USHORT  next_frame= 1;         /* 0 reserved for AT[] functions */
static USHORT AT[MAX_FRAMES];        /* Allocation Table array */

#ifdef VMEM_INLINE
struct frame_t *myframe = Mem::frame;
#endif

/* **************************************** */
/*   system functions                       */
/* **************************************** */
/************************************************************************/
void *Mem::vmem_trans (vptr p_in)    
{ 
  vptr1 = p_in;
  return (byte *)*((ULONG *)&frame +vs.page)+ vs.offset; 

//   return (byte *)*(&frame +vs.page)+ vs.offset;  // doesn't work

//  struct frame_t *found;           // original radick routine
//  found = x.frame + LOUSHORT(p_in); 
//  return (byte *)found->contents + HIUSHORT(p_in);

}

/********************* **************************************** */

SHORT Mem::vmem_setup()                     /* initialize the vmem system */
{                                /* MALLOC here,  framecount x 8192 bytes */ 
register USHORT loop;

for (loop= 0; loop <= framecount; loop++) {
   frame[loop].contents = new char[VMEM_PAGESIZE]; 
   if (!frame[loop].contents) 
      break;
   }
DBG_VMEM(dout<<" vmem_setup allocated "<<framecount<<" pages "<<endl);
return framecount;
}

/* **************************************** */

void Mem::vmem_shutdown()
{
USHORT loop;
   for (loop= 0; loop < framecount; loop++) {
     delete[] frame[loop].contents;
   }
DBG_VMEM(dout<<" vmem_shutdown deleted "<<framecount<<" pages "<<endl);
}

/* ******************************************************************** */
/*   page functions                                                     */
/* ******************************************************************** */

vptr vmem_newpage ()       /* allocate a page of memory */
{                          /* returns pointer to start of page */
struct vmem_struct p = {0,0};    
                        
if (AT[0] != 0) {      /* AT[0] is a list of freed pages */
                       /* there is something on the freelist.reuse a page. */
   p.page= AT[0];      /* use the first page on the list */
   AT[0]= AT[p.page];  /* put next "space" or page on freelist */
   AT[p.page]= 0;      /* take this page off the freelist */
   DBG_VMEM(dout<<" vmem_newpage used freed page"<<endl);
}
else {                                         /* use a new page */ 
   if(frame[next_frame].contents != NULL) {  // is next_frame valid?
      p.page= next_frame;                      // use it
      DBG_VMEM(dout<<" vmem_newpage used existing page num ="<<next_frame<<endl);
   }
}
if (++next_frame == framecount) {     /* if we're maxed,create a new frame */
   if(next_frame == MAX_FRAMES -1) {
    //  my_assert (next_frame, "MAX_FRAMES exceeded", "vmem.c", 141);
     exit(23);
   }                                  // WARNING I don't check malloc
   frame[framecount].contents = new char[VMEM_PAGESIZE]; 
   DBG_VMEM(dout<<" vmem_newpage allocated new page framecount = "<<framecount<<endl);
   framecount++;
}
return *(vptr *)&p;
}

/* **************************************** */

void vmem_droppage (vptr v)
{                           /* return a page to the free pool */
struct vmem_struct p;

p.page   = LOUSHORT(v);
AT[p.page]= AT[0];
AT[0]= p.page;
DBG_VMEM(dout<<" vmem_droppage dropped page = "<<(p.page)<<endl);
}

/* **************************************** */
/*     high level allocation                */
/* **************************************** */

//static bool present(SHORT space,SHORT page)
//{                       /* is the page in that space? */
//while (space != 0) {
//   if (space == page) {   /* "walks" down the allocated list */
//       return TRUE;       // this execs much less
//   }
//   space= AT[space];       // this execs much more
//}
//return FALSE;
//}

/* **************************************** */

SHORT vmem_create_space()
{
USHORT page;
vptr v= vmem_newpage();       /* gets next free page from  AT[] list of  */
                              /* "freed" pages or creates new page       */
page = LOUSHORT(v);
                                /* the AT entry will already be 0 */
dmm_configure (VMEM_PAGESIZE/2, (USHORT *)frame[page].contents);
DBG_VMEM(dout<<" vmem_create_space on page = "<<page<<endl);
return page;
}

/* **************************************** */

void vmem_delete_space (SHORT space)
{
SHORT p= space;      /* only looks at AT[] "allocation table"  */

while (AT[p]) {    /* find the end of the list */
   p= AT[p];
}                  /* p is now the number of the last page, AT[p] is 0 */
AT[p]= AT[0];      /* append freelist */
AT[0]= space;      /* new freelist */
}

/* **************************************** */

void *vmem_alloc (SHORT space, USHORT size, vptr* v)
{                            /* allocate a block of memory */
register USHORT page;
dmm_offset ofs;
struct vmem_struct new_v;
vptr new_v_2;
size= (size+1)/2;                      /* convert bytes to integers */
SHORT myspace = space;

DBG_VMEM(dout<<" vmem_alloc  space = "<<space<<"  size = "<<size<<endl);

for (page= 0; page < framecount; page++) { /*First check pages already used */
//   if (present(space,page)) {           /* looks at AT[] list  */
     myspace = space;                      // inline present()
     while (myspace != 0) {
        if (myspace == page) {   /* "walks" down the allocated list */
          ofs= dmm_alloc ((USHORT *)frame[page].contents, size);
          if (ofs) 
            goto got_it;
        }
     myspace= AT[myspace];
     }
}
                        /* no room on any of them.  allocate a new page. */
new_v_2= vmem_newpage();
//new_v.offset = HIUSHORT(new_v_2);    // mod 10-26-93
new_v.page   = LOUSHORT(new_v_2);

page= new_v.page;
AT[page]= AT[space];                      /* add it to the space's list */
AT[space]= page;
                                                
dmm_configure (VMEM_PAGESIZE/2, (USHORT *)frame[page].contents); /* set up new page*/
ofs= dmm_alloc ((USHORT *)frame[page].contents, size);           /* use the page */

got_it:
vs.page = page;                 // should work......
vs.offset = 2 * ofs;

//((struct vmem_struct *)v)->page= page;
//((struct vmem_struct *)v)->offset= 2*ofs;      /* convert to byte offset */

*v = vptr1;

return (byte *)frame[page].contents + vs.offset;
}

/************************************************************************
   find which page is pointing to this one in the space.  If
   it is the head of a space, return itself.  That is the only
   possible flag value, since any other number is a valid result. 

   called only by vmem_free   
***********************************************************************/
static USHORT find_parent_page (USHORT page)
{
register USHORT slot;

for (slot= 0;  slot < framecount;  slot++) {
   if (AT[slot] == page) {
       return slot;
   }
}
return page;       // never gets here DR 1-16-93
}

/* **************************************** */

void vmem_free (vptr p_in)
{
struct vmem_struct p;
static struct vmem_struct base;
static USHORT* contents;             

if (!p_in) return;
p.offset = HIUSHORT(p_in);
p.page   = LOUSHORT(p_in);

base.page= p.page;

p_in = MAKEULONG(base.page, 0);

contents= (USHORT *)Mem::vmem_trans (p_in);  // get pointer to start of block
DBG_VMEM(dout<<"vmem_free base.page = "<<base.page<<" contents = "<<contents<<"  p.offset = "<<(p.offset)<<endl);

dmm_free (contents, p.offset/2);   // free the block      
                                              /* is this page empty */
if (contents[0] == 1 && contents[1] == (VMEM_PAGESIZE/2)-1) {
   /*  First word is the freelist, pointing to the following word. 
       First word in the block header is the size of the block in words.  
       This looks for the heap set up as one big free block.  */

                         /* page `base.page' can be removed from the space */
    USHORT parent_page = find_parent_page (base.page);
    if (parent_page != base.page) {
      AT[parent_page]= AT[base.page];    /* delete from list */
      if (parent_page != 0) {
         AT[base.page]= AT[0];           /* add it to head of freelist */
         AT[0]= p.page;                  /* new head of freelist */
         }
      }
   }
}

/* **************************************** */
/*     burst mode allocation                */
/* **************************************** */

static vptr burst_page_v;
static USHORT* burst_page;
static struct vmem_struct burst_v;
static SHORT burst_roomleft;
static SHORT burst_space;

/* **************************************** */

SHORT vmem_burst_start (SHORT space)
{
burst_page_v= vmem_newpage();    /* grabs 1st free page or creates new page */
burst_roomleft= VMEM_PAGESIZE-2;
burst_v.page   = LOUSHORT(burst_page_v);

burst_v.offset= 2;                 /* beginning of block after freepointer */
if (space == -1) 
    burst_space= burst_v.page;
else 
    burst_space= space;

return burst_space;
}

/* **************************************** */

void vmem_burst_end()
{
SHORT page;

   burst_page= (USHORT *)Mem::vmem_trans (burst_page_v);

if (burst_roomleft) {                     /* put last piece in freelist */
   USHORT offset= burst_v.offset/2;
   burst_page[offset]= burst_roomleft/2;
   burst_page[1+offset]= 0;             /* end of freelist */
   burst_page[0]= offset;
}
else burst_page[0]= 0;                  /* no freelist */
                                        /* link into space allocation */
page= burst_v.page;
if (page == burst_space) {              /* first page in the space */
                         /* AT[page]= 0;   not needed, entry is already 0 */
 }
else {
   AT[page]= AT[burst_space];
   AT[burst_space]= page;
 }
}

/* **************************************** */

void* vmem_burst_alloc (USHORT size, vptr *v)
{
void *retval;

size += 2;                              /* room for header */
if (size < 4) size= 4;
else size= (size+1)&~1;                 /* force to be even */
if (size > (USHORT)burst_roomleft) {
   vmem_burst_end();                    /* done with this page */
   vmem_burst_start (burst_space);      /* start a new one */
}                                        /* roomleft is now very large */
if (size+4 > (USHORT)burst_roomleft) 
    size= burst_roomleft;               /* take it all */
burst_roomleft -= size;
   burst_page= (USHORT *)Mem::vmem_trans (burst_page_v);
burst_page[burst_v.offset/2]= size/2;
burst_v.offset += 2;
retval= burst_page+burst_v.offset/2;
//*v= *(vptr*)&burst_v;

*v = MAKEULONG(burst_v.page,burst_v.offset);  // mod 8-18-92

burst_v.offset += (size-2);             /* in position for next call */
return retval;
}


