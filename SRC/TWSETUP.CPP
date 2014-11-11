/********************************************************************
file: WSETUP.CPP              Copyright 1986-1991 by Dlugosz Software
  part of TOW Text Output Windows
  meta_window members -- window system initialization
********************************************************************/
#include "usual.h"
#include "tow.h"
#include <string.h>
#include "misc.h"      // fillwords

#include "test.h"    

#define DESKTOP             // clears/restores screen on startup/shutdown

extern void err_exit(const char *msg, unsigned short number); //err_hand.cpp

#ifndef __OS2__
extern "C" void fillwords (void far* dest, unsigned val, unsigned count);
#else
extern "C" void fill_screen(unsigned fillval);
extern "C" void screen_restore();
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>
#endif

//twocorner meta_window::range= {{0,0},{79,24}};
char* meta_window::term_mess= 0;
//unsigned short* meta_window::screen_save= 0;

#ifndef __OS2__
unsigned short* meta_window::address= 0;
#else
unsigned short _far16* meta_window::address= 0;
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#ifndef __OS2__                 // OS/2 setup in tow_os2.cpp
bool meta_window::setup()
{     // set up structure based on current mode
unsigned mode= video_int (0x0f00);
width= mode >> 8;
display_iscolor= FALSE;
switch (mode & 0xff) {
   case 3:  //normal color
      display_iscolor= TRUE;      
      //drop through
   case 2:  //color card, mono screen
      address= (unsigned short*)0xb8000000;
      break;
   case 7:  //monochrome card
      address= (unsigned short*)0xb0000000;
      break;
//   default:  // assume non-text mode
//      return FALSE;
   default:    // assume exotic text mode text
      display_iscolor= TRUE;
      address= (unsigned short*)0xb8000000;
   }
height= 1+ *(byte *)(0x00400084L);
if (height==1) height= 25;  //byte is 0 on older cards.
range.lr.x= width-1;
range.lr.y= height-1;
return TRUE;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

extern "C" void abort();
extern "C" int write (int, const char*, int);
extern "C" unsigned read_cursor_position();  //video_int() doesn't help me here

meta_window::meta_window()
{
static const char error_mess[]= "Unknown video mode.  Cannot initialize.\r\n";
//#ifdef TESTING
//ofstream dout("logfile");
//#endif
if (!setup()) {
   write (2, error_mess, sizeof error_mess -1);
   abort();
   }
fillval= 0x0020;  //black space
display_list= 0;
term_mess= 0;
                            //save screen image and move cursor off screen
//screensize= height*width;
//screen_save= new unsigned short[screensize];
//memset(screen_save, 0x0000, screensize);
//screensize= 2* height*width;                       // mod 1-1-95
//screen_save= new unsigned char[screensize];        // cvt to byte buf
//memset(screen_save, 0x00, screensize);

old_cursor_pos= read_cursor_position();          
video_int (0x0200,0,0,(height<<8));  //move cursor off screen
//memcpy (screen_save, address, 2*screensize);
//memcpy (screen_save, address, screensize);
                            
#ifdef DESKTOP                       //clear the screen
#ifndef __OS2__
fillwords (address, fillval, screensize);
#else
fill_screen(fillval);
#endif
#endif
//DBG_WIN(dout<<" in meta_window ctor"<<endl);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

meta_window::~meta_window()
{
                                           //restore old screen and cursor
if (!screen_save) {  //cannot restore
   fillwords (address, 0x0720, screensize); // this will fail for OS/2 !!
  }
else {
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
    rc = VioWrtCellStr(screen_save,screensize, 0, 0, 0);
//    screen_restore();
    DBG_VID(dout<<"meta_window dtor  VioWrtCellStr rc = "<<rc<<endl);
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
#ifdef __OS2__
//    rc = VioShowBuf(0, vidbufsize, 0);   
    DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
    video_int (0x0300,0,0,old_cursor_pos);  //restore cursor pos
    DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
#else
   video_int (0x0200,0,0,old_cursor_pos);  //restore cursor pos
#endif
 }                                   
   delete[] screen_save;                // MOD 5-9-94 !!
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
if (term_mess != NULL) {
   err_exit(term_mess, 10);
//   write (1, term_mess, strlen(term_mess));
}
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}







