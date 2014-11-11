/********************************************************************
        VERSION.CPP    prompt box shows what it is

**********************************************************************/
#include "usual.h"
#include "getkey.h"
#include "setting.h"

#include "basewin.h"

#define VERSION "OS/2 Version 2.10"

#define colors settings.colors[DIALOG_COLORS]

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
//void show_version (void)
void ked_version (void)
{
basewin verw (21, 61,(Screen.get_screen_height()-20)/2,(Screen.get_screen_width()-60)/2);
verw.clearwin(BLACK, _LGREY, ' ');
verw.border(window_core::Double);     
verw.put_centered (1,"The Kudzu Editor (KEd)   OS/2 Version 2.17 E ");
verw.put_centered (2,"Copyright 1992-1995 by Kudzu Software");
verw.put_centered (3,"Portions Copyright 1991 Dlugosz Software");
verw.put_centered (4,"Kudzu Software");
verw.put_centered (5,"1452 North Ave Atlanta, GA 30307");
verw.put_centered (6,"Compuserve ID: 76254,764");
verw.put (8,1,  "The Kudzu Editor (KEd) is a general purpose text editor.");
verw.put (9,1,  "There is extensive HyperText help and user configuration.");
verw.put (10,1, "You can cut and paste between files using lines or ");
verw.put (11,1, "columns, shell out to your compiler or other tools. ");
verw.put (12,1, "KEd is a full 32 bit application for OS/2 2.x and WARP");
verw.put (13,1, "that uses multiple threads and supports HPFS filenames. ");
verw.put (15,1, "KEd is distributed under the Shareware system.");
verw.put (16,1, " ");
verw.put (17,1, "PLEASE REGISTER! Registration is $20 for personal use");
//verw.put (17,1, "   BETA TEST VERSION  DO NOT DISTRIBUTE        ");
verw.put_centered (19,"press any key to continue");
verw.show();

getkey ();
}
