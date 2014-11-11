/*****************************************************
file: VIEW_WIN.C    Copyright 1989 by Dlugosz Software
   part of the sheet viewer.  These functions are
   using window functions that are optionally linked.
   These functions do not need to be linked
   unless they are used.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "help.h"             // help for window move and size
#include "keycodes.h"
#include "getkey.h"           // getkey()
#include "misc.h"             /* need fillwords() */
#include "setting.h"          /* need color symbols */
#include "status.h"           // status functs

#include "basewin.h"
#include "test.h"
      
/* **************************************** */
/* stub out support for moving windows   1-9-93   */

#ifdef MOVE_WIN

SHORT window_move(void)
{
//#ifndef NO_HELP
DEF_HELP (WINDOW_MOVE,1,20,5);
NEW_HELP;
//#endif
//status_note("Window move mode: Press arrow keys to move, F1 = Help, ESC to exit");

  USHORT key = 0x0000;
  while((key = getkey()) != kESC) {
     switch(key) {
        case kUp:
                   view_move_window(UP, 1);
                   break;
        case kDown:
                   view_move_window(DOWN, 1);
                   break;
        case kLeft:
                   view_move_window(LEFT, 1);
                   break;
        case kRight:
                   view_move_window(RIGHT, 1);
                   break;
     }
  }
OLD_HELP;
//status_note (NULL);
return 0;
}

/************************************************************************/
SHORT window_size(void)
{
//#ifndef NO_HELP
DEF_HELP (WINDOW_SIZE,1,20,5);
NEW_HELP;
//#endif

//status_note ("Window size mode: Press arrow keys to size, F1 = Help, ESC to exit");

  USHORT key = 0x0000;
  while((key = getkey()) != kESC) {
     switch(key) {
        case kUp:
                   view_size_window(UP, 1);
                   break;
        case kDown:
                   view_size_window(DOWN, 1);
                   break;
        case kLeft:
                   DBG_WIN2(dout<<" in window_size resize LEFT "<<endl);
                   view_size_window(LEFT, 1);
                   break;
        case kRight:
                   DBG_WIN2(dout<<" in window_size resize RIGHT "<<endl);
                   view_size_window(RIGHT, 1);
                   break;
     }
  status_redraw();
  }
OLD_HELP;
//status_note (NULL);
return 0;
}

/************************************************************************/
void view_move_window (enum udlr direction,SHORT count)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
            //move_window (v->w, direction, count);
v->flags |= 4;
vwptr[v->view_num]->move(direction, count);
}

#else 
void view_move_window (enum udlr direction,SHORT count)
{
#pragma warn -par
}
#pragma warn .par
#endif

//bool keymove_window (window_t w, SHORT location, SHORT key)
//{
//#pragma warn -par
//return FALSE;        // say NO, window was not moved, use the key
//}
//#pragma warn .par


/* **************************************** */
#ifdef MOVE_WIN
void view_size_window (enum udlr direction,SHORT count)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
SHORT  newsize;
USHORT fillval= (v->colors[0] << 8)|' ';
DBG_SIZE1(dout<<" view_size_window START v->length = "<<v->length<<" v-width = "<<v->width<<" count = "<<count<<endl);
switch (direction) {
   case UP:  count *= -1;
   case DOWN:
      newsize= v->length + count;
      if (newsize < 1 || newsize >= 64) 
         return;
      if (count > 0) {                     /* bigger */
//         SHORT  row= v->length-1;
         SHORT  row= v->length;
         DBG_SIZE1(dout<<" view_size row = "<<row<<endl);
         SHORT  word= row/16;
         SHORT  bit= row%16;
         USHORT maxsize= ((VMEM_PAGESIZE-4)/2)/(v->width+2)-(1+v->top_border)-1;
         if (newsize > (SHORT )maxsize) 
            newsize= maxsize;
         v->touches[word] |= (((USHORT)0xffff) << bit);
         while (++word < 4) 
            v->touches[word] = 0xffff;
         }                                                // end bigger
      v->length= newsize;
      DBG_SIZE1(dout<<" view_size NEW v->length = "<<v->length<<endl);
//      resize_window (v->w, 1, (v->length= newsize) + 1 + v->top_border, fillval);
      vwptr[v->view_num]->change_size(Down, count, fillval);
      DBG_SIZE1(dout<<" view_size moving status win Down count = "<<count<<endl);
      statwptr[v->view_num]->move(Down, count);    // TEMP TEMP
      break;
   case LEFT:  count *= -1;
   case RIGHT:
      newsize= v->width + count;
      DBG_SIZE1(dout<<" view_size NEW v->width = "<<newsize<<endl);
      if (newsize < 1) return;
      if (count > 0) {                     /* bigger */
         USHORT maxsize= ((VMEM_PAGESIZE-4)/2)/(1+v->length+v->top_border)-2-1;
         if (newsize > (SHORT )maxsize) 
             newsize= maxsize;
         fillwords (v->touches, 0xffff, 4);
         }
      v->width = newsize;
//      resize_window (v->w, 2, (v->width= newsize) + 2, fillval);
      vwptr[v->view_num]->change_size(Right, count, fillval);
      statwptr[v->view_num]->change_size(Right, count, (attr(YELLOW,_RED)<<8) | ' ');
      break;
  }
v->flags |= 4;
vwptr[v->view_num]->view_border(window_core::Double,v->colors[BORDER_COLOR]);
view_flush (active_view);  /* this line is debatable */
}
#endif
