/********************************************************************
file: WCORE.CPP               Copyright 1986-1991 by Dlugosz Software
   Part of TOW   Text Output Windows
   most methods for class window_core
**********************************************************/
#include "usual.h"
#include "tow.h"
#include <string.h>  //need memset()
#include "misc.h"     // fillwords

#include "test.h"

#pragma intrinsic memset

window_core* window_core::default_parent= 0;

#ifndef __OS2__
extern "C" void fillwords (void far* dest, unsigned val, unsigned count);
#endif

#ifdef FUCKUP
#ifdef __OS2__
void fillwords (void* dest, unsigned value, unsigned count)
{
  unsigned short* mydest = (unsigned short*)dest;
  while (count--) 
    *(mydest++) = (unsigned short)value;
}
#endif
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef INTERSECT_INLINE
bool window_core::intersection (const twocorner& a,const twocorner& b,twocorner* result)
/* finds the intersection of 2 boxes and returns
   TRUE if intersection exists. */
{
return (
(result->ul.x= (a.ul.x > b.ul.x) ? a.ul.x : b.ul.x /* left */) <=
(result->lr.x= (a.lr.x < b.lr.x) ? a.lr.x : b.lr.x /* right */) &&
(result->ul.y= (a.ul.y > b.ul.y) ? a.ul.y : b.ul.y /* top */) <=
(result->lr.y= (a.lr.y < b.lr.y) ? a.lr.y : b.lr.y /* bottom */));
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
const unsigned short* window_core::get_buffer (int row, int col) const
{
return save_area + (row*Size.x) + col;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     eclipse mask management              */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

inline void window_core::clear_mask ()
{     /* deallocate eclipse mask for a window */
delete[] eclipse_mask;  //the correct form, not yet supported
eclipse_mask= 0;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::add_to_mask (unsigned short* mask,twocorner& box, 
                               short int maskwidth, twocorner& shadow)
{     // add the eclipse to mask
twocorner overlap;

if (intersection(shadow, box, &overlap)) {
   overlap.ul.x -= box.ul.x;
   overlap.ul.y -= box.ul.y;
   overlap.lr.x -= box.ul.x;
   overlap.lr.y -= box.ul.y;  /* normalize coordinates */
   unsigned short firstword= (1+overlap.ul.x) >> 4;
   unsigned short lastword= (1+overlap.lr.x) >> 4;
   unsigned short firstmask=  0xffff << ((1+overlap.ul.x) & 15);
   unsigned short lastmask= ~(0xfffe << ((1+overlap.lr.x) & 15));
   mask += maskwidth * overlap.ul.y;   /* start here */
   int loop= overlap.lr.y - overlap.ul.y +1;   /* number of rows */
   unsigned short wordcount= lastword-firstword-1;
   if (wordcount==0xffff) {  /* all in one word */
//   if (wordcount==-1) {  /* all in one word */
      unsigned short maskbyte= firstmask & lastmask;  /* middle part */
      while (loop--) {
         mask[0] |= 1;  /* mark row as used */
         mask[firstword] |= maskbyte;
         mask += maskwidth;
         }
      }
   else while (loop--) {   /* multiple word mask */
      mask[0] |= 1;  /* mark row as used */
      unsigned short* maskptr= mask + firstword;
      maskptr[0] |= firstmask;
//      fillwords (maskptr+1, 0xffff, wordcount);
      memset(maskptr+1, 0xffff, (2 * wordcount));     // mod RADICK
      maskptr[wordcount+1] |= lastmask;
      mask += maskwidth;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void sub_from_mask (unsigned short* mask,twocorner& box, 
                           short int maskwidth, twocorner& shadow)
/* subtract the shadow from mask */
{
twocorner overlap;

/* shadow MUST be contained within box.  Does not check. */
overlap.ul.x = shadow.ul.x - box.ul.x;
overlap.ul.y = shadow.ul.y - box.ul.y;
overlap.lr.x = shadow.lr.x - box.ul.x;
overlap.lr.y = shadow.lr.y - box.ul.y;  /* normalize coordinates */
unsigned short firstword= (1+overlap.ul.x) >> 4;
unsigned short lastword= (1+overlap.lr.x) >> 4;
unsigned short firstmask= ~(0xffff << ((1+overlap.ul.x) & 15));
unsigned short lastmask= 0xfffe << ((1+overlap.lr.x) & 15);
mask += maskwidth * overlap.ul.y;   /* start here */
int loop= overlap.lr.y - overlap.ul.y +1;   /* number of rows */
unsigned short wordcount= lastword-firstword-1;
   if (wordcount==0xffff) {  /* all in one word */
//   if (wordcount==-1) {  /* all in one word */
   unsigned short maskbyte= firstmask | lastmask;  /* middle part */
   while (loop--) {
      mask[firstword] &= maskbyte;
      mask += maskwidth;
      }
   }
else while (loop--) {   /* multiple word mask */
   unsigned short* maskptr;
   if (*mask & 1) {
      maskptr= mask + firstword;
      maskptr[0] &= firstmask;
//      fillwords (maskptr+1, 0, wordcount);
      memset(maskptr+1, 0, (2 * wordcount));     // mod RADICK
      maskptr[wordcount+1] &= lastmask;
      if (mask[0]==1) {  // see if I can eliminate this row
         int loop= 1;
         while (loop < maskwidth && mask[loop] == 0) loop++;
         if (loop == maskwidth) /* got all the way through */
            mask[0]= 0;  /* mark as 'all clear' */
         }
      }
   mask += maskwidth;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::make_mask ()
/* zap the old mask and recompute a new one from scratch. */
{
memset (eclipse_mask, 0, 2 * Size.y * maskwidth);
extend_mask (eclipse_mask,Location, Location);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::extend_mask (unsigned short* mask,twocorner& mask_pos, twocorner& zone)
/* adds all shadows above 'this' falling in zone to the mask. */
{
twocorner overlap;
window_core* w= display_list();

while (w != this)  {
   if (intersection(w->Location, zone, &overlap)) {
      add_to_mask (mask,mask_pos,maskwidth, overlap);
      }
   w= w->Next;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::stomp (twocorner& shadow, window_core* w, window_core* firstnot)
/* adds the shadow to all windows starting with w,
   ending with (not including) firstnot.  */
{
while (w != firstnot) {
   add_to_mask (w->eclipse_mask, w->Location, w->maskwidth, shadow);
   w= w->Next;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::unstomp (twocorner& shadow, window_core* w)
/* recomputes masks to all windows intersecting shadow starting with w. */
/* other version has another parameter */
{
twocorner overlap;

while (w) {
   if (intersection (shadow, w->Location, &overlap)) {
      unsigned short* mask= w->eclipse_mask;
      sub_from_mask (mask,w->Location, w->maskwidth, overlap);
      w->extend_mask (mask,w->Location, overlap);
      w->display (overlap);
      }
   w= w->Next;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     create & destroy                     */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#pragma argsused  //bogus warning from BC++

window_core::window_core (const meta_window& disp)
{     //create a master window
Size.y= disp.get_screen_height();
Size.x= disp.get_screen_width();
Location.ul.x= 0;
Location.lr.x= disp.get_screen_width()-1;
Location.ul.y= 0;
Location.lr.y= disp.get_screen_height()-1;
range= Location;
status= SharedSave;  //may add more flags later.
maskwidth= (Size.y +15 +1) >> 4;  //is this ever used?  should not be.
eclipse_mask= 0;
fillwords (border_width, 1, 4);
My_Mem = FALSE;
save_area= (unsigned short _far16 *)disp.address;  
Parent= 0;
First_child= 0;
default_parent= this;
fillval= disp.get_fillval();
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

window_core::window_core (window_core* par, int height, int width, int y_pos, int x_pos)
{
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
Size.x= width;
Size.y= height;
range.ul.x= range.ul.y= 0;
range.lr.x= width-1;
range.lr.y= height-1;
Location.ul.x= x_pos;
Location.lr.x= x_pos+ width -1;
Location.ul.y= y_pos;
Location.lr.y= y_pos+ height -1;
status= 0;
maskwidth= (width +15 +1) >> 4;
eclipse_mask= 0;
fillwords (border_width, 1, 4);
My_Mem = TRUE;
save_area= new unsigned short[height*width];
memset(save_area, 0x0000, (height*width));
Parent= par;
First_child= 0;
fillval= 0x0020;
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_WIN1(dout<<" in wcore reg ctor "<<endl);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

window_core::~window_core ()
{
DBG_MEM(memout<<" wcore delete file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
hide();
if(My_Mem) {          // TRUE only for a regular window object, not meta
   delete[] save_area;    
}
DBG_MEM(memout<<" wcore delete file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     public methods                       */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::clear (int r1,int r2,int c1,int c2, USHORT fillvalue)
{
unsigned short* buf= get_buffer(r1,c1);
int count= c2 - c1 +1;
curr_cell = fillvalue;             // save fillval as default
alt_attr = HIUCHAR(fillvalue);    // set alt_attr also (see border())
if (c2 < c1 || r2 < r1 || r1 > Size.y || r2 >Size.y 
                     || c1 > Size.x || c2 > Size.x) return;
DBG_WIN1(dout<<"wcore::clear start row = "<<r1<<" end row ="<<r2<<"  "<<count<<" cells with fillvalue "<<hex<<fillvalue<<dec<<endl);
DBG_MEM(memout<<" wcore clear file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
for (int row= r1; row <= r2; row++) {
   DBG_WIN1(dout<<"wcore::clear start row =  "<<row<<" count = "<<count<<endl);
   fillwords (buf, fillvalue, count);
   buf += Size.x; 
}
if (displayed()) {  /* display */
   twocorner q;
   q.ul.x= c1 + Location.ul.x;
   q.lr.x= c2 + Location.ul.x;
   q.ul.y= r1 + Location.ul.y;
   q.lr.y= r2 + Location.ul.y;
   display(q);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::hide()
{  //make sure it is not on the display
DBG_MEM(memout<<" wcore hide file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
if (visible()) {  // on display list
   window_core* scanner= display_list();
   if (scanner == this)
      Parent->First_child= Next;  //unlink from head of list
   else {   //locate in stacking sequence
      while (scanner->Next != this) scanner= scanner->Next;
      scanner->Next= Next;  //unlink from middle of list
      }
   blank_screen (Location);  /* erase window */
   clear_mask ();
   status &= ~(Display|Refresh);
   unstomp (Location, Next);
   }
}

/**********************************************************************/
                          //make sure it is the TOP of the display
void window_core::show()
{ 
window_core* first_under;
window_core* scanner= display_list();
if (this == scanner) return;          // nothing to do
int size= Size.y * maskwidth;           
if (visible()) {                      /* in display list */
   while (scanner->Next != this)      /* locate in stack */
      scanner= scanner->Next;
   first_under= Next;                 /* remember first one under this one*/
   scanner->Next= Next;               /* unlink w */ 
   memset (eclipse_mask, 0, 2*size);
   }
else {                                /* not on display-- make it so */
   status |= (Display|Refresh);
   eclipse_mask= new unsigned short[size];
   memset (eclipse_mask, 0, 2*size);
   first_under= 0;                    /* none were under it */
   }
display(Location);                    /* draw window */
stomp (Location, display_list(), first_under);   /* new shadow */
Next= display_list();                 /* place on top */
Parent->First_child= this;
}

