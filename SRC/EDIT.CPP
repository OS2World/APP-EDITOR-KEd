/*****************************************************
File: EDIT.CPP     copyright 1989,90 by Dlugosz Software
   This is an example of the editor toolbox.  This is
   my current test of a working editor.
*****************************************************/
#define INCL_SUB
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#define INCL_DOS
#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "keycodes.h"
#include "sheet.h"
#include "view.h"
#include "status.h"
#include "redbox.h"
#include "dirlist.h"             // need path[] and cwd[] globals
#include "block.h"
#include "setting.h"
#include "getkey.h"
#include "err_hand.h"            // critical error exit function
#include "mylib.h"               /* needed for file close, open */
#include <string.h>
#include <stdlib.h>              /* need atoi(), exit() */
#include <process.h>             /* _beginthread */
#include "help.h"
                                             //#include "no_zoom.h"
#include "window.h"    
#include "mymenu.h"
#include "initkeys.h"

#include "tow.h"

#include "test.h"

static void  process_env_string (char const* name);
static bool load_help(void);
static bool load_ini(void);

//#define THREADSTACK 4000                     /* size of stack each thread*/
#define THREADSTACK (32 * 1024)               /* size of stack each thread*/
#define redbox_locus 20

extern _stdcall void inchar(void *);    // separate thread - watches keyboard 

unsigned  rc;
HMTX hkbdsem;           /* controls main execution - execs only if keyhit */
HMTX hkquesem;          /* controls access to keyboard queue        */
HMTX hshell_ok_sem;
static bool debug_on = TRUE;

#ifdef TESTING             // create DBG stream here, so meta_win ctor has it
ofstream dout("logfile");
ofstream memout("mem_err");
#endif

meta_window Screen;          // create a meta_window
window_core master (Screen); //create window_core obj, pass it a meta_window

/********************************************************************* 
     Main processing loop.
     get a key record from the input queue.
     if a key was input from keyboard (owner == TRUE), then look at
     the key record and try to call a function. If no function, then
     just input it into the sheet.

     if input was from clipboard (owner == FALSE), then don't try to 
     call a function, just put it in the sheet.

     v 3.0  9-12-94
            still needs work. sigh..

************************************************************************/
static void  edit_loop (void)
{
USHORT key;
bool ok = FALSE;        // temp for load_key
USHORT cmd_result = 0x0000;      // returned from do_command
UCHAR lokey, hikey;     // low and high byte of KEY USHORT
USHORT keystate;
struct key_rec edit_key_rec;

#ifndef NO_COMMANDO
//char commando_cwd[FNAME_LEN];
//strcpy (commando_cwd, cwd);
#endif

#ifndef NO_HELP
DEF_HELP(EDIT_HELP,1,-1,-1);
init_help (Help);
NEW_HELP;
#endif

for (;;) {
get_next:
   edit_key_rec = view_getkey();
   key = edit_key_rec.value;
   keystate = edit_key_rec.state;
   lokey = LOUCHAR(key);
   hikey = HIUCHAR(key);

//  DBG_INI(dout<<"edit.cpp line "<<__LINE__<<" shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
   switch (key) {
      case kESC:   
             if (pulldown_menu() && tryquit ()) {
                  return;
             }
             else {
                lokey = 0x00;     // "eat" the ESC key
             }
             break;
      default:
         DBG_KEY(dout<<'\n'<<"edit key =  "<<hex<<key<<"  high = "<<hikey);
         DBG_KEY(dout<<" low = "<< lokey<<" keystate = "<<keystate<<dec<<endl);
         if(edit_key_rec.owner == TRUE) {                // keyboard key
            if((CTL_KEY(keystate)) || ((lokey == 0x00)||(lokey == 0xE0))) {
               if(key != 0x0000) {
                  cmd_result = do_command(mykey_rec);
                  DBG_KEY(dout<<" edit_key after do_command() "<<endl);        
               }
               if(cmd_result == CMD_QUIT) {
                  DBG_KEY(dout<<" edit key QUIT cmd_result = "<<hex<<cmd_result<<dec<<endl);
                  return;
               }
               if(cmd_result == CMD_HANDLED) {
                  DBG_KEY(dout<<" edit key ignoring cmd_result = "<<hex<<cmd_result<<dec<<endl);
                  goto get_next;
               }
            }
            if((lokey != 0xE0) && (lokey >0)) { // ignore any ext key not handled
               DBG_KEY(dout<<" edit_key calling view_addkey() "<<endl);        
               view_addkey(lokey);
               break;
           }
         }
         else {
           DBG_KEY(dout<<" edit key not owner"<<endl);
           if((lokey < 255) && (lokey >0)) { // ignore any ext key not handled
//            DBG_KEY(dout<<" edit_loop addkey  key = "<<hex<<lokey<<dec<<endl);
              view_addkey(lokey);
              break;
           }
         }
   }                               // end switch/case
}                                  // end for(;;)
#pragma warn -aus        // eliminate 'OLD_HELP'assigned but not used
}
#pragma warn +aus

/* ******************************************************************
     initialization stuff                 
***************************** **************************************** */

static bool settings_loaded= FALSE;
static bool help_loaded=FALSE;

/************************************************************************/
static void  assure_windows_ready(void)
{
  bool ok = FALSE;
  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<< checkmem2<<endl);
  DBG_INI(dout<<"file "<<__FILE__<<" line "<<__LINE__<<" settings.numlock_on = "<<settings.numlock_on<<endl);
  settings_loaded = load_ini();
  DBG_INI(dout<<"file "<<__FILE__<<" line "<<__LINE__<<" settings.numlock_on = "<<settings.numlock_on<<endl);
//  if(settings_loaded == FALSE)
    DBG_INI(dout<<"edit.cpp line "<<__LINE__<<" shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
                   //  checkmem("EDIT.C line 150");
  help_loaded = load_help();
                  //    checkmem("EDIT.C line 152");

  if (!Screen.iscolor()) {           /* color card and color monitor  */
      setting_defaultcolors(0);
  }
  DBG_INI(dout<<"file "<<__FILE__<<" line "<<__LINE__<<" settings.numlock_on = "<<settings.numlock_on<<endl);
  ok = keybd_setup();
  if(!ok)
     err_exit("Failed to setup Keyboard", 23);
//  status_setup();
  my_new_scrap();
  DBG_INI(dout<<"edit.cpp line "<<__LINE__<<" shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2 <<endl);
}

/*************************************************************
     Do I really need 2 different functions to load the 
     INI and HLP files? They both should reside in the
     current directory or path. But only HLP should
     be in the system HELP directories.
***************************************************************/
bool load_ini(void)
{
   bool ok = FALSE;
   USHORT return_code;
   char ini_path[FNAME_LEN];
   char temp_path[FNAME_LEN];
   memset(temp_path, '\0', FNAME_LEN);
   memset(ini_path, '\0', FNAME_LEN);

   if(!settings_loaded){
      ok = setting_load ("ked.ini");       /* try to load from current dir */
      DBG_INI(dout<< " Settings loaded cur dir ="<<ok<<endl);
      if(!ok) {                     /* KED.INI must be on PATH */
        return_code = DosSearchPath(2,(PSZ)"PATH",(PSZ)"KED.INI",ini_path, sizeof(ini_path));
        if (return_code == 0) {
            ok = setting_load((char *)ini_path);
            DBG_INI(dout<< " Settings loaded path ="<<ok<<endl);
        }
        else {
            strcpy(temp_path, exe_dir);
//            DBG_INI(dout<< "settings from exe dir = "<< temp_path<<endl);
            strcpy(ini_path, find_basesubdir(temp_path));
            strcat(ini_path, "KED.INI");
            ok = setting_load(ini_path);
            DBG_INI(dout<<"loaded settings from exe dir = "<<ok<<endl);
        }
      }
   }
return ok;
}

/*********************************************************************/
bool load_help(void) 
{
#pragma warn -ucp                  // signed/unsigned warn
   bool ok = FALSE;
   unsigned char help_path[FNAME_LEN];
   ULONG     PathSource  = SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY;

   USHORT return_code;
   mylib_file_t help_f;

   memset(help_path, '\0', FNAME_LEN);

  if(!help_loaded){                         
//    ok = FALSE;                       // try to load HELP from current dir
    ok = mylib_open("ked.hlp", &help_f, MYLIB_READONLY);
    if(!ok) {
       delta_filename (help_path, exe_dir, "*.HLP");  // load from exe_dir?
       ok = mylib_open(help_path, &help_f, MYLIB_READONLY); 
    }
    if(ok) {
       strcpy (settings.help_path, cwd);
       strcat (settings.help_path, "KED.HLP");
       mylib_close(help_f);
    }
    else {                       /* KED.HLP might  be on PATH */
        return_code = DosSearchPath(PathSource,
                                    "PATH",
                                    "KED.HLP",
                                    help_path, sizeof(help_path));

        if (return_code != 0)  /* or KED.HLP might be in OS/2 HELP dirs */
        return_code = DosSearchPath(SEARCH_ENVIRONMENT,
                                    (PSZ)"HELP",
                                    (PSZ)"KED.HLP",
                                    help_path, sizeof(help_path));        
        if (return_code == 0) {
           strcpy (settings.help_path, help_path);
      //     help_loaded = TRUE;
         }
    }
#pragma warn .ucp
    }
return ok;
}

/* **************************************** */

static void process_option (char *s)
{
switch (*s) {
//   case 's':                                  /* Swap option */
//      switch (s[1]) {
//         default:  resume_mode= 1; break;     /* fast */
//         }
//      break;
//   case 'r':                                  /* display rows */
//      Vid::get_height()= atoi(s+1);
//      break;
//   case 'c':                                  /* display columns */
//      Vid::get_width()= atoi(s+1);
//      break;
//   case 'M':                                  /* video mode */
//      Video::vidparms.w_mode= atoi(s+1);
//      break;
//#ifdef KED_BETA
//   case 'd':
//   case 'D':
//        debug_on = TRUE;
//        break;
//#endif
   case 'g':               /* load settings file if not already loaded */
      if (settings_loaded) break;
                                              /* else falls through */
   case 'G':                                  /* load a settings file */
      if (setting_load (s+1))
         settings_loaded= TRUE;
      break;
   case 'P':                                  /* program name */
      strcpy (progname, s+1);
      break;
   case 'e':                                  /* default file extension */
      strncpy (settings.default_ext, s+1, 4);
      break;
   default:
      redbox_s (20,201,eh_continue,s-1);
   }
}

/* **************************************** */

static void  process_arg (char *s)
{
switch (s[0]) {
   case '-':                                      /* normal option switch */
      process_option (s+1);           break;
//   case '@':                                      /* resume file name */
//      strcpy (resume_file, s+1);  break;
   case '&':                             /* more switches from environment */
      process_env_string (s+1);   break;
   case '#':                                      /* jump to line number */
      if (active_view) view_gotoline (atoi(s+1)-1);
      else {
         redbox (redbox_locus, 200, eh_continue);
         }
      break;
   default:                                /* assume it is a file to load */
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      locate_view (s, 1);  /* load if not found */
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      break;
   }
}

/* **************************************** */

static void  process_env_string (char const* name)
{
char *s= getenv(name);
if (s == NULL) return;
for (;;) {
   char *p, save;
   while (*s==' ') s++;                           /* skip whitespace */
   if (*s == '\0') return;                       /* all done */
   p= s+1;
   while (*p != ' ' && *p != '\0') p++;          /* find end of parameter */
   save= *p;
   *p= '\0';                                    /* mark as end of string */
   process_arg (s);                             /* feed it in */
   *p= save;
   s= p;
   }
}

/* **************************************** */

static void  assure_1_view(void)
{
if (!active_view) {
   new_sheet();
   new_standard_view (0);
//   status_setup();
   }
} 

/********************************************************************/
static bool thread_init(void)
{
              /* I have to create SEMs here, but only used in getkey  */
   rc = DosCreateMutexSem(NULL, &hkbdsem, 1L, FALSE);
   if (rc != 0)  return FALSE;
   rc = DosCreateMutexSem(NULL, &hkquesem, 1L, FALSE);
   if (rc != 0)  return FALSE;
   rc = DosCreateMutexSem(NULL, &hshell_ok_sem, 1L, FALSE);
   if (rc != 0)  return FALSE;

                                 /* start keyboard thread    */
  if(_beginthread(inchar,THREADSTACK,NULL) == -1) {
     err_exit(" Failed to start critical threads", 0);
  }
  while(!inchar_ready) {      // 1st time, wait for inchar to start
    DosSleep(50);
  }

return TRUE;
}

/*************************************************************************/
void _cdecl main (SHORT argc, char *argv[])
{
//  bool ok = FALSE;
//  char test[5];       // test for trap 05 errors
ULONG ulTimes;          // need for Trap Ctrl-Break Signal

#ifdef TESTING
  if(!dout) {
    DosBeep(500,300);
  }
  if(!memout) {
    DosBeep(1000,300);
  }
#endif

#ifdef ERR_HAND
rc = set_int24();
  if(rc != 0) {
     err_exit("Error initializing error handler", 3);
  }

MYEXCEPTIONREGISTRATIONRECORD myExceptionRegRecord;
myExceptionRegRecord.prev_structure = NULL;
myExceptionRegRecord.ExceptionHandler = MyExceptionHandler;
DosSetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD) &myExceptionRegRecord);
DosSetSignalExceptionFocus(SIG_SETFOCUS, &ulTimes);

if(setjmp(myExceptionRegRecord.env))
   goto OnException;
#endif

  DBG_INI(dout<<"file "<<__FILE__<<" line "<<__LINE__<<" settings.numlock_on = "<<settings.numlock_on<<endl);

filesys_init();
get_current_directory(cwd);               // init cur dir
Mem::vmem_setup();
strcpy(exe_dir, *argv);    // assume loaded via X:\subdir\KED.EXE

DBG_FILE(dout << "EXE_DIR =  " << exe_dir <<endl);
DBG_MEM(memout<< " main after except setup "<< checkmem2 << endl);

assure_windows_ready();

if(!thread_init()) {
   err_exit("Error creating initial threads", 5);
}
process_env_string ("EDITOR");  /* do parameters from environment string */
strcpy (progname,*argv);
argc--;                         /* don't count program name */
argv++;
while (argc--) {                /* process each parameter */
   process_arg (*argv);
   argv++;
   }
if (!*settings.help_path) {     /* help is usually in same dir as program */
    delta_filename (settings.help_path, progname, "*.HLP");
   }

//#ifdef TESTING
//if(!debug_on) {
//   dout.close();
//   memout.close();
//}
//#endif


  assure_1_view();
  DBG_INI(dout<<"edit.cpp line "<<__LINE__<<" shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
  DBG_INI(dout<<"file "<<__FILE__<<" line "<<__LINE__<<" settings.numlock_on = "<<settings.numlock_on<<endl);
//  ok = load_keys();        //  Version 2.08 
//  if(!ok)
//    err_exit("Failed to load Keyboard driver", 10);

  edit_loop();
//  test[5000] = 'A';                    // test trap array bounds

  goto Ked_Exit;

OnException:
#ifdef ERR_HAND
DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD) &myExceptionRegRecord);
//  Screen.term_mess = "application trapped";
  err_exit(" application trapped", 20);
#endif

Ked_Exit:
#ifdef ERR_HAND
if(ORG_NUMLOCK) {
   DBG_NUML(dout<<" exit - setting NUMLOCK    on"<<endl);
   numlock_set(TRUE);
}
else {
   DBG_NUML(dout<<" exit - setting NUMLOCK    off"<<endl);
   numlock_set(FALSE);   
}
DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD) &myExceptionRegRecord);
#endif

DBG_MEM(memout<< " main after except setup "<< checkmem2 << endl);
DBG_BUGS(dout<<'\n'<<"    Normal Closedown in LOGFILE "<<endl);
}



