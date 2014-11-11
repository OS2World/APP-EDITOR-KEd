/*****************************************************
File: SHELL.C       Copyright 1990 by Dlugosz Software
   OS/2 Shell function
*****************************************************/
#define INCL_DOS
#define INCL_NOPMAPI
#define INCL_KBD
#define INCL_DOSPROCESS
#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "redbox.h"
#include "misc.h"
#include "setting.h"
#include "sheet.h"             // need sheet filename
#include "view.h"
#include "menu.h"
#include "edx.h"
#include "status.h"            // need status_note()
#include "help.h"
#include "window.h"           // need screen_swap()
#include "shell.h"
#include <stdlib.h>
#include <stdio.h>       /* need puts() and sprintf */
#include <string.h>

#include "test.h"

#define redbox_locus 20

#define EXEC_SYNC    0     //  wait on child terminate
#define EXEC_ASYNC   1     //  copy process ID of child to results.codeTerm
#define EXEC_TRACE   3     //   like #2 with debug info

USHORT exec_os2(char const* s);

extern ULONG shell_ok_sem;
extern HMTX  hshell_ok_sem;

/****************************** **************************************** */

bool shell (char const* s, bool dosscreen)
{
DBG_SHELL(dout<<"shell line "<<__LINE__<<" settings.shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
DBG_SHELL(dout<<"shell line "<<__LINE__<<" s = °"<<s<<"°"<<endl);
DBG_SHELL(dout<<"shell line "<<__LINE__<<" dosscreen = "<<dosscreen<<endl);
SHORT retval;
  rc = DosRequestMutexSem(hshell_ok_sem, -1L); //wait indefinitely
if (dosscreen) 
   screen_swap();

puts(">> type EXIT to return to Editor<<");
retval= exec_os2 (s);    /* execute a copy of the shell */
rc = DosReleaseMutexSem(hshell_ok_sem);

DBG_SHELL(dout<<"returned from shell retval = "<<retval<<endl);         
DBG_SHELL(dout<<"shell DosReleaseMutexSem  rc = "<<rc<<endl);         

if (dosscreen) {
    screen_swap();
   }
if (retval != 0) {
   SHORT er= errno;
   if (er != 0) {
      static char message[]= "errno= xxxx";
      num2str (message+7, errno, 4, ' ');
      redbox_s (redbox_locus, 202, eh_continue, message);
      return FALSE;
      }
   }
return TRUE;
}

/* ******************************************************************* */
void shell_and_run (void)
{
char buf[128];
const char *source= settings.shell_command.s;
char *dest= buf;
bool OK;

memset(buf, 0, sizeof(buf));
DBG_SHELL(dout<<"shell_and_run settings.shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
                                    /* save file */
if (settings.shell_command.flags & 16) {
   if (!view_assure_all_saved()) 
      return;                       /* escape out of it */
   }
else if (settings.shell_command.flags & 8) {
   if (!view_dont_throw_away (active_view)) return;
   }
while (*source) {
   if (*source == '%') {           /* expand a command */
      source++;
      switch (*source++) 
      {
         case '%':                 /* %% if you want a real % */
            *dest++ = '%';
            break;
         case 'n':  
         {                         /* file name */
            struct sheet *sh= (struct sheet *)Mem::vmem_trans (active_sheet);
            strcpy (dest, sh->filename);
            dest += strlen (dest);
         } break;
      }                            /* default is to ignore the character */
   }
   else *dest++ = *source++;
   }
*dest= '\0';
//status_note (buf);

DBG_SHELL(dout<<"shell_and_run  passing buf = "<<buf<<endl);
//OK= shell (buf, settings.shell_command.flags & 1);
OK= shell (buf, settings.shell_command.flags);

//status_note (NULL);
if (OK && settings.shell_command.flags & 4)
   locate_view ("LOG.TXT", 2);        /* always reload file */
}

/* **************************************** */

void shell_menu (void)
{
DBG_SHELL(dout<<"shell menu before DEF_HELP s = °"<<settings.shell_command.s<<"°"<<endl);
static char *choices[]= {
   "Command",
   "Run!",
   "Screen    xxx",
   "Load log  xxx",
   "File save xxx",
   "All save  xxx" };
static SHORT lastchoice= 0;
DBG_SHELL(dout<<"shell menu before DEF_HELP s = °"<<settings.shell_command.s<<"°"<<endl);
//DEF_AUTOHELP (SHELL_HELP,10);
DEF_HELP (SHELL_HELP,1,1,10);

NEW_HELP;
for (;;) {
   menu_yesno (choices[2], 10, settings.shell_command.flags & 1);
   menu_yesno (choices[3], 10, settings.shell_command.flags & 4);
   menu_yesno (choices[4], 10, settings.shell_command.flags & 8);
   menu_yesno (choices[5], 10, settings.shell_command.flags & 16);
   switch (menu (choices, 6, &lastchoice, MENU_USE_INITIALS|MENU_LOCATION|MENU_ITEMHELP, 13)) {
      case 0:                /* edit command */
         DBG_SHELL(dout<<"shell menu passing to prompt box s = °"<<settings.shell_command.s<<"°"<<endl);
         prompt_box ("Shell command:", settings.shell_command.s, sizeof(settings.shell_command.s), 0, 2, 0);
         break;
      case 1:                /* Run */
         shell_and_run();
         OLD_HELP;
         return;
      case 2:               /* screen */
         settings.shell_command.flags ^= 1;
         break;
      case 3:               /* log */
         settings.shell_command.flags ^= 4;
         break;
      case 4:               /* save this file */
         settings.shell_command.flags ^= 8;
         break;
      case 5:               /* save all files */
         settings.shell_command.flags ^= 16;
         break;
      case -1:             /* done */
         OLD_HELP;
         return;
      }
   }
}

/*******************************************************************/
USHORT exec_os2(char const* s)
{
   char failbuf[128];
   PID pidasync;
   RESULTCODES resultcodes;
   USHORT rc;
   char args[128];
   const char* ResultPointer;  // Environment scan result pointer (returned) 

   DBG_SHELL(dout<<"passed arg =  °"<<s<<"°"<<endl);
#pragma warn -ucp                  // signed/unsigned warn

                                                    // usual: F:\OS2\CMD.EXE
   if((rc=DosScanEnv("COMSPEC", &ResultPointer))!= 0)  {
      DBG_SHELL(dout<<"shell COMSPEC = "<<ResultPointer<<endl);
   }
   memset(args, 0, sizeof(args));            // zero out the bufs
//   ptr = strrchr((char*)ResultPointer, 0x5C);//find last '\' in E:\OS2\CMD.EXE
   DBG_SHELL(dout<<__FILE__<<" Line "<<__LINE__<<"  COMSPEC  = °"<<ResultPointer<<"°"<<endl);
//   strcpy(args, (ptr)+1);     // grab the right str = "CMD.EXE" or "4OS2.EXE"
   sprintf(args, "%s%c%s%s", ResultPointer,'\0'," /k ", s);
   DBG_SHELL(dout<<__FILE__<<" Line "<<__LINE__<<"  args  = °"<<args<<"°"<<endl);
   DBG_SHELL(dout<<__FILE__<<" Line "<<__LINE__<<"  args  = °"<<args<<"°"<<endl);  
//   for(int x = 0; x < 128; x++) {
//     dout<<"shell args["<<x<<"] = °"<<args[x]<<"°"<<endl;
//   }
#pragma warn .ucp
    rc = DosExecPgm(failbuf,
               sizeof(failbuf),
               EXEC_SYNC,
               args,
               0L,
               &resultcodes,
               args);
    if(rc != 0) {
         pidasync = resultcodes.codeTerminate;
         DBG_SHELL(dout<<"return from DosExecPgm =  "<<rc<<endl);         
    }
 return rc;
#pragma warn -aus        // eliminate 'pidasync' assigned but not used
}
#pragma warn +aus
