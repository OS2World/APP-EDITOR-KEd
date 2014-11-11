/********************************************************************
file: WDISP.CPP               Copyright 1986-1991 by Dlugosz Software
   part of TOW  Text Output Windows
   window display functions
**********************************************************/

#ifdef __OS2__
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>
#endif

#include "usual.h"
#include "basewin.h"
#include "test.h"
#include <string.h>

#pragma intrinsic memcpy

#define DESKTOP            // comment out DESKTOP for some applications

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
inline void mov2scr (void* source, void* dest, unsigned count)
{
memcpy ((UCHAR _far16 *)dest, source, 2*count);
}
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*   display functions                      */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

inline void disp1 (unsigned short _far16* dest, unsigned short word)
{
*dest= word;  //simpler in this implementation
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
static void display_row (
   unsigned short _far16* dest,                    // display position
   unsigned short* r, unsigned short* rmask, // row of data and eclipse mask
   unsigned startcol, unsigned count) // portion of row to display
{
unsigned short* source= r + startcol;
if (! (rmask[0]&1)) {  // no mask, just move.
   mov2scr(source, dest, count);
}
else { /* the hard way */
   //rather than showing one char at a time, locate each run of
   //chars and print them all at once.
   // 1.skip blocked chars
   // 2.see if done
   // 3.memorize position of first non-blocked char
   // 4.skip non-blocked
   // 5.display the run of consecutive chars
   // 6.start over
   unsigned const short* maskword= rmask + ((startcol+1) >> 4);
   unsigned short maskbit= 1 << ((startcol+1) & 15);
   unsigned short* first;    
   unsigned short _far16* rundest;

   int runlength;
   for (;;) {
      while (count && (*maskword & maskbit)) {  //skip blocked
         dest++;  source++;
         count--;
         if (!(maskbit <<= 1)) {
            maskbit= 1;
            maskword++; 
         }
      }
      if (count <= 0) {
        break;   //see if done
      }
      first= source;   //remember position
      rundest= dest;
      runlength= 0;
      do {        //skip blocked
         runlength++;
         dest++;  source++;
         if (!(maskbit <<= 1)) {
            maskbit= 1;
            maskword++; 
         }
         count--;
         } while (count && !(*maskword & maskbit));
      mov2scr (first, rundest, runlength);
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#ifdef DESKTOP

static void blank_row (
   unsigned short _far16* dest,          // display position
   unsigned short* rmask,         // eclipse mask
   unsigned startcol, unsigned count, // portion of row to display
   unsigned fillval)
{
if (! (rmask[0]&1)) { // no mask, just zap
   DBG_WIN1(dout<<"blank_row no mask "<<endl);
   while (count--) {
      disp1(dest++,fillval);
   }
}
else { // the hard way
   unsigned short* maskword= rmask + ((startcol+1) >> 4);
   unsigned short maskbit= 1 << ((startcol+1) & 15);
   DBG_WIN1(dout<<"blank_row mask "<<endl);
   while (count--) {
      if (! (*maskword & maskbit)) {
         disp1(dest, fillval); /* show thru */
      }
      dest++;
      if (!(maskbit <<= 1)) {
         maskbit= 1;
         maskword++; 
      }
   }
 }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::blank_screen(twocorner& box)
{
twocorner overlap;  //area to clear
    DBG_WIN1(dout<<"blank_screen passed  coords ul.x = "<<box.ul.x<<" ul.y = "<<box.ul.y<<" lr.x = "<<box.lr.x<<" lr.y = "<<box.lr.y<<endl);
if (!intersection(Parent->range, box, &overlap)) {
    DBG_WIN1(dout<<"blank_screen no intersection "<<endl);
    DBG_WIN1(dout<<"blank_screen Parent  coords ul.x = "<<Parent->range.ul.x<<" ul.y = "<<Parent->range.ul.y<<" lr.x = "<<Parent->range.lr.x<<" lr.y = "<<Parent->range.lr.y<<endl);
    DBG_WIN1(dout<<"blank_screen overlap coords ul.x = "<<overlap.ul.x<<" ul.y = "<<overlap.ul.y<<" lr.x = "<<overlap.lr.x<<" lr.y = "<<overlap.lr.y<<endl);
    return;
}
/* this assumes the box is contained entirely within the mask */
unsigned short pos= Parent->Size.x*(overlap.ul.y-Parent->range.ul.y) + 
                                   (overlap.ul.x-Parent->range.ul.x);
int rows= overlap.lr.y - overlap.ul.y +1;
int length= overlap.lr.x - overlap.ul.x + 1;
unsigned short* rmask= eclipse_mask + (overlap.ul.y- Location.ul.y) * maskwidth;
int startcol= overlap.ul.x - Location.ul.x;
USHORT offset = pos;
USHORT charcount = (Screen.get_screen_width() - startcol) +(rows * Screen.get_screen_width());

while (rows--) {
   DBG_WIN1(dout<<"blank_screen calling blank_row row = "<<rows<<" startcol = "<<startcol<<" count = "<<length<<endl);
   blank_row (Parent->save_area+pos, rmask, startcol, length, Parent->fillval);   
   pos += Parent->Size.x;
   rmask += maskwidth;
   }
   VioShowBuf(2 * offset, 2 * charcount, 0);

if (Parent)
   Parent->refresh(overlap);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#else                                          // DESKTOP null
void window_core::blank_screen(twocorner& box)
{   //does nothing
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::display (const twocorner& box)
{
twocorner overlap;  //area to actually display
twocorner temp;     //intersection of box and window location

// clip against window location
if (!visible() || !intersection(Location, box, &temp))
   return;
// clip against display location
if (!intersection(temp,Parent->range,&overlap))
   return;
int row= overlap.ul.y;

unsigned short pos= Parent->Size.x* (overlap.ul.y -Parent->range.ul.y) +
              (overlap.ul.x -Parent->range.ul.x);
int relrow= row- Location.ul.y;
unsigned short* r= save_area + (relrow * Size.x);
unsigned short* rmask= eclipse_mask + relrow * maskwidth;
int count= overlap.lr.x - overlap.ul.x + 1;
int startcol= overlap.ul.x - Location.ul.x;
USHORT offset = pos;
USHORT charcount = (Screen.get_screen_width() - startcol) +((overlap.lr.y - overlap.ul.y) * Screen.get_screen_width());

while (row <= overlap.lr.y)  {
   display_row (Parent->save_area+pos, r,rmask, startcol, count);
   pos += Parent->Size.x;
   r += Size.x;
   rmask += maskwidth;
   row++;
}
   VioShowBuf(2 * offset, 2 * charcount, 0);

if (Parent)
   Parent->refresh(overlap);
}
                                           
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::refresh (twocorner box)
{
box.ul.y += Location.ul.y;
box.lr.y += Location.ul.y;
box.ul.x += Location.ul.x;
box.lr.x += Location.ul.x;
display (box);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//#if FALSE
void tdraw_screen (twocorner& box)
{
unsigned short _far16* dest;          // display position
twocorner myrange = Screen.get_range();

// first, clear that area of the window
#ifdef DESKTOP
int width = box.lr.x - box.ul.x +1;
dest = meta_window::address + Screen.get_screen_width()*(box.ul.y-(myrange.ul.y)) +
                (box.ul.x- (myrange.ul.x));
for (int row= box.ul.y; row <= box.lr.y; row++, dest+=Screen.get_screen_width()) {
   int loop= width;
   while (loop--) 
      disp1(dest,Screen.get_fillval());
   }
   VioShowBuf(0,Screen.get_vidbufsize(), 0);

#endif
// part 2-- redraw the zone
//for (window_core* w= meta_window::display_list; w != 0; w= w->Next)
//   w->display(box);
}
//#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
