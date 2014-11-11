/*****************************************************
file: VIEW_DRW.C    Copyright 1989 by Dlugosz Software
   view drawing stuff
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "status.h"
#include "misc.h"                    /* need fillwords() */
#include <string.h>                     /* need memcpy() */
#include "getkey.h"                    /* need keyready() */

#include "basewin.h"
#include "test.h"

#define viewset settings.viewer_defaults

//static struct line_header empty_header= {0,0};
/* **************************************** */

void view_clear (vptr view_v)
{
/* the sheet has been cleared, so all lines and marks are now
   invalid.  Also, the view should be reset */
while (view_v) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
   view_v= v->next;
   v->firstrow= 0;
   v->firstcol= 0;
//  fillwords (v->touches, 0xffff, 4);
  fillwords (v->touches, -1, 4);
//   memset(v->touches,0xffff,(2*4));
   v->flags |= 4;
   v->cursor= new_mark_any (v->sheet);           // mycursor needed here ?
   v->block_mode= view_rec::bk_stream;
   v->block_active= view_rec::bk_undefined;
                          // DON'T THINK I need THIS with TOW  ???  5-29-94
//   shade_all (v);         /* clear block attributes */
   }
}

/********************************************************************
     called by sheet for any line
      mark line for redraw if it exists in this window.
      loop for all views linked to this one. 
**********************************************************************/
void view_linetouch (vptr view_v, ULONG line)   
{
while (view_v) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
   if (line >= v->firstrow) {
      USHORT row= (USHORT)(line - v->firstrow);  // window row
      if (row < v->length) {                     // yes, it IS in this window 
         SHORT word= row/16;
         SHORT bit= row%16;
         v->touches[word] |= 1 << bit;
         v->flags |= 4;
      }
   }
   view_v= v->next;
}

}

/* **************************************** */
static void  touch_to_end (struct view_rec *v, USHORT row)
{
                                      /* dummy out- clear the whole thing */
if (row >= v->length) 
   return;                        /* after this window */
fillwords (v->touches, 0xffff, 4);
//memset(v->touches,0xffff,(2*4));

v->flags |= 4;         // mark the view for redraw
}

/* **************************************** */
void view_lineadd (vptr view_v, ULONG line)
{
/* A line has been inserted at the stated line.  I could do the insertion
   in the window bump down the touches, but for now I'll just clear the
   touches to have the bottom redrawn.  */
while (view_v) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
   if (line >= v->firstrow)       /* visible in this window */
      touch_to_end (v, (USHORT)(line - v->firstrow));
   else                         /* line inserted before this window */
      v->firstrow++;            /* keep the window in the same spot */
   view_v= v->next;
   }
}

/* **************************************** */
void view_linedel (vptr view_v, USHORT line, SHORT count)
//void view_linedel (vptr view_v, ULONG line)
{
while (view_v) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
   while(count--) {
   if (line >= v->firstrow)           /* this line is in the display window */
      touch_to_end (v, (USHORT)(line - v->firstrow));
   else                                /* line deleted before this window */
      v->firstrow--;    // what ??
   line++;
   }
   view_v= v->next;
   }
}

void view_flush (vptr view_v)
{
/******************************************************************
*  PURPOSE: Flush all changes in the indicated view.  
*           Any pending changes are  updated to the viewer's window.
*  CALLS:      shade_win_row
*              put_win_row
*              view_redraw_block
*  CALLED BY:  view_scroll
*              do_replace in search.c
*              view_cursor_in_range
*              flush_while_waiting
*******************************************************************/
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
if (! (v->flags & 4))                        // why bother?  no changes 
   return;     

vptr sheet= v->sheet;
struct sheet *cur= (struct sheet *)Mem::vmem_trans (v->sheet);

//DBG_VIEW1(dout<<" view_flush active_view = "<<view_v<<" active_sheet = "<<(v->sheet)<<endl);
           //byte off_color = 0
byte on_color = 0;
bool block;
enum view_rec::block_mode_t mode= v->block_mode;
SHORT b_first = 0, b_last = 0;           // block column markers
struct mark_rec b_begin, b_end;
struct line_header *h;
SHORT len;

v->flags &= ~4;
if (TRUE == (block= v->block_active == view_rec::bk_on)) {
   on_color= v->colors[2];
                      //   off_color= v->colors[0];
//   b_begin= *get_mark (v->block_begin);
//   b_end= *get_mark (v->block_end);
   b_begin= *get_mark_fast (cur,v->block_begin);
   b_end= *get_mark_fast (cur,v->block_end);

   if (mode == view_rec::bk_lines) {
      mode= view_rec::bk_stream;
      b_begin.col= b_end.col= 0;
      }
   else if (mode == view_rec::bk_columns) {
      b_first= b_begin.col;
      b_last= b_end.col;
      }
   }
/* all the needed information is copied out of v.  I can now "forget" it. */

SHORT w_row= v->top_border;
ULONG line= v->firstrow;
SHORT startcol= v->firstcol;
USHORT touches[4];
USHORT *word= touches;
USHORT mask= 1;
CHAR *tempstr = "";
//ULONG loop= v->length;
USHORT loop= v->length;

memcpy (touches, v->touches, sizeof(touches));
//fillwords (v->touches, 0, 4);                    /* all clear */
memset(v->touches,0,(2*4));                    /* all clear */

//DBG_WIN2(dout<<" view_flush v->length = "<<v->length<<endl);

while (loop--) {
//    DBG_WIN1(dout<<" v_flush looping through viewline = "<<loop<<endl);
    if (*word & mask) {                                /* update this line */
    h= line_fetch (sheet, line);
//    h= line_fetch2 (line);
    DBG_WIN2(dout<<"v_flush sheet line = "<<line);
    len= h->length - startcol;
    if (len < 0) len= 0;
    if (block) {
       if (line >= b_begin.row && line <= b_end.row) {
          DBG_WIN1(dout<<" view_flush calling shade_block "<<endl);
          if (mode == view_rec::bk_stream) {  /* check for special cases */
             if (line == b_begin.row && line == b_end.row) {
                b_first= b_begin.col;
                b_last= b_end.col-1;
                }
             else if (line == b_begin.row) {
                b_first= b_begin.col;
                b_last= 32000;
                }
             else if (line == b_end.row) {
                b_first= 0;
                b_last= b_end.col;
                }
             else {                       // b_beginrow > line < b_end.row
                b_first= 0;
                b_last= 32000;
                }
          }                                // end if mode = stream
          else {                               /*     column mode */
             if (line == b_begin.row || line == b_end.row) {
                shade_block (active_view, TRUE,0,0);
                block= FALSE;
              }                   // endif
          }                      // end else
//       shade_win_row (w, w_row, off_color, on_color, b_first-startcol, b_last-startcol);
         DBG_WIN1(dout<<" view_flush highlight row = "<<w_row<<endl);
         vwptr[v->view_num]->highlight(w_row,b_first-startcol, on_color,
                                     (b_last-startcol) - (b_first-startcol));
       }                         // end if line is visible
    }                            // end if word & mask
    if(h->length == 0)
       tempstr = "";
    else
       tempstr = (char *)(h+1);

//    if (v->flags&8) {                            /* use tabs? */
//       vwptr[v->view_num]->put_w_tabs(w_row, 1, startcol, startcol + tempstr, len);
//    }
//    else {
       DBG_WIN2(dout<<" v_flush w_row = "<<w_row<<" str =°"<<tempstr<<"°"<<endl);
       vwptr[v->view_num]->put_fill(w_row, 1, startcol + tempstr, len);
//    }
//    put_win_row (w, startcol + (char *)(h+1), len, w_row);
   }           // end if word & mask
   w_row++;
   line++;
   if (0 == (mask<<=1)) {
      word++;
      mask= 1;
      }
   }              // end while loop
   DBG_WIN3(dout<<" view_flush calling status_show "<<endl);
//   status_redraw();
   statwptr[v->view_num]->show(); // TEMP mod 9-3-94
}

/* **************************************** */

static void  flush_while_waiting(void)
{
               // update as many views while waiting for a key to be pressed 
vptr v_w= top_view;
vptr next_view;
struct view_rec *v;

for (;;) {
   if (key_ready()) return;      /* quit if something typed */
   v= (struct view_rec *)Mem::vmem_trans (v_w);
   next_view= v->under;          /* next one down */
   if (v->flags & 4) 
       view_flush (v_w);
   if (next_view == top_view) 
       return;                   /* looked at them all */
   v_w= next_view;
   }
}

/****************************************************************/

void view_cursor_inrange (struct view_rec **view ,SHORT *wrow, SHORT *wcol)
{
/******************************************************************
Makes sure the cursor in the active view is in range, that is, in
the area shown in the window.  The answer areas pointed to by the
parameters are set to the translated address of the active view,
and the row and col (in window relative coordinates) of the
cursor position. 
******************************************************************/
             //struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct view_rec *v;
v = my_active_view;              // mod 8-27-93
            //struct sheet *s= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *s = my_active_sheet;       // mod 8-25-93
            //struct mark_rec *m= get_mark (v->cursor);
struct mark_rec *m= get_mark_fast (s,v->cursor);
//v->mycursor.row = m->row;
//v->mycursor.col = m->col;
ULONG row = m->row;               // MOD 9-18-94 for block_read()
SHORT col= m->col - v->firstcol;                         
SHORT top_border= v->top_border;

*view= v;
DBG_WIN1(dout<<" view_cursor_inrange line "<<__LINE__<<" cursor col = "<<m->col<<" row = "<<m->row<<" v->firstrow = "<<(v->firstrow)<<endl);
DBG_WIN1(dout<<" view_cursor_inrange line "<<__LINE__<<" top_border = "<<top_border<<" row = "<<row<<endl);

                            /* first visible row is below current cursor */   
if (v->firstrow > m->row) {     
   DBG_WIN1(dout<<endl<< "view_cursor_inrange, calling view_scroll(DOWN)");
   DBG_WIN1(dout<< " v->firstrow= "<<v->firstrow<< " m->row= "<<m->row<<endl);
   view_scroll (DOWN, v->firstrow - m->row);
   row= 0;
   }
else {                   /* first visible row is above current cursor */   
   row=  m->row - v->firstrow;
   if (row >= v->length) {
      DBG_WIN1(dout<<endl<< "view_cursor_inrange, calling view_scroll(UP)"<<endl);
      DBG_WIN1(dout<< "v->firstrow = "<<v->firstrow<<"  m->row= "<<m->row<< " row = "<<row<<endl);
      view_scroll (UP, 1+ row - v->length);
      row= v->length-1;
   }
 }
if (col < 0) {
   DBG_WIN1(dout<<endl<<"in view_cursor_inrange, calling view_scroll(RIGHT)");
   DBG_WIN1(dout<<" v->firstcol= "<<v->firstcol<<"  m->col= "<<m->col<<endl);
   view_scroll (RIGHT, -col);
   col= 0;
   }
else if (col >= v->width) {
   DBG_WIN1(dout<<endl<<"in view_cursor_inrange, calling view_scroll(LEFT)");
   DBG_WIN1(dout<<" v->firstcol= "<<v->firstcol<< " m->col= " <<m->col<<endl);
   view_scroll (LEFT, 1+ col - v->width);
   col= v->width-1;
 }
                                  
//view_flush (active_view);
         //  view_coords are transformed here from 0,0 based window coords to
         //    view coords which are based 1,1 
*wrow = (SHORT)row+top_border;               /* return window coordinates */
*wcol = 1+col;
DBG_WIN1(dout<<dec<<" view_cursor_inrange line "<<__LINE__<<"  cursor col = "<<(*wcol)<<" row = "<<(*wrow)<<endl);
}

/* **************************************** */
//USHORT view_getkey(void)
struct key_rec& view_getkey(void)
{
struct view_rec *v;
struct sheet *s = my_active_sheet;       // sheet only used for STATUS flags
struct mark_rec *m;
basewin* vwp;
SHORT row;
SHORT col;
byte color;

view_cursor_inrange (&v, &row, &col);           // sets "*v", row, col
m = get_mark_fast (s,v->cursor);
//status_numlockmode ();
status_quick_redraw(m->row +1, m->col +1, s->changed);

vwp = vwptr[v->view_num];

DBG_WIN1(dout<<" view_getkey cursor row = "<<row<<" col = "<<col<<endl);

if (v->block_active == view_rec::bk_on) {
   bool in;
   if (v->block_mode == view_rec::bk_stream)
      in= !mark_greater(v->block_begin,v->cursor) && mark_greater(v->block_end, v->cursor);
   else {
      struct mark_rec b,e,c;
      b= *get_mark (v->block_begin);
      e= *get_mark (v->block_end);
      c= *get_mark (v->cursor);
      if (v->block_mode == view_rec::bk_columns)
         in= (c.row >= b.row && c.row <= e.row && c.col >= b.col && c.col < e.col);
      else in= c.row >= b.row && c.row < e.row;
      }
   if (in) color= v->colors[3];
   else color= v->colors[1];
   }
else                    /* no block */
   color= v->colors[1];
flush_while_waiting ();                        
return window_getkey (vwp, row, col, color);
}
