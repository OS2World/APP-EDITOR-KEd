/* MYMENU.C     
          holds code that used to be in EDIT.C for Kudzu menus.
          this can be linked to ED2.c if desired..

*/

//#define NO_HELP

#include "usual.h"
#include "window.h"
#include "vmem.h"
#include "menu.h"
#include "mymenu.h"
#include "help.h"
#include "sheet.h"
#include "view.h"
#include "block.h"
#include "mylib.h"
#include "version.h"
#include "setting.h"
#include "askfile.h"
#include "edx.h"
#include "dirlist.h"
#include "filelist.h"

/*********************************************************************/
SHORT blockmenu()       
{
static char *choices[]= {"Copy","Cut","Paste","Delete","Quote","Adjust",
                          "Mode","View", "Read", "Write"};
static char *modechoices[]= {"Stream","Lines","Columns"};
static char fname[FNAME_LEN], scrap_file[FNAME_LEN]= "*.TXT";
static SHORT lastchoice= 0, mode= 0, distance= 0;
SHORT retval = MENU_DISMISS;      // mod 6-18-95
#ifndef NO_HELP
DEF_HELP (BLOCK_HELP,2,20,10);
NEW_HELP;
#endif

switch (menu(choices,10,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|EDIT_LOCATION,6)) {
   case 0:                               /* copy to scrap */
      block_copy (FALSE);
      break;
   case 1:                               /* cut to scrap */
      block_copy (TRUE);
      break;
   case 2:                               /* paste */
      block_paste(FALSE);
      break;
   case 3:                               /* just delete block! */
      block_zap ();
      break;
   case 4:                               /* Quote paste */
      block_paste(TRUE);
      break;
   case 5:                               /* adjust */
      if (view_block_active () && prompt_int
         ("Adjust:",&distance,-256,256,100)) block_adjust (distance);
      retval = 0;
      break;
   case 6: /* mode */
      {
#ifndef NO_HELP
      DEF_HELP (BLOCK_HELP,7,20,10);
      NEW_HELP;
#endif
      mode= menu(modechoices,3,&mode,MENU_USE_INITIALS|MENU_LOCATION,7);
      if (mode != -1) 
         block_changemode (mode);
      retval = 0;
#ifndef NO_HELP
      OLD_HELP;
#endif
      }
      break;
   case 7:                               /* view */
      block_view ();
      break;
   case 8:                               /* Read block */
      if (get_load_name (scrap_file, fname, cwd, "TXT"))
         block_read (fname);
      break;
   case 9:                               /* Write block */
      if (get_save_name (fname, cwd, "TXT"))
         block_write (fname);
      break;
   default:
      retval = 0;
      break;
   }
#ifndef NO_HELP
OLD_HELP;
#endif
return retval;
}

/*************************************************************************
        mod 6-18-95 implemented MENU_DISMISS
                    set all menus to return SHORT to support this
                   
*************************************************************************/
SHORT filemenu()
//bool filemenu(void)
{
//bool quit= FALSE;
SHORT retval = MENU_DISMISS;      // mod 6-18-95
bool loaded = FALSE;
static char *choices[]= { "Load ","Open","Save","Save As",
                          "New window","Duplicate","Close","Quit" };

static SHORT lastchoice= 0;
#ifndef NO_HELP
DEF_HELP (FILE_HELP,2, 20,10);

NEW_HELP;
#endif
switch (menu(choices,8,&lastchoice,               // works OK
        MENU_USE_INITIALS|MENU_ITEMHELP|FILE_LOCATION,10)) {

   case 0:                          // load file into current sheet
      view_load_file (NULL, cwd, settings.default_ext);
      break;
   case 1:                          /* Open */
      new_sheet();
//      new_standard_view (1);
      new_standard_view (0);
      loaded = view_load_file (NULL, cwd, settings.default_ext);
      if(!loaded) 
        closewindow();
      break;
   case 2:                          // save
      view_resave_file ();
      break;
   case 3:                          // save as
      view_save_file (cwd, settings.default_ext);
      break;
   case 4:                                        /* new window */
      new_sheet();
      new_standard_view (1);
      break;
   case 5:                                        /* duplicate */
      {
      struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
      struct mark_rec *m= get_mark (v->cursor);
      ULONG row= m->row;
//      ULONG row = v->mycursor.row;

      new_standard_view (2);
      view_gotoline (row);
      break;
      }
   case 6:                                        /* close */
      closewindow();
      break;
   case 7:
    //  quit= TRUE;
      retval = MENU_APP_EXIT;
      break;
   default:
      retval = 0;
      break;
   }                                              /* switch */
#ifndef NO_HELP
OLD_HELP;
#endif
// return quit;
//return -1;
return retval;
}

/* ******************************************************************** */

SHORT printmenu()
{
static char *choices[]= { "Print file","Block print","Scrap print",
                           "Linefeed","Formfeed" };
static SHORT lastchoice= 0;
mylib_file_t file;
SHORT retval = MENU_DISMISS;      // mod 6-18-95
#ifndef NO_HELP
DEF_HELP (PRINT_HELP,10,10,10);

NEW_HELP;
#endif
loop:
switch (menu(choices,5,&lastchoice,
        MENU_USE_INITIALS|MENU_ITEMHELP|PRINT_LOCATION,11)) {
   case 0:  /* print file */
      sheet_print (active_sheet);
      break;
   case 1:  /* print block */
      block_print ();
      break;
   case 2:  /* print scrap */
      scrap_print ();
      break;
   case 3: /* issue linefeed */
      if (mylib_open (settings.printer,&file,MYLIB_WRITEONLY)) {
         mylib_write_text (file,(UCHAR *)"\x0a",1);
         mylib_close (file);
         }
//      goto loop;
      retval = 0;
      break;
   case 4: /* issue formfeed */
      if (mylib_open (settings.printer,&file,MYLIB_WRITEONLY)) {
         mylib_write_text (file,(UCHAR *)"\x0c",1);
         mylib_close (file);
         }
//      goto loop;
      retval = 0;
      break;
   default:
      retval = 0;
      break;
   }
#ifndef NO_HELP
OLD_HELP;
#endif
return retval;
}

/* **************************************** */
SHORT aboutmenu()
{
static char *choices[]= {"About the Editor"};
static SHORT lastchoice;

switch (menu(choices,1,&lastchoice,MENU_USE_INITIALS|HELP_LOCATION,16)) {
  case 0:
         ked_version();
         break;

  default:
         break;
 }
return 0;
}

/* **************************************** */
#ifdef USE_FILELIST
void  filelist_menu (void)
{
static char *choices[]= {"Set","Clear","Next","Prev","Top","Bottom","Import"};
static SHORT lastchoice;
char *s;
#ifndef NO_HELP
DEF_HELP (FILELIST_HELP,10,10,10);

NEW_HELP;
#endif
switch (menu(choices,7,&lastchoice,MENU_USE_INITIALS|MENU_LOCATION,6)) {
   case 0:                                     /* set */
      filelist_set (active_view); break;
   case 1:                                     /* clear */
      filelist_set (0L); break;
   case 2:                                     /* next */
      if (NULL != (s= filelist_next ()))
         view_load_file (s,NULL,NULL);
      break;
   case 3:                                     /* prev */
      if (NULL != (s= filelist_prev ()))
         view_load_file (s,NULL,NULL);
      break;
   case 4:                                     /* top */
      filelist_top (); break;
   case 5:                                     /* bottom */
      filelist_bottom (); break;
   case 6:                                     /* import */
      if (NULL != (s= filelist_fetch ()))
         view_load_file (s,NULL,NULL); break;
   }
#ifndef NO_HELP
OLD_HELP;
#endif
}
#endif
/* **************************************** */
bool tryquit(void)          
{                  /* returns TRUE to quit, FALSE to not quit. */
           /* make sure all files are saved.  dialog box choice if not  */
return view_assure_all_saved();
}

#ifdef FUCKUP
bool tryquit2(void) // dontcare = TRUE means we dont care      
{                  /* returns TRUE to quit, FALSE to not quit. */
static bool idontcare = TRUE;
if(dontcare == FALSE)
   idontcare = dontcare;  
           /* make sure all files are saved.  dialog box choice if not  */
return view_assure_all_saved2(idontcare);
}
#endif

void closewindow (void)
{
if (!view_dont_throw_away (active_view)) 
     return;                                /* changed his mind */

delete_view();
if (!active_view) {
   new_sheet();                
   new_standard_view (0);
   }
}

SHORT pulldown_menu(void)
{                /*  implements pulldown menus using DLUGOSZ API     */
            
static char *choices[]= {"File  ","Block  ","Search  ","Print  ","About  ","Options"};
static SHORT lastchoice;
SHORT ret_val;

switch (pull_menu(choices, 6, &lastchoice,
//switch (menu(choices, 6, &lastchoice,
         MENU_USE_INITIALS|MENU_LOCATION|MENU_MULTI_UP|MENU_PULLDOWN,8)) {
   case  -1:           // escape key pressed
           ret_val = TRUE;
           break;
   default:
           ret_val =  FALSE;
            break;
 }
return ret_val;
}
