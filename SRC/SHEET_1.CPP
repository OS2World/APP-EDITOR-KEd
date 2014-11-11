/*****************************************************
file: SHEET.C       Copyright 1989 by Dlugosz Software
   manipulate the current sheet
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "setting.h" /* need printer filename for sheet_print() */
#include "mylib.h"   // mylib_write call in sheet_print
#include "view.h"   /* sheet calls view functions */
#include "misc.h"   // fillwords
#include <string.h>

#include "test.h"

#define HDR_SZ sizeof(struct line_header)
SHORT sheet_mem= 0;
static struct line_header empty_header= {0,0};
//static SHORT xbuf = 4; // extra line buf capacity // was 8
SHORT xbuf = 4;          // extra line buf capacity // was 8

//static struct sheet *cur;
struct sheet *cur;               // mod 9-10-95

vptr active_sheet= 0L;
static vptr all_sheets= 0L;
struct sheet *my_active_sheet;

/* **************************************** */
/*     line functions                       */
/* **************************************** */

vptr get_line_anysheet (struct sheet *s,ULONG n)
{
//SHORT tier= n/(TIER_SIZE), offset= n%(TIER_SIZE);
SHORT tier= n/(TIER_SIZE);
vptr *p;
p= (vptr *)Mem::vmem_trans (s->tiers[tier]);
return p[(n%(TIER_SIZE))];
}

/* **************************************** */

vptr get_line (ULONG n)
{
//cur= (struct sheet *)Mem::vmem_trans (active_sheet);  // mod 8-31-93
// return get_line_anysheet (cur, n); 
                             /* optimize 6-19-92  */
//SHORT tier= n/(TIER_SIZE), offset= n%(TIER_SIZE);
//vptr *p;
cur = my_active_sheet;        /* mod 6-19-91  */
//p= (vptr *)Mem::vmem_trans (cur->tiers[tier]);
vptr *p= (vptr *)Mem::vmem_trans (cur->tiers[n/(TIER_SIZE)]);
//return p[offset];
return p[(n%(TIER_SIZE))];
}

/* **************************************** */

void put_line_anysheet (struct sheet* s,ULONG n, vptr line)
{
//SHORT offset= n%(TIER_SIZE);        // mod 8-31-93
SHORT tier=   n/(TIER_SIZE);
//vptr *p;

if (!s->tiers[tier]) 
   s->tiers[tier]= vmem_newpage();
vptr *p= (vptr *)Mem::vmem_trans (s->tiers[tier]);
p[(SHORT)(n%(TIER_SIZE))]= line;
//p[offset]= line;
}

/*********************************** **************************************** 
        space for the line vptr is alloc'ed here

*************************************************************************/


void put_line (ULONG n, vptr line)      // n = line_num
{
//cur= (struct sheet *)Mem::vmem_trans(active_sheet);  // mod 8-31-93
//SHORT tier= n/(TIER_SIZE), offset= n%(TIER_SIZE);
SHORT tier=  n/(TIER_SIZE);
vptr *p;
cur = my_active_sheet;          
//put_line_anysheet (cur, n, line);

if (!cur->tiers[tier]) 
   cur->tiers[tier]= vmem_newpage();
p= (vptr *)Mem::vmem_trans (cur->tiers[tier]);
//p[offset] = line;
p[(n%(TIER_SIZE))]= line;
}

/********************************** **************************************** 
 append part of the vptr line passed to the destination sheet.  
 c1 is the first col, c2 is the first col not to include.  
 If c2 == -1, do 'til the end of the line.  

   vmem_alloc allocates space for the line string here
*********************************************************************/
void append_line_part (struct sheet *s, vptr line,SHORT c1,SHORT c2)
{
/* append part of the line to the sheet.  c1 is the first col, c2 is the first
   col not to include.  If c2 == -1, do 'til the end of the line.  */

struct line_header *l2, *l= (struct line_header *)Mem::vmem_trans (line);
vptr l2_v;
SHORT len;
#pragma intrinsic memcpy

if (c1 > l->length) len= 0;
else {
   if (c2 > l->length) c2= l->length;
   if (c2 >= 0) len= c2-c1;
   else len= l->length - c1;
   }
l2= (struct line_header *)vmem_alloc (s->space, len+sizeof(struct line_header), &l2_v);
l2->capacity= l2->length= len;
memcpy (1+l2, c1+(char*)(1+l), len);
                           /* the line part is duplicated.  Now append to s */
put_line_anysheet (s, s->line_count++, l2_v);
}
#pragma intrinsic -memcpy

/********************************** **************************************** 
 append all of the vptr line passed to the destination sheet.  
 c1 is the first col, c2 is the first col not to include.  
 If c2 == -1, do 'til the end of the line.  

   vmem_alloc allocates space for the line string here

  Special version, because regular Block_Copy was too slow for
  large (> 10,000 line) copies

  Block Stream or Line Copy always does startcol = 0, endcol = end of line

  c1 always 0, 
  c2 always -1
*********************************************************************/
void bulk_append_lines(struct sheet *dest, ULONG start, ULONG end)
{
struct line_header *l2;
struct line_header  *l;
vptr l2_v;
SHORT len;
#pragma intrinsic memcpy

dest->space= vmem_burst_start (dest->space);
while(start < end) {
   l = line_fetch(active_sheet, start);
   len = l->length;
   l2= (line_header *)vmem_burst_alloc(len+sizeof(line_header), &l2_v);
   l2->capacity= l2->length= len;
   memcpy (1+l2, (char*)(1+l), len);    // copy L1 to L2                        
   put_line_anysheet (dest, dest->line_count++, l2_v); // put into dest sheet
   start++;
}
vmem_burst_end();

}
#pragma intrinsic -memcpy


#define VPTR_COUNT (VMEM_PAGESIZE/sizeof(vptr))
//const VPTR_COUNT = (VMEM_PAGESIZE/sizeof(vptr)); // how many lines per page
                                  /* used by insert_lines and delete_lines */
/* ************************************************************************/

static void  insert_lines (ULONG row, USHORT count)
{            // fills dest sheet with count blank lines
#pragma intrinsic memcpy

ULONG new_linecount;

SHORT cur_tier;
vptr *src, *dest;
SHORT start_tier, end_tier, insertpage_count;
USHORT start_ofs, end_ofs, linecount;

cur= (struct sheet *)Mem::vmem_trans (active_sheet);
new_linecount= cur->line_count+count;

start_tier= row/VPTR_COUNT;
start_ofs= row%VPTR_COUNT;
end_tier= (cur->line_count)/VPTR_COUNT;  // last tier in dest sheet 
end_ofs= (cur->line_count)%VPTR_COUNT;   // offset of last line in dest sheet 
insertpage_count= count / VPTR_COUNT;    // number of pages to insert 
linecount= count % VPTR_COUNT;           // remainder from insertpage_count 

DBG_SHEET_CPY(dout<<"VPTR_COUNT = "<<VPTR_COUNT<<endl);
DBG_SHEET_CPY(dout<<"sheet insert lines start_tier= "<<start_tier<<" start_ofs = "<<start_ofs<<endl);
DBG_SHEET_CPY(dout<<"sheet insert lines end_tier  = "<<end_tier<<" end_ofs = "<<end_ofs<<endl);
DBG_SHEET_CPY(dout<<"sheet insert lines insertpage = "<<insertpage_count<<" linecount = "<<linecount<<endl);

/*if inserting on last tier and no overflow, move lines down to make space */
if (start_tier == end_tier && end_ofs+count <= VPTR_COUNT) {
   DBG_SHEET_CPY(dout<<"insert last tier no overflow"<<endl);
   src= (vptr *)Mem::vmem_trans (cur->tiers[end_tier]);
   memmove (src+start_ofs+count, src+start_ofs, (end_ofs-start_ofs)*sizeof(vptr));
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
   }
else {
   const USHORT lastcount= end_ofs + linecount;  /* how many now on last page */
//   cur_tier = end_tier-1;
   cur_tier = (end_tier-1 < 0 ? 0 : end_tier - 1);
   DBG_SHEET_CPY(dout<<"lastcount = "<<lastcount<<" cur_tier = "<<cur_tier<<endl);
   if (lastcount > VPTR_COUNT) {                    /* append new page to end */
      const USHORT copy_count= lastcount - VPTR_COUNT;  /* how many overflow */
      DBG_SHEET_CPY(dout<<"lastcount = "<<lastcount<<" > VPTR_COUNT append page to end"<<endl);
      cur->tiers[end_tier+1]= vmem_newpage ();

                                    /* copy from end_tier to the new tier */
      src= (vptr *)Mem::vmem_trans (cur->tiers[end_tier]);
      dest= (vptr *)Mem::vmem_trans (cur->tiers[end_tier+1]);
      memcpy (dest,src+(end_ofs-copy_count), copy_count * sizeof(vptr));

                                   /* slide upper lines down to fill in gap */
      memmove (src+linecount,src,(VPTR_COUNT-linecount) * sizeof(vptr));
      end_tier++;
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      }
   else {          /* not appending new page, just slide down existing one */
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      DBG_SHEET_CPY(dout<<"lastcount <= VPTR_COUNT slide down"<<endl);
      src= (vptr *)Mem::vmem_trans (cur->tiers[end_tier]);
      DBG_SHEET_CPY(dout<<"memmove dest = "<<(src+linecount)<<" src = "<<src<<" num bytes = "<<(end_ofs*sizeof(vptr))<<endl); 
      memmove (src+linecount, src, end_ofs*sizeof(vptr));
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      }
            /* for everything up to the first tier, copy out & slide down */
      DBG_SHEET_CPY(dout<<"sheet insert_lines "<<__LINE__<<endl);
   while (cur_tier > start_tier) {
      DBG_SHEET_CPY(dout<<"sheet insert_lines "<<__LINE__<<endl);
      dest= src;                                        /* copy tail out */
      src= (vptr *)Mem::vmem_trans (cur->tiers[cur_tier]);
      DBG_SHEET_CPY(dout<<"sheet insert_lines "<<__LINE__<<endl);
      memcpy (dest, src+(VPTR_COUNT-linecount), linecount* sizeof(vptr));
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
                                                        /* slide it down */
      memmove (src+linecount, src, (VPTR_COUNT-linecount)*sizeof(vptr));
      cur_tier--;
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      }
   dest= src;                                /* now handle first tier */
   src= (vptr *)Mem::vmem_trans (cur->tiers[cur_tier]);
   if (insertpage_count == 0) {
      USHORT const leftover= VPTR_COUNT - start_ofs;
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      if (leftover > linecount) {
         /* just like in the loop, but stop the slide at the insert point */
         memcpy (dest, src+(VPTR_COUNT-linecount), linecount* sizeof(vptr));
         memmove (src+linecount+start_ofs, src+start_ofs, (leftover-linecount)*sizeof(vptr));
         DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
         }
      else { /* copy entire leftover into end of dest */
         memcpy (dest+(linecount-leftover), src+start_ofs, leftover*sizeof(vptr));
         DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
         }
      }
   else {
      USHORT const leftover= VPTR_COUNT - start_ofs;
      if (leftover <= linecount) 
          memcpy (dest+(linecount-leftover), src+start_ofs, leftover*sizeof(vptr));
      else 
         memcpy (dest, src+(VPTR_COUNT-linecount), linecount* sizeof(vptr));
                                          /* insert new tiers */
      DBG_SHEET_CPY(dout<<"sheet insert_lines "<<__LINE__<<endl);
      cur_tier= start_tier+1;
      memmove (cur->tiers+cur_tier+insertpage_count, cur->tiers+cur_tier,(end_tier-start_tier)*sizeof(vptr));
      while (insertpage_count--) {
         cur->tiers[cur_tier]= vmem_newpage();
         cur_tier++;
         }
      if (leftover > linecount) {   /* now copy rest of stuff from first page */
         dest= (vptr *)Mem::vmem_trans (cur->tiers[cur_tier-1]);  /* last one added */
         src= (vptr *)Mem::vmem_trans (cur->tiers[start_tier]);
         DBG_SHEET_CPY(dout<<"sheet insert_lines "<<__LINE__<<endl);
         memcpy (dest+(VPTR_COUNT-(leftover-linecount)), src+start_ofs, (leftover-linecount)*sizeof(vptr));
         }
      }
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
 }
cur->line_count= new_linecount;
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_SHEET_CPY(dout<<"sheet insert_lines new line_count = "<<(cur->line_count)<<endl);
DBG_SHEET_CPY(dout<<"sheet mark_insert row = "<<row<<" count = "<<count<<endl);
mark_insertlines (row, count);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}
#pragma intrinsic -memcpy

/************************************************************************
     copy 'count' lines from source sheet 's' to the current sheet. 
************************************************************************/

void sheet_copylines (vptr s_v,ULONG from,ULONG to, USHORT count)
{
#pragma intrinsic memcpy
                 /* copy 'count' lines from 's' to the current sheet. */
struct sheet *s = NULL;
SHORT space;
vptr v1, v2;
struct line_header *h, *h2;

s= (struct sheet *)Mem::vmem_trans (s_v);
DBG_SHEET(dout<<" sheet_copylines, source sheet = "<<s_v<<" source_sheet ptr = "<<s<<endl);
DBG_SHEET(dout<<" sheet_copylines, active sheet = "<<active_sheet<<" active_sheet ptr = "<<my_active_sheet<<endl);
insert_lines (to, count);        // this takes TONS of time.....
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
space= cur->space;
vmem_burst_start(space);       // MOD 8-9-95 increase speed

while (count--) {
   v1= get_line_anysheet (s, from++);
   h= (struct line_header *)Mem::vmem_trans (v1);   // so does vmem_alloc
//   h2= (struct line_header *)vmem_alloc (space, h->length+sizeof(struct line_header), &v2);
                               // MOD 8-9-95 increase speed
   h2= (line_header *)vmem_burst_alloc(h->length+sizeof(line_header), &v2);

   h2->capacity= h2->length= h->length;
   memcpy (1+h2, 1+h, h->length);
   put_line (to++, v2);
   }
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
//view_redisplay (cur->viewer);
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (cur->viewer);
   fillwords (v->touches, 0xffff, 4);  //inline
   v->flags |= 4;
   vmem_burst_end();        // MOD 8-9-95 increase speed
//   beep_ok();
#pragma intrinsic -memcpy
}

/* **************************************** */
vptr zero_line(void)
{
/* return a new empty line */
vptr l;
struct line_header *h;

//cur= Mem::vmem_trans (active_sheet);
cur = my_active_sheet;
h= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header), &l);
*h= empty_header;
return l;
}

/* **************************************** */
/*     sheet functions                      */
/* **************************************** */

vptr new_sheet_general (USHORT flags)
{
// flags:1- no marks  2- OK to free without saving 4- OK to have no viewers                       */
vptr p_v;

if (!sheet_mem) {
    DBG_SHEET(dout<< "new_sheet_general creating space for sheet_mem"<<endl);
    sheet_mem= vmem_create_space();
}
cur= (struct sheet *)vmem_alloc (sheet_mem, sizeof (struct sheet), &p_v);

memset (cur, 0, sizeof (struct sheet));   /* this clears most fields */
cur->next= all_sheets;                    /* link into list of all sheets */
//cur->next= 0L;                            // all_sheets not used
all_sheets= p_v;
                                       
if (flags & 4)                             /* flags */
    cur->flags |= 1;                       /* Preserve */
if (flags & 2)
    cur->flags |= 2;                       /* free w/o saving */
                                           /* continue with other fields */
cur->space= vmem_create_space();
DBG_SHEET(dout<< "new_sheet_general creating space "<<endl);
DBG_SHEET(dout<<" new_sheet_general sheet = "<<p_v<<" sheet ptr = "<<(cur)<<endl);
if (!(flags&1)) {                           // vmem_alloc marks
   DBG_SHEET(dout<< "new_sheet_general allocating marks"<<endl);
   vmem_alloc (cur->space, (cur->mark_capacity=10) * sizeof(struct mark_rec), &cur->marks_v);
}
return p_v;
}

/* **************************************** */

void new_sheet(void)
{
                       /* create a new sheet and make it the active sheet */
active_sheet= new_sheet_general(0);
my_active_sheet = (struct sheet *)Mem::vmem_trans (active_sheet);
DBG_SHEET(dout<<" new_sheet, active_sheet = "<<active_sheet<<" my_active_sheet = "<<my_active_sheet<<endl);
cur->line_count= 1;            // can't have an empty sheet, put in a line 
put_line_anysheet (cur, 0, zero_line());
}

/* **************************************** */

void sheet_clear (vptr s_v)
{
                                     /* erase all the lines in the sheet */
struct sheet *s= (struct sheet *)Mem::vmem_trans (s_v);
s->line_count= 0;
s->filename[0]= '\0';
vmem_delete_space (s->space);        /* delete everything */
s->space= vmem_create_space();
vmem_alloc (s->space, (s->mark_capacity=10) * sizeof(struct mark_rec), &s->marks_v);
s->mark_count= 0;
s->changed= FALSE;
view_clear (s->viewer);
}

/* **************************************** */

void delete_sheet (vptr sheet_v)
{
//  struct view_rec *v = (struct view_rec *)Mem::vmem_trans(active_view);

                       // mod 9-16-93 view not used ??
//  struct view_rec *v;
//  v = my_active_view;              // mod 8-27-93
//  cur = (struct sheet *)Mem::vmem_trans(v->sheet);

struct sheet* p= (struct sheet *)Mem::vmem_trans (sheet_v);

DBG_SHEET(dout<<" delete_sheet, sheet = "<<sheet_v<<" sheet ptr = "<<(p)<<endl);

        // I should need the next line, but causes problems
//my_active_sheet = (struct sheet *)Mem::vmem_trans (p->next); 
vmem_delete_space (p->space);
vmem_droppage (p->tiers[0]);           /* mod  3-93   */
if(p->viewer != NULL) {            // mod 9-16-93
   view_delete_all (p->viewer);
}
vmem_free (sheet_v);
}

/* **************************************** */

void sheet_delete_view (vptr sheet_v, vptr view)
{
/* unlink the view from the linked list of views attached to this sheet */
/* this function uses both sheet and view information */
vptr next = 0L;
struct sheet *p= (struct sheet *)Mem::vmem_trans (sheet_v);
struct view_rec *q, *v= (struct view_rec *)Mem::vmem_trans (p->viewer);
if (p->viewer == view) {                      /* first one in the list */
   p->viewer= v->next;
   if (!p->viewer && !(p->flags&1)) {         /* delete the sheet, too. */
      vmem_delete_space (p->space);
      vmem_droppage (p->tiers[0]);           /* mod  3-92   */
                  /* need to delete sheet struct from sheet_mem space also ?  */
      vmem_free (sheet_v);
      return;
      }
   }
else {  /* gotta find it and unlink it */
   while (v->next != view) {
      next= v->next;
      v= (struct view_rec *)Mem::vmem_trans (next);
      }
   q= (struct view_rec *)Mem::vmem_trans (v->next);
   v->next= q->next;
   }
}

/* **************************************** */
/*     lines & sheets                       */
/* **************************************** */

struct line_header *line_fetch (vptr sheet_v, ULONG n)
{
cur= (struct sheet *)Mem::vmem_trans (sheet_v);
          //cur = my_active_sheet;        // can't use this here.....
if (n < cur->line_count) {           
   SHORT tier= n/(TIER_SIZE);
   vptr *p= (vptr *)Mem::vmem_trans (cur->tiers[tier]);
   return (struct line_header *)Mem::vmem_trans (p[n%(TIER_SIZE)]);
   }
else 
   return &empty_header;

}

/********************************************************************/
#ifdef FUCKUP                              won't work on startup
struct line_header *line_fetch2(ULONG n)
{
//cur= (struct sheet *)Mem::vmem_trans (sheet_v);
cur = my_active_sheet;        // can't use this here.....
if (n < cur->line_count) {           
   SHORT tier= n/(TIER_SIZE);
   vptr *p= (vptr *)Mem::vmem_trans (cur->tiers[tier]);
   return (struct line_header *)Mem::vmem_trans (p[n%(TIER_SIZE)]);
   }
else 
   return &empty_header;
}
#endif
/* **************************************** */

static char *add_past_end (vptr line_v, struct line_header *h, struct mark_rec *m)
{
char *s;
   DBG_SHEET(dout<< "\nadd past end at col = "<<m->col<<endl);
if (m->col >= h->capacity) {                  /* need to reallocate line */
    DBG_SHEET(dout<< "before memcpy, str = °"<<(char *)(h +1)<<"°"<<endl);
   vptr new_line_v;
   h->capacity= m->col+xbuf;    // temp for debug
//   struct line_header *new_h= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header) + (h->capacity= m->col+xbuf), &new_line_v);
   struct line_header *new_h= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header) + h->capacity + xbuf, &new_line_v);
                   // I don't think memset does anything!!
   memset(new_h, 0x00, sizeof (struct line_header) + h->length + xbuf);
   memcpy (new_h, h, sizeof (struct line_header) + h->length);
   DBG_SHEET(dout<<"h->capacity = "<<h->capacity<<"  h->length = "<< h->length<<endl);
   put_line (m->row, new_line_v);
   h= new_h;
   DBG_SHEET(dout<<"after memcpy, str = °"<<(char *)(h +1)<<"°"<<endl); 
   vmem_free (line_v);
   line_v= new_line_v;
   }
s= (char *)(h+1);                            /* the contents of the line */
memset (s+h->length, ' ', m->col - h->length);
h->length= m->col+1;
DBG_SHEET(dout<< "h->length= "<<h->length<<"  mcol = "<< m->col<<endl);
//memset (s+h->length, '0x00', 1);
//s[1 +h->length] = 0x00;
//h->length= m->col+1;
DBG_SHEET(dout<<"end add_past_end str = ±"<<s<<"°"<<endl);
return s+m->col;
}

/* **************************************** */

void sheet_addchar (mark_t marknum, char c)
{                                      /* add a character to the sheet */
#pragma intrinsic memcpy
//struct mark_rec *m= get_mark (marknum);
struct mark_rec *m= get_mark_fast (marknum); // mod 3-14-93
vptr line_v= get_line (m->row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);

//DBG_SHEET(dout<<" sheet_addchar adding char "<<endl);

if (m->col >= h->length) {
    DBG_SHEET(dout<<"  sheet_addchar adding past end  char = ±"<<c<<"±"<<endl);
    *add_past_end (line_v, h, m)= c;
}
else {
   DBG_SHEET(dout<< "sheet_addchar adding in middle"<<endl);
   char *s;
//   DBG_SHEET(dout<<"original str = ±"<<(char *)(h+1)<<"°"<<
//                            " h->length= "<< h->length<<endl);
//   DBG_SHEET(dout<<"add char at col = "<< m->col<<" char = "<<c<<endl);

   if (h->length == h->capacity) {         /* need to reallocate line */
      vptr new_line_v;
      struct line_header *new_h= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header) + (h->capacity += xbuf), &new_line_v);
      memcpy (new_h, h, sizeof (struct line_header) + h->length);
      put_line (m->row, new_line_v);
      h= new_h;
      vmem_free(line_v);    // mod 3-16-93
      line_v= new_line_v;            /* this may not be needed */
      }
   s= (char *)(h+1);                 /* the contents of the line */
   memmove (s+m->col+1, s+m->col, h->length++ - m->col);
   s[m->col] = c;
   s[h->length +1] = 0x00;
//   DBG_SHEET(dout<< "addchar new str = ±"<<s<<"°  h->length= "<< h->length<<endl);
   }
                                  /* now inform other parts of the system */
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);
//mark_addcol (marknum);             

//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;        // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
ULONG row= mark[marknum].row;
SHORT col= mark[marknum].col;
//SHORT loop= cur->mark_count;       

//while (loop--) {
   if (mark->row == row && mark->col >= col)
      mark->col++;
//   mark++;
//   }
#pragma intrinsic -memcpy
}

/* **************************************** */

void sheet_ovwtchar (mark_t marknum, char c)
{
                                  /* overwrite a character to the sheet */
struct mark_rec *m= get_mark (marknum);
vptr line_v= get_line (m->row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);

if (m->col >= h->length) 
   *add_past_end (line_v, h, m)= c;
else {
   char *s= (char*)(1+h);
   s[m->col]= c;
   }
                                 /* now inform other parts of the system */
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);
m->col++;
}

/* **************************************** */

void sheet_ovwtchars (mark_t marknum, const char* string,SHORT length)
{
while (length--) 
   sheet_ovwtchar (marknum, *string++);
}

/* *********************************************************************
        add string "new_str" of "length at row,col "position" 
************************************************************************/
void sheet_addstr(mark_t position, const char* new_str, const SHORT length)
{
//DBG_SHEET(dout<< "sheet_addstr string = °"<<new_str<<"°"<<endl);
                                
//while (length--)        /* quick & dirty for now */    // OLD
//    sheet_addchar (position, *string++);

//#pragma intrinsic memcpy
struct mark_rec *m= get_mark_fast (position); // mod 3-14-93
vptr line_v= get_line (m->row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
int new_str_len = strlen(new_str);

DBG_SHEET(dout<< "sheet_addstr string = °"<<new_str<<"°"<<endl);

if (m->col >= h->length) {
    DBG_SHEET(dout<<"addstr past end "<<endl);
    for(int j=0; j<=length; j++) {
       *add_past_end (line_v, h, m)= new_str[j];
    }
}
else {
   DBG_SHEET(dout<< "addstr in middle"<<endl);
   char *s;
   int new_len = h->length + length;   // existing str + added str
   DBG_SHEET(dout<<"original str = ±"<<(char *)(h+1)<<"°"<<" h->length= "<< h->length<<endl);
//   DBG_SHEET(dout<<"add char at col = "<< m->col<<" char = "<<c<<endl);

   if (new_len >= h->capacity) {     // need to reallocate line 
      vptr new_line_v;
//      line_header *new_h = (line_header *)vmem_alloc(cur->space, sizeof(line_header)+(h->capacity+xbuf), &new_line_v);
      line_header *new_h = (line_header *)vmem_alloc(cur->space, HDR_SZ+new_len+xbuf, &new_line_v);
//      memcpy (new_h, h, sizeof (struct line_header) + h->length);
      memcpy (new_h, h, HDR_SZ + new_len);
      h->length = new_len;
      put_line (m->row, new_line_v);
      h= new_h;
      vmem_free(line_v);    // mod 3-16-93
      line_v= new_line_v;            /* this may not be needed */
      }
   s= (char *)(h+1);                 /* the contents of the line */
//   memmove (s+m->col+1, s+m->col, h->length++ - m->col);
   memmove (s+m->col+length, s+m->col, h->length++ - m->col);
//   s[m->col] = c;
   memcpy(s+m->col, new_str, new_str_len);
   h->length = new_len;
   s[h->length +1] = 0x00;
   DBG_SHEET(dout<< "addstr after memcpy new str = ±"<<s<<"°  h->length= "<< h->length<<endl);
   }                                 // end else

cur->changed= TRUE;                 /* now inform other parts of the system */
view_linetouch (cur->viewer, m->row);

//#pragma intrinsic -memcpy
//}                         

}
/*******************************************************************/
//static void sheet_insert_line (ULONG row, vptr line)
void sheet_insert_line (ULONG row, vptr line)
{
insert_lines (row, 1); 
put_line (row, line);
cur->changed= TRUE;
view_lineadd (cur->viewer, row);   
}

/* **************************************** */

void sheet_insert_blank_line (ULONG row)
{
vptr v= zero_line();
sheet_insert_line (row, v);
}

/* **************************************** */

void sheet_splitline (mark_t marknum)
/****************************************************************/
/* split the line at the mark position */
/*******************************************************************/
{
struct mark_rec *m= get_mark (marknum);
SHORT col= m->col;
ULONG row= m->row;

DBG_SHEET(dout<<"sheet_splitline col = "<<col<<" row =  "<<row<<endl);

if (col == 0) 
   sheet_insert_blank_line (row);
else {                             // col != 0
   vptr line_v= get_line (row);
   struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
   if (col >= h->length) {               // split after end of line
      sheet_insert_blank_line (1+row);
      m= get_mark (marknum);
      m->row++;
      m->col= 0;
      }
   else {                      // split in middle of line, the normal case 
      vptr next_line_v;
      struct line_header *h2;
    //  char* str;
      SHORT len= h->length - col;
      h2= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header) + len+xbuf, &next_line_v);
      h2->capacity= (h2->length= len) + xbuf;
      DBG_SHEET(dout<<"sheet_splitline org line = "<<(h->length)<<" chars long "<<" col = "<<col<<endl);
      memcpy (h2+1, col + (char *)(h+1), len);
      h->length -= len;
      
      DBG_SHEET(dout<<"sheet_splitline  "<<row<<" now "<<(h->length)<<" chars"<<endl);
      DBG_SHEET(dout<<"sheet_splitline  "<<(row + 1)<<" now "<<len<<" chars"<<endl);
      view_linetouch (cur->viewer, row);
      sheet_insert_line (row+1, next_line_v);
      mark_splitline (marknum);                // move cursor
      }
   }
}
/* **************************************** */

static void  free_a_tier (SHORT tiernum,USHORT linecount)
{
USHORT loop;
vptr tier= cur->tiers[tiernum];
vptr * src= (vptr *)Mem::vmem_trans (tier);

for (loop= 0; loop < linecount; loop++) {  /* free all lines on page */
   vmem_free (src[loop]);
   }
vmem_droppage (tier);
//cur= (struct sheet *)Mem::vmem_trans (active_sheet);   // mod 8-25-93
}

/* **************************************** */

static void  free_tier_part (SHORT tiernum,USHORT linecount,USHORT startline)
{
vptr tier= cur->tiers[tiernum];
vptr * src= (vptr *)Mem::vmem_trans (tier);

DBG_SHEET(dout<< "free_tier_part source = "<<src<<" startline = "<<startline<<endl);

while (linecount--) {
   vmem_free (src[startline]);     // was [++]
   startline++;
   }
}

/* **************************************** */

void sheet_delete_lines (ULONG row,SHORT count)
{
ULONG new_linecount;
USHORT c1,c2;
SHORT end_tier, cur_tier;
vptr *src, *dest;

SHORT dest_tier= row/VPTR_COUNT;
USHORT dest_ofs= row%VPTR_COUNT;
USHORT end_ofs= (cur->line_count)%VPTR_COUNT; //offset of 1+last line in file 
SHORT source_tier= (row+count)/VPTR_COUNT;    // where the copy starts from 
USHORT source_ofs= (row+count)%VPTR_COUNT;
SHORT delcount;

DBG_SHEET(dout<< "sheet_delete_lines source_tier = "<<source_tier<<" source_ofs = "<<source_ofs<<" row = "<<row<<endl);

//cur= (struct sheet *)Mem::vmem_trans (active_sheet);
cur = my_active_sheet;                    // mod 8-25-93
new_linecount= cur->line_count-count;

if (cur->line_count == row+count) {    /* deleting last lines on last page */
   if(row == 0) {                    // deleting 1st and only line in sheet
      put_line(1,zero_line());       // kludge - add dummy line
   }
   free_tier_part (source_tier,count,source_ofs);
   cur->changed= TRUE;
   if (new_linecount == 0) {
      if (count > 1) {
         view_linedel (cur->viewer,1,(USHORT)count-1);
         mark_dellines (1, count-1);
      }                            // else deleting only line in the sheet 
      put_line (0,zero_line()); 
      cur->line_count= 1;
      view_linetouch (active_view, 0);
      mark_dellines (row, count);
      }
   else {
      view_linedel (cur->viewer,row,count);
//      view_linedel (cur->viewer,row);
      cur->line_count= new_linecount;
      mark_dellines (row, count);
      }
   return;
   }

end_tier= (cur->line_count)/VPTR_COUNT;  /* last tier in file */
if (dest_tier != source_tier) {          /* delete all pages that are empty */
   cur_tier= dest_tier+1;
   delcount= source_tier-cur_tier;
   if (delcount > 0) {                   /* free any whole pages */
      while (cur_tier < source_tier) free_a_tier (cur_tier++, VPTR_COUNT);
                                         /* slide tier pointers up */
      memmove (cur->tiers+dest_tier+1, cur->tiers+source_tier,
                    (end_tier+1-source_tier)*sizeof(vptr));
      source_tier -= delcount;
      end_tier -= delcount;
      }
   free_tier_part (dest_tier, c1=VPTR_COUNT-dest_ofs, dest_ofs);
   free_tier_part (source_tier, source_ofs, 0);
   }
else c1= count;

dest= (vptr *)Mem::vmem_trans (cur->tiers[dest_tier]);
src= (vptr *)Mem::vmem_trans (cur->tiers[source_tier]);

if (cur->line_count-row <= c1) {      /* whole thing fits back in dest_tier */
   if (end_tier == source_tier) {       /* in one piece.  just grab it */
      memcpy (dest+dest_ofs, src+source_ofs, (cur->line_count-row)*sizeof (vptr));
      vmem_droppage (cur->tiers[source_tier]);
      }
   else {                                 /* in two pieces */
      c2= VPTR_COUNT-source_ofs;
      memcpy (dest+dest_ofs, src+source_ofs, c2*sizeof(vptr));
      vmem_droppage (cur->tiers[source_tier]);
      src= (vptr *)Mem::vmem_trans (cur->tiers[end_tier]);
      memcpy (dest+dest_ofs+c2, src, ((cur->line_count-row)-c2)*sizeof(vptr));
      vmem_droppage (cur->tiers[end_tier]);
      }
   }
else {                                    /* slide page up */
   if (dest_tier == source_tier) {  
      free_tier_part (dest_tier, c1, dest_ofs);
      dest= (vptr *)Mem::vmem_trans (cur->tiers[dest_tier]);
      memmove (dest+dest_ofs,src+source_ofs,(VPTR_COUNT-source_ofs)*sizeof(vptr));
      source_tier++;
      }
   else {
      c2= VPTR_COUNT - source_ofs;  /* how many left on page */
      if (c2 <= c1) {                 /* move whole thing */
         memcpy (dest+dest_ofs, src+source_ofs, c2*sizeof(vptr));
         vmem_droppage (cur->tiers[source_tier]);
         c1 -= c2;                  /* size of hole, at end of dest_tier */
         memmove (cur->tiers+source_tier, cur->tiers+source_tier+1, (end_tier-source_tier)*sizeof(vptr));
         end_tier--;                /* got moved up */
         }
      else {
         /* fill up hole in dest, all freespace now in source.  then slide
            it up to put the hole at the end, and make it the new dest. */
         memcpy (dest+dest_ofs, src+source_ofs, c1*sizeof(vptr));
         memmove (src, src+source_ofs+c1, (c2-c1)*sizeof(vptr));
         c1= VPTR_COUNT- (c2-c1);  /* now the hole size */
         dest= src;
         source_tier++;
         }
      }
   if (dest_tier != end_tier) {        /* now bubble hole down to the end */
      dest_ofs= VPTR_COUNT-c1;
      while (source_tier < end_tier) {
         /* before: the hole is at the end of dest.
            after: the hole is at the end of source.   */
         src= (vptr *)Mem::vmem_trans (cur->tiers[source_tier]);
         memcpy (dest+dest_ofs, src, c1*sizeof(vptr));
         memmove (src, src+c1, dest_ofs*sizeof(vptr));
         dest= src;
         source_tier++;
         }
                                                   /* the last one */
      src= (vptr *)Mem::vmem_trans (cur->tiers[end_tier]);
      if (end_ofs <= c1) {                   /* take the whole thing */
         memcpy (dest+dest_ofs, src, end_ofs*sizeof(vptr));
         vmem_droppage (cur->tiers[end_tier]);
         }
      else {
         memcpy (dest+dest_ofs, src, c1*sizeof(vptr));
         memmove (src, src+c1, (end_ofs - c1)*sizeof(vptr));
         }
      }
   }
                                     /* stick 0's in any dropped tier slots */
c2= new_linecount / VPTR_COUNT;                 /* what page am I now on? */
while (++c2 <= end_tier) 
   cur->tiers[c2]= 0L;    /* set trailing ones to 0 */

cur->changed= TRUE;

//if (new_linecount == 0) {      // mod 7-28-95
//   put_line (0,zero_line());  
 //  new_linecount= 1;
//}

view_linedel (cur->viewer,row,count);
//view_linedel (cur->viewer,row);
cur->line_count= new_linecount;
mark_dellines (row, count);
}

/* **************************************** */

void sheet_joinlines (mark_t marknum)
{
                 /* join the line containing the mark to the previous line */
struct mark_rec *m= get_mark (marknum);
ULONG row= m->row;
vptr line, line1;
struct line_header *h1, *h2;
SHORT newlength, line_1_length;

if (row == 0) {            //return;         /* no previous line */
   if(m->col == 0) {
      line1= get_line (row);
      h1= (struct line_header *)Mem::vmem_trans (line1);
      if(h1->length == 0) {     // delete this row
         row++;
      }
      else {
         goto join_line_exit;
      }
   }
}                                                /* get the 2 lines */
line1= get_line (row-1);
h1= (struct line_header *)Mem::vmem_trans (line1);
line_1_length= h1->length;
line= get_line (row);
h2= (struct line_header *)Mem::vmem_trans (line);
                                    /* how big will the resulting line be ? */
newlength= line_1_length + h2->length;
if (newlength > h1->capacity) {
                                               /* reallocate the line */
   struct line_header *new_h= (struct line_header *)vmem_alloc (cur->space, sizeof (struct line_header) + (h1->capacity= newlength+xbuf), &line);
   memcpy (new_h, h1, sizeof (struct line_header) + line_1_length);
   vmem_free (line1);
   h1= new_h;
   put_line (row-1, line);
   }
                                 /* concatenate the second after the first */
memcpy (line_1_length + (char *)(h1+1), h2+1, h2->length);
h1->length= newlength;
mark_joinlines (row, line_1_length);
view_linetouch (cur->viewer, row-1);                                            
sheet_delete_lines (row, 1);                 /* close up the sheet */
mark_joinlines (row, line_1_length);         /* update the marks */
join_line_exit:
}

/* **************************************** */

void sheet_delchar (mark_t marknum)
{
//struct mark_rec *m= get_mark(marknum);
struct mark_rec *m= get_mark_fast (marknum);
vptr line_v= get_line (m->row);
//struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
struct line_header *h= (struct line_header *)Mem::vmem_trans(line_v);
char *s;

if (m->col >= h->length) return;
s= (char *)(h+1);
memmove (s+m->col, s+m->col +1, h->length-m->col);
h->length--;
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);    

DBG_SHEET(dout<< "sheet_delchar row "<<m->row<<" col "<<m->col<<endl);

//mark_delcol (marknum);

//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
cur = my_active_sheet;              // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
ULONG row= mark[marknum].row;
SHORT col= mark[marknum].col;
//SHORT loop= cur->mark_count;

//while (loop--) {
   if ((mark->row == row) && (mark->col > col))
      mark->col--;
//   mark++;
//   }

}

/* **************************************** */

SHORT sheet_line_length (struct sheet *sh, ULONG row)   
{
struct line_header *line_hdr= (struct line_header *)Mem::vmem_trans (get_line_anysheet (sh, row));

return line_hdr->length;
}

/* **************************************** */

SHORT sheet_line_length (ULONG row)   
{
line_header *line_hdr= (line_header *)Mem::vmem_trans (get_line(row));

return line_hdr->length;
}

/* **************************************** */

//ULONG sheet_linecount (vptr sh)
//{
//struct sheet *s= (struct sheet *)Mem::vmem_trans (sh);

//return s->line_count;
//}

/* **************************************** */

SHORT sheet_peek (vptr sh, mark_t mark)
{
SHORT col;                                   
struct mark_rec *m;
struct line_header *line_hdr;

m= get_mark_array (sh)+mark;
col= m->col;
line_hdr= (struct line_header *)Mem::vmem_trans (
          get_line_anysheet ((struct sheet *)Mem::vmem_trans (sh), m->row));
if (col >= line_hdr->length) return '\n';
return (SHORT)(((UCHAR *)(line_hdr+1))[col]);
}

/* **************************************** */
#ifdef VIEW_EXTENDED_COM          // disable adv wp features 1-7-93
SHORT sheet_peek_fast (struct sheet *sh, struct mark_rec *m)
{
SHORT col;                                  
struct line_header *line_hdr;

col= m->col;
line_hdr= (struct line_header *)Mem::vmem_trans (get_line_anysheet (sh, m->row));
if (col >= line_hdr->length) return '\n';
return (SHORT)(((UCHAR *)(line_hdr+1))[col]);
}
#endif
/* **************************************** */

void sheet_print (vptr sh)
{
/* simply save sheet to printer. */
/* A more elaborate print function could be placed here */

mylib_file_t f;
struct sheet *cur= (struct sheet *)Mem::vmem_trans (sh);
bool sheet_saved = cur->changed;

if (stricmp((char *)settings.printer_setup_string, "")!= 0) {
   if (mylib_create (settings.printer, &f)) {
      mylib_write_text (f, settings.printer_setup_string, sizeof(settings.printer_setup_string));
   }
}

sheet_save (sh, settings.printer);
if(sheet_saved)           // sheet_save sets changed == FALSE;
   cur->changed = TRUE;   // make sure we know it's not really saved to disk

}
