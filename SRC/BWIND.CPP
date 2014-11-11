/*****************************************************
file: BWIND.C       Copyright 1989 by Dlugosz Software
   basic window management functions
*****************************************************/
#include "usual.h"
#include "getkey.h"            /* getkey() used by window_getkey() */
#include "basewin.h"
#include "test.h"

/*********************************************************************/
//USHORT window_getkey (basewin* win ,SHORT row, SHORT col, byte color)
struct key_rec& window_getkey(basewin* win ,SHORT row, SHORT col, byte color)
{
byte old_color;
//#ifdef DBG_KEY1
//USHORT key;
//UCHAR lokey, hikey;     // low and high byte of KEY USHORT
//USHORT keystate;
//#endif

old_color= win->peek_attr(row, col);
win->highlight(row, col, color, 1);   
mykey_rec = getexkey();
//#ifdef DBG_KEY1
//   key = mykey_rec.value;
//   keystate = mykey_rec.state;
//   lokey = LOUCHAR(key);
//   hikey = HIUCHAR(key);
//     DBG_KEY1(dout<<'\n'<<"window_getkey =  "<<hex<<key<<"  high = "<<hikey<<
//                    " low = "<< lokey<<" keystate = "<<keystate<<dec<<endl);
//#endif

                                 /* process any internal keys here ... */
win->highlight(row, col, old_color, 1);   
//return key;
return mykey_rec;
}

/* **************************************** */
#ifdef FUCKUP
void window_get_position (window_t w_v, SHORT *row, SHORT *col)
{
/*   reads GLOBAL window_rec w_v     */

struct window_rec *w= (window_rec *)Mem::vmem_trans (w_v);
*row= w->location.ul.y;
*col= w->location.ul.x;
}

/* **************************************** */

void window_get_size (window_t w_v, SHORT *height, SHORT *width)
{
/*   reads GLOBAL window_rec w_v     */

struct window_rec *w= (window_rec *)Mem::vmem_trans (w_v);
*height= w->size.y;
*width= w->size.x;
}
#endif
/*****************************************************************/


#ifdef FUCKUP 
window_t create_window (USHORT height,USHORT width,USHORT y_pos,USHORT x_pos)
{
//struct window_rec *w;
vptr w_v;
   //USHORT wsize = (USHORT)2*height*width;        // just for debug 8-19-92

//w= (window_rec *)vmem_alloc(window_mem, sizeof (struct window_rec), &w_v);
//w->size.x= width;
//w->size.y= height;
   //w->location.ul.x= x_pos;
   //w->location.lr.x= x_pos+ width -1;
   //w->location.ul.y= y_pos;
   //w->location.lr.y= y_pos+ height -1;

//w->location.ul.x= Screen.range.ul.x;
//w->location.lr.x= x_pos+ width -1;
//w->location.ul.y= Screen.range.ul.y;
//w->location.lr.y= y_pos+ height -1;

   //w->status_word= 0;
   //w->eclipse_mask= 0L;
   //w->maskwidth= 2 * ((width+1+15)/16);
   //w->save_area_size = wsize;
   //fillwords ((USHORT *)w->border_width, 1, 4);
   //memset(((USHORT *)w->border_width),1,(4));
   //vmem_alloc (window_screen_mem, wsize, &w->save_area);
return w_v;
}
#endif
/* **************************************** */
#ifdef FUCKUP
void  print_string (window_t w_v, SHORT row, SHORT col, char const* s)
{
/*  similar to put_string, but handles CR LF embedded in strings
    this is called by OS2_REDBOX system
*/
//struct window_rec *w;
struct dt { byte letter, attribute; } *display;
char const* scan= s;
//SHORT length = strlen(s);
SHORT mycol = 0;

//w= (window_rec *)Mem::vmem_trans (w_v);

//display= ((struct dt *)Mem::vmem_trans (w->save_area)) + (row * w->size.x) + col;

while (*scan) {
   if (mycol++ == (w->size.x-3))    /* move 1 row down */
      display= ((struct dt *)Mem::vmem_trans(w->save_area))+((row+1)* w->size.x)+col+8;
   if (*scan == 0x0d || *scan == 0x0a) {
      *(scan++);                          /* discard char */  
      (display++)->letter = ' ';        /* substitute a space */
   }
   else
     (display++)->letter = *(scan++);
}
if (w->status_word & 1) {
   twocorner box;
   box.ul.y= box.lr.y= w->location.ul.y + row;    
   box.lr.y = box.ul.y + row + 1;
   box.ul.x= w->location.ul.x + col;
   box.lr.x= box.ul.x + (scan - s)-1;
//   display_window (w_v, &box);
   }
}
#endif
/* **************************************** */
//void window_border_size (window_t w_v,USHORT bwidth[4])
//{
//struct window_rec *w= (window_rec *)Mem::vmem_trans (w_v);
//memcpy (w->border_width, bwidth, 4*sizeof(USHORT));
//}

/* **************************************** */
#ifdef FUCKUP
void shade_win_row (window_t w_v, SHORT row, byte off_color, byte on_color, SHORT b_first, SHORT b_last)
{
struct window_rec *w= (window_rec *)Mem::vmem_trans (w_v);
struct dt { byte letter, attribute; } *display;
SHORT spaces, loop= 0;
display= ((struct dt *)Mem::vmem_trans (w->save_area)) + (row * w->size.x) + 1;
//spaces= w->size.x - (w->border_width[LEFT]+w->border_width[RIGHT]);
spaces= w->size.x - (1+1);

if (b_first < 0) 
   b_first= 0;
else if (b_first > spaces) 
   b_first= spaces;
if (b_last < 0) 
   b_last= 0;
else if (b_last > spaces) 
   b_last= spaces;
while (loop < b_first) 
   display[loop++].attribute= off_color;
while (loop < b_last) 
   display[loop++].attribute= on_color;
while (loop < spaces) 
   display[loop++].attribute= off_color;
}
#endif
/*********************************************************************/

