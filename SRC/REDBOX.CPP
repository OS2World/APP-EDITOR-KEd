/****************************************************
file: REDBOX.C       Copyright 1989 by John M. Dlugosz
   This is the general purpose error handler
*****************************************************/
#define INCL_OS2
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>

#include "usual.h"
#include "getkey.h"
#include "keycodes.h"
#include "misc.h"
#include "redbox.h"
#include "setting.h"
#include <string.h>  /* need strlen() */

#include "basewin.h"
#include "test.h"
                            // OS/2 only 
//static void  os2_show_message (window_t w, SHORT locus, SHORT num, ULONG errnum);
static void  os2_show_message (basewin &w, SHORT locus, SHORT num, ULONG errnum);


#define err_colors settings.colors[REDBOX_COLORS]

/* the data for the error messages is stored this way because it is
   simple to combine all messages into one table, and it is sparse.  */

struct err_string {
   SHORT locus, num;
   char const* line1;
   char const* line2;
   };

struct err_string err_messages[]= {
   { 0,0, "Test message", "Doesn't mean anything" },
                                /* locus 2 -- sheet file system   VIEW_FIL */
   { 2,1, "File could not be loaded."},
   { 2,100, "File has unsaved changes.", "Save file?"},
   { 2,101, "File is too big, maximum is 256K lines ",
            "File Save will overwrite original and truncate" },
   { 2,200, "Could not open output file", "File NOT saved"},
   { 2,201, "Error writing to file", "File NOT saved"},
   { 2,203, "File has no name!" },
   { 2,204, "Could not move file to backup"},
   { 2,205, "Cannot find KED.HLP, please put it in a subdirectory",
            "in your PATH or in HELP PATH"},
                                                 /* locus 3 -- COMMANDO.C */
   { 3,100, "Are you sure?","" },
   { 3,101, "Destination file already exists;","Overwrite it anyway?"},
   { 3,102, "Are you sure you want to modify file attributes?",""},
   { 3,200, "Disk Error!","Unable to complete operation" },
   { 3,201, "Cannot copy file to itself!", "Operation aborted"},
   { 3,202, "No files selected for multi-file operation",""},
   { 3,204, "Error accessing DOS service" },
                                                        /* locus 4 */
   { 4,1, "No block defined."},
   { 4,200, "Heap full!", "Cannot allocate scrap buffer"},
   { 4,201, "No current scrap buffer"},
   { 4,202, "Bad block command:  active sheet == scrap sheet"},
                                                        /* locus 5 */
   { 5,200, "File has no name", "Cannot save"},
                                               /* locus 8 -- search errors */
   { 8,200, "Unable to compile search expression",""},
   { 8,201, "Unable to allocate memory for","search expression"},
   { 8,202, "No search/replace strings specified",""},
                                            /* locus 9 -- keyboard macros */
   { 9,200, "already recording a macro.", "cannot nest recording sessions"},
                                            /* locus 10 -- filelist */
   { 10, 200, "Filename is too long"},
   { 10, 201, "You must set a filelist first"},
                                           /* locus 11 -- settings */
   { 11, 1, "Unable to load settings"},
   { 11, 200, "Unable to save settings"},
                                          /* locus 12 -- critical error */
   { 12, 200, "DOS Critical Error"},
                                     /* locus 13 -- context-sensitive help */
   { 13, 200, "Subject not found in help file"},
   { 13, 201, "Error reading from help file"},
   { 13, 202, "Corrupted help file"},
   { 13, 203, "No help on help availiable"},

   {20,200, "#<line> option given with no file loaded" },
   {20,201, "bad option switch"},
   {20,202, "could not shell"},
   {30,1, "error parsing filename"},
   { -1,-1}  /* special end of list mark */
   };

/* **************************************** */

struct err_string *find_message (SHORT locus, SHORT num)
{
/* this just looks up the message in the array.  You could have it
   generate the message from system dependant information, or use
   a combination of both techniques.   */
struct err_string *p= err_messages;
while (p->locus != -1 && ! (p->locus == locus && p->num == num) ) p++;
return p;

}

/* **************************************** */

//static void  show_message (window_t w, SHORT locus, SHORT num)
static void  show_message (basewin &errw, SHORT locus, SHORT num)
{
char image[8];
struct err_string* e;
char const* errname;
                                    /* start with the error bannar */
if (num < 100) errname= "Notice:";
else if (num < 200) errname= "Warning:";
else errname= "Error:";
//put_string (w, 1,2, errname);
//put_string (w, 1, 11, num2str (image, locus, 7, 0));
//put_string (w, 1, 16, num2str (image, num, 7, 0));
errw.put(1,2, errname);
errw.put(1, 11, num2str (image, locus, 7, 0));
errw.put(1, 16, num2str (image, num, 7, 0));

/* print specific messages */
e= find_message (locus, num);
//if (e->line1) put_string (w, 3,1,e->line1);
//if (e->line2) put_string (w, 4,1,e->line2);
if (e->line1) 
   errw.put(3,1,e->line1);
if (e->line2) 
   errw.put(4,1,e->line2);

}

/* **************************************** */

//static enum err_choices  get_choice (window_t w, enum err_choices choices)
static enum err_choices  get_choice (basewin &errw, enum err_choices choices)
{
static char const* mess[]= {
   "SPACE-continue",
   "ESC-abort",
   "RETURN-retry",
   "ESC-cancel",
   "Y-yes",
   "N-no"
   };
SHORT col= 2, loop= 0;
USHORT mask= 1;
while (loop < 6) {                /* print the valid keys */
   if (mask & choices) {
      errw.put(5,col, mess[loop]);
//      put_string (w, 5, col, mess[loop]);
      col += 1+strlen (mess[loop]);
      }
   loop++;
   mask <<= 1;
   }
/* wait for input */
for (;;) {
   USHORT key= getkey();
   switch (key) {
      case kESC:
         if (choices & eh_abort) return eh_abort;
         if (choices & eh_cancel) return eh_cancel;
         break;
      case GREY_Enter:
      case Enter:
         if (choices & eh_retry) return eh_retry;
         break;
//      case ' ':
      case Space:
         if (choices & eh_continue) return eh_continue;
         break;
      case 0x1559:        // 'Y': 
      case 0x1579:        // 'y':
         if (choices & eh_yes) return eh_yes;
         break;
      case 0x314E:       // 'N': 
      case 0x316E:        // 'n':
         if (choices & eh_no) return eh_no;
         break;
      }
   }
}

/* **************************************** */

enum err_choices redbox_s (SHORT locus, SHORT num, enum err_choices choices, char const* s)
{
SHORT len;
enum err_choices result;
basewin errw (7, 60,(Screen.get_screen_height()-7)/2,(Screen.get_screen_width()-60)/2);
errw.clearwin(settings.colors[REDBOX_COLORS][NORMAL_COLOR], ' ');
errw.border(window_core::Double);   
errw.highlight(5,1,settings.colors[REDBOX_COLORS][HIGHLIGHT_COLOR], 58);
errw.show();

/* the s is a file name, so show the right part of it if it won't all fit */
len= strlen(s);
if (len > 58) 
   s += len-58;
errw.put(2,1,s);
show_message (errw, locus, num);
result= get_choice (errw, choices);
return result;
}

/* **************************************** */

enum err_choices redbox (SHORT locus, SHORT num, enum err_choices choices)
{
return redbox_s (locus, num, choices, "");
}


/************************************************************************/

          /* WARNING OS/2 ONLY     11-20-91      */
          /* string s is passed as the name of the C source module */
/* **************************************** */

enum err_choices os2_redbox_s (SHORT locus, SHORT num, SHORT userr, enum err_choices choices, char const* s)
{
SHORT len;
enum err_choices result;

basewin errw (7, 61,(Screen.get_screen_height()-7)/2,(Screen.get_screen_width()-61)/2);
errw.clearwin(settings.colors[REDBOX_COLORS][NORMAL_COLOR], ' ');
errw.border(window_core::Double);  
errw.highlight(5,1,settings.colors[REDBOX_COLORS][HIGHLIGHT_COLOR], 59);
errw.show();

/* the s is a file name, so show the right part of it if it won't all fit */
len= strlen(s);
if (len > 58) 
   s += len-58;

errw.put(2,2, "File: ");
errw.put(2,10, s);
beep_ok();
os2_show_message (errw, locus, num, userr);
result= get_choice (errw, choices);
DBG_ERR(dout<<" os2_redbox returning "<<result<<endl);
return result;
}



/* **************************************** */

static void  os2_show_message (basewin &errw, SHORT locus, SHORT num, ULONG ulmsgnum)
{
char image[8];
CHAR szmsg[100];
ULONG cbszmsg;
APIRET rc;
CHAR *mymsg[1] = {" "};
ULONG datalen = 100;
                                    /* start with the error bannar */
errw.put(1,2, "ERROR:  Locus");
errw.put(1, 18, num2str (image, locus, 7, 0));
errw.put(1,20, " Class:");
errw.put(1, 28, num2str (image, num, 7, 0));

                             /*  - search system message file OSO001.msg  */
rc = DosGetMessage(mymsg,0L,szmsg, datalen, ulmsgnum, (PSZ)"OSO001.MSG", &cbszmsg);

DBG_ERR(dout<<" os2_show_message rc ="<<rc<<endl);
DBG_ERR(dout<<" os2_show_message msg ="<<szmsg<<endl);

if(rc == 0){
  szmsg[cbszmsg] = '\0';           // terminate the string
  errw.print(3, 2, szmsg);    // in tbasewin.cpp
 }
}
