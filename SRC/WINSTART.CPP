/*****************************************************
file: WINSTART.C    Copyright 1989 by Dlugosz Software
   window init,shutdown and screensave
*****************************************************/
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "window.h"
#include <stdlib.h>   // malloc in screen swap
#include "err_hand.h"

//#include "tow.h"     // mod 1-27-94
#include "basewin.h"     // mod 1-27-94

//extern meta_window Screen;
//static USHORT   rc;
static char* screen_buf = NULL;

#include "test.h"    
/* **************************************** */

//bool window_init ()     /* set up memory and initalize system */
//{                        
//   window_mem= vmem_create_space();            // this will trap if you don't
//return TRUE;        
//}
#ifdef FUCKUP
//bool Vid::init(void)
bool vid_init(void)
{
VIOMODEINFO   viomode;
VIOCONFIGINFO vioconfig;
//USHORT _far16 *disp_address; 
//USHORT _far16 **temp = &disp_address;        // BCC optimize workaround
USHORT mode = 0;
USHORT w_mode;

    vioconfig.cb = sizeof(vioconfig);
    rc = VioGetConfig(0, &vioconfig, 0);
    if(rc != 0)
      err_exit("Unknown VIDEO adapter", rc);

/* #define COLORS_2        0x0001   VIOMODEINFO.color constants 
   #define COLORS_4        0x0002
   #define COLORS_16       0x0004 */

    viomode.cb = sizeof(viomode);
    rc = VioGetMode(&viomode, 0);
    if(rc != 0)
       err_exit("Unknown VIDEO mode", rc);
                                         // set disp_address ptr, vidbufsize
//    rc = VioGetBuf((PULONG16)temp , &usvidbufsize, 0);
//    if(rc != 0)
//       err_exit(" Error in winstart.c  VIOGETBUF", rc);

    mode = vioconfig.adapter;                                // set globals
//    saved_cursor= Vid::get_cursor();

/* #define DISPLAY_MONOCHROME      0x0000 VIOCONFIGINFO.adapter constants 
   #define DISPLAY_CGA             0x0001
   #define DISPLAY_EGA             0x0002
   #define DISPLAY_VGA             0x0003
   #define DISPLAY_8514A           0x0007
   #define DISPLAY_IMAGEADAPTER    0x0008
   #define DISPLAY_XGA             0x0009 */

/* #define MONITOR_MONOCHROME      0x0000  VIOCONFIGINFO.display constants 
   #define MONITOR_COLOR           0x0001
   #define MONITOR_ENHANCED        0x0002
   #define MONITOR_8503            0x0003
   #define MONITOR_851X_COLOR      0x0004
   #define MONITOR_8514            0x0009
   #define MONITOR_FLATPANEL       0x000A
   #define MONITOR_8507_8604       0x000B
   #define MONITOR_8515            0x000C
   #define MONITOR_9515            0x000F
   #define MONITOR_9517            0x0011
   #define MONITOR_9518            0x0012  */

   switch (mode) {
      case 0:                                 // MDA adapter
             if(vioconfig.display == 0x0000)  // monochrome screen
             w_mode = 2;                  
             break;
      case 1:
             w_mode= 1;
//           if((vioconfig.display == 0x0000) || (vioconfig.display == 13))
             if(vioconfig.display == 0x0000)
                w_mode = 2;  /* color card with mono screen */
             break;
      case 2:                          
      case 3:      /* color card */       /* OS/2 2.0 returns 8 for vga */
      case 8:                          
             w_mode= 1;
             if(vioconfig.display == 0x0000)
                w_mode = 2;  /* color card with mono screen */
             break;
      case 7:
             w_mode= 2;      /* BW card */
             break;                    /* add more cases here, if needed */
      case 10:                         /* OS/2 2.1 returns 10 for SVGA */

      default:
             w_mode= 1;      /* assume color SVGA card */
             break;                        
         //    return FALSE;                   /* unknown card */
      }

//    vid_buf_length = viomode.buf_length;

//#ifdef TESTING
  DBG_WIN1(dout<<endl<<" viomode.rows = "<< viomode.row<<endl);
  DBG_WIN1(dout<< "viomode.cols   = " <<viomode.col<<endl);
  DBG_WIN1(dout<< "vid_buf_length  = "<< viomode.buf_length<<endl);
//  writelog( "vid_full_length = %d\n", viomode.full_length);
//  writelog( "video buf size  = %d\n", usvidbufsize);

//  pause();
//  err_exit(" TEST", 14);
//#endif

return TRUE;
}
#endif

/*****************************************************************
      called by SHOW_DOS_SCREEN to show old screen and restore KEd 
      screen when user presses any key.
********************************************************************/
//void Vid::screen_swap (void)
void screen_swap (void)
{
  USHORT bufsize = Screen.get_vidbufsize();
  static bool buf_ok = FALSE;
  static bool save_screen = TRUE;

  if(!buf_ok) {
    screen_buf = (char*)malloc(bufsize);
   buf_ok = TRUE;
  }
  if(save_screen) {
     rc = VioReadCellStr(screen_buf, &bufsize, 0,0,0);
     DBG_WIN1(dout<<"screen swap save VioReadCellStr rc = "<< rc<<endl);
     rc = VioWrtCellStr(Screen.get_save_buf(), bufsize, 0,0,0);
     DBG_WIN1(dout<<"screen swap save VioWrtCellStr rc = "<<rc<<endl);
//     set_cursor(63, 0);                        /* hide cursor   */
     video_int (0x0200,0,0,0);  //hide cursor 
     save_screen = FALSE;
  }
  else {
     rc = VioWrtCellStr(screen_buf, bufsize, 0,0,0); 
     DBG_WIN1(dout<<"screen swap restore VioWrtCellStr rc = "<< rc<<endl);
     DBG_WIN1(dout<<"screen_swap set_cursor OK  "<<endl);
     save_screen = TRUE;
  }
}




