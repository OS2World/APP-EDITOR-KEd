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

extern "C" void fill_screen(unsigned fillval);
extern "C" void screen_restore();
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>

//twocorner meta_window::range= {{0,0},{79,24}};
char* meta_window::term_mess= 0;
unsigned short _far16* meta_window::address= 0;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#ifndef __OS2__                 // OS/2 setup in tow_os2.cpp
bool meta_window::setup()
{     // set up structure based on current mode
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

old_cursor_pos= read_cursor_position();          
video_int (0x0200,0,0,(height<<8));  //move cursor off screen
                            
#ifdef DESKTOP                       //clear the screen
fill_screen(fillval);
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
    DBG_VID(dout<<"meta_window dtor  VioWrtCellStr rc = "<<rc<<endl);
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
    DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
    video_int (0x0300,0,0,old_cursor_pos);  //restore cursor pos
    DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
 }                                   
   delete[] screen_save;                // MOD 5-9-94 !!
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
if (term_mess != NULL) {
   err_exit(term_mess, 10);
}
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}

