/*****************************************************
file: PICKLIST.C    Copyright 1989 by Dlugosz Software
   a general purpose pick list driver
*****************************************************/

#include "usual.h"
#include "window.h"
#include "keycodes.h"
#include "getkey.h"
#include "picklist.h"

#include "basewin.h"

extern basewin *picw;
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//SHORT picklist (struct picklist_portal *p, window_t w, struct picklist_thunk *t, SHORT count, SHORT location)
SHORT picklist (struct picklist_portal *p, struct picklist_thunk *t, SHORT count, SHORT location)
{
SHORT const col= p->startcol;
SHORT const width= p->columns;
SHORT const length= p->rows;
SHORT row, index, first= 0;
//char const* s;             // mod 7-27-94
byte const* s;

/* first, draw all the choices */
row= p->startrow;
for (index= 0;  index < length;  index++) {
   s= (*t->fetch_choice) (index);
   if (!s) break;  /* end of the list */
//   put_string_len (w, row, col, s, width);
   picw->put(row, col, s, width);
   row++;
   }
row= p->startrow;
index= 0;  /* ready to go */
for (;;) {
   SHORT key;
   if (t->tellme) (*t->tellme) (index);
   /* display cursor and get key */
//   highlight (w, row, col, p->cursor_color, width);
   picw->highlight(row, col, p->cursor_color, width);
   do {
      key= getkey();
      } while (t->keyfilter && !(*t->keyfilter)(key));
//   highlight (w, row, col, p->normal_color, width);
   picw->highlight(row, col, p->normal_color, width);

   switch (key) {                                /* process key */
      case kESC: return -1;
      case GREY_Enter:
      case Enter: return index;
      case kUp:
         if (index > 0) {
            index--;
            if (index < first) {
               s= (*t->fetch_choice)(index);
//               scroll_window (w, DOWN, 1);
               picw->scroll(Down, 1);
               first--;
//               pad_window (w, row, col, ' ', width);
//               picw->pad(row, col, ' ', width);
               picw->put_fill(row, col, "", width);
//               put_string_len (w, row, col, s, width);
               picw->put(row, col, s, width);
               }
            else row--;
            }
         break;
      case kDown:
         if (index < count-1) {
            index++;
            if (index >= first+length) {
               s= (*t->fetch_choice)(index);
//               scroll_window (w, UP, 1);
               picw->scroll(Up, 1);
               first++;
//               pad_window (w, row, col, ' ', width);
//               picw->pad(row, col, ' ', width);
               picw->put_fill(row, col, "", width);
//               put_string_len (w, row, col, s, width);
               picw->put(row, col, s, width);
               }
            else row++;
            }
         break;
//      case kLeft: case kRight: break; /* don't let keymove_window() see these */
//      default:
//         keymove_window (w,location,key);
      }
   }
#pragma warn -par        // eliminate "location " not used
}
#pragma warn +par
