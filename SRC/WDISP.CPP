/*****************************************************
file: WDISP.C       Copyright 1989 by Dlugosz Software
   window display functions, for IBM
*****************************************************/
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include <stdlib.h>   // malloc in screen swap
#include "window.h"
#include "basewin.h"

#include "test.h"    

extern meta_window Screen;
static char* screen_buf = NULL;

/********************************************************************* */
void window_redraw_all()         // called by shell_os2  only
{
{                                         /* part 1-- clear the zone */
//register USHORT row, loop;
//SHORT width;
CHAR chFill[2] = {'\x0f', ' '};
//USHORT col = box->ul.x;
//USHORT col = Screen.range.ul.x;

//width= box->lr.x - box->ul.x +1;
//width= Screen.range.lr.x - Screen.range.ul.x +1;

//for (row= box->ul.y; row <= box->lr.y; row++) {
//for (row= Screen.range.ul.y; row <= Screen.range.lr.y; row++) {
//   loop= width;
//   while (width--){
//     rc = VioWrtCellStr(chFill, sizeof(chFill), row, col, 0);
//    }
  rc = VioScrollUp(0, 0, -1, -1, -1, chFill, 0);
//}                                         /* end for loop   */
}                                         /* end part 1   */

{                                        /* part 2-- redraw the zone */
//vptr w_v= vid.get_topwin();
//vptr next;
//struct window_rec *w;

//  Screen.regenerate();

//while (w_v) {
//   w= (struct window_rec *)Mem::vmem_trans (w_v);
//   next= w->next;
//   display_window (w_v, box);
//   w_v= next;
//   }                                       /* end while  */
}                                          /* end part 2   */
}

/*****************************************************************
      called by SHOW_DOS_SCREEN to show old screen and restore KEd 
      screen when user presses any key.
********************************************************************/
void screen_swap (void)
{
  static bool buf_ok = FALSE;
  static bool save_screen = TRUE;
  USHORT bufsize = Screen.get_vidbufsize();

  if(!buf_ok) {
    screen_buf = (char*)malloc(bufsize);
   buf_ok = TRUE;
  }
  if(save_screen) {
     rc = VioReadCellStr(screen_buf, &bufsize, 0,0,0);
     DBG_WIN1(dout<<"screen swap save VioReadCellStr rc = "<<rc<<endl);
     rc = VioWrtCellStr((char*)Screen.get_save_buf(), bufsize, 0,0,0);
     DBG_WIN1(dout<<"screen swap save VioWrtCellStr rc = "<<rc<<endl);
//     set_cursor(63, 0);                        /* hide cursor   */
     video_int (0x0200,0,0,0);  //hide cursor 
     save_screen = FALSE;
  }
  else {
     rc = VioWrtCellStr(screen_buf, bufsize, 0,0,0); 
     DBG_WIN1(dout<<"screen swap restore VioWrtCellStr rc = "<< rc<<endl);
     DBG_WIN1(dout<<"screen_swap set_cursor OK "<<endl);
     video_int (0x0200,0,0,0);  //hide cursor 
     save_screen = TRUE;
  }
}
