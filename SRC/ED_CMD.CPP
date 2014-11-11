/*
      ED_CMD.C      holds all function calls for v2.08 KeyTable
      
      v 1.0         March 26, 1994
*/
#define INCL_SUB
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#define INCL_DOS

#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "window.h"
#include "keycodes.h"
#include "sheet.h"
#include "view.h"
#include "menu.h"
#include "status.h"
#include "redbox.h"
#include "dirlist.h"
#include "block.h"
#include "setting.h"
#include "edx.h"
#include "keymac.h"
#include "misc.h"
#include "ascii.h"
#include "edset.h"
#include "getkey.h"
#include "version.h"
#include "filelist.h"
#include "askfile.h"
#include "err_hand.h" // critical error exit function
#include "search.h"  /* need packup & unpack functions */
#include "mylib.h" /* needed for file close */
#ifdef TIME_STAMP
#include <time.h>   /* for timestamp function */
#endif
#include "help.h"
#include "shell.h"     /* for shell() function */
#include "mymenu.h"
#include "tool.h"
#include "initkeys.h"
#include "ed_cmd.h"     // function prototypes

#include "test.h"

static SHORT retval = 0;

#pragma warn -par         // many funcs don't use  ULONG param 
#pragma warn -rvl         // and don't return a value

#ifndef NO_ASCIIBOX
//SHORT  show_ascii_chart (void)
SHORT ascii_chart (void)
{
char str[4], *s= str;
struct view_rec *v;
DEF_HELP (ASCII_HELP,1,25,10);

NEW_HELP;
switch (ascii_chart (view_peek (), str)) {
   case 4:                                         /* character */
      v= (struct view_rec *)Mem::vmem_trans(active_view);
      sheet_addchar (v->cursor,*s);
                                       /* low-level sheet function is used
                                          so a CR or other special char will
                                          not be interpreted as special,
                                          which view_addkey() does. */
      break;
   case -1:
      break;
   default:
      view_addstring (s);
      break;
   }
OLD_HELP;
}
#endif
/* **************************************** */
#ifdef TIME_STAMP
SHORT  timestamp(void)
{
SHORT loop;
time_t time_of_day= time(NULL);
char* image= ctime (&time_of_day);
for (loop= 0;  loop < 24;  loop++)
   view_addkey (image[loop]);
}
#endif

/* **************************************** */

SHORT  keycode(void)
{
USHORT key;
char image[6], *s;

key= getkey();
s= num2str(image, key, 5, '\0');
while (*s)
    view_addkey (*s++);
}

/* **************************************** */
SHORT  prompt_go_line (void)
{
static long line= 1;
struct sheet *sh= (struct sheet *)Mem::vmem_trans (active_sheet);
ULONG max_row= sh->line_count;

if (prompt_long ("Goto line:",&line,1,(long)max_row, 0)) {
   view_gotoline (line-1);
   view_gohome();
   }
}

/* **************************************** */

SHORT  quick_paste(void)
{                                   /* copy the marked block without */
if (view_block_active()) {          /*   disturbing current scrap */
   my_new_scrap();
   block_copy (FALSE);
   block_paste(FALSE);
   restore_scrap();
   }
else block_paste(FALSE);
}

SHORT help(void)
{ 
 bool ok;
 ok = do_help(Help);  // pass Help key = F1
 return ok;
}

SHORT file_quit(void) { closewindow(); }
SHORT file_save(void)  { view_resave_file(); }
SHORT file_next(void)  { view_switch_to_next(); }
SHORT file_prev(void)  { view_switch_to_prev(); }
SHORT file_load(void)  { filemenu(); }
SHORT file_save_all(void) { view_save_all(); }

SHORT toggle_insert(void) { view_insertmode(2); }


SHORT menu_print(void) { printmenu(); }
SHORT menu_file(void) { filemenu(); }
SHORT menu_block(void) { blockmenu(); }
SHORT menu_search(void) { searchmenu(); }
#ifndef NO_SETTING
SHORT menu_settings(void) { edset(); }
#else
SHORT menu_settings(void) {  }
#endif

SHORT menu_shell(void) 
{
SHORT retcode = 0;
DBG_INI(dout<<"ed_cmd.cpp line "<<__LINE__<<" shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
shell_menu(); 
return retcode;
}

SHORT block_start(void)  { mark_block(0); }
SHORT block_end(void)  { mark_block(1); }
SHORT block_copy(void) { block_copy(FALSE); }
SHORT block_cut(void)  { block_copy(TRUE); }
SHORT block_paste(void) { block_paste(FALSE); }
SHORT block_quote(void) { block_paste(TRUE); }
SHORT block_delete(void) { block_zap(); }


#ifdef WORD_WRAP
SHORT reformat(void) {  reform_doc_down(1); }
#else
SHORT reformat(void) {  }
#endif

//SHORT window_move(void) { 
//SHORT window_move(void) { 

SHORT show_version(void) { ked_version(); }
SHORT show_files(void) { select_view(); }
SHORT show_os_screen(void) 
{
   screen_swap();
   getkey();  /* wait for a keypress */
   screen_swap();
//   window_redraw_all();  // screen_swap needs this
}

SHORT go_up(void) {  view_move_cursor(UP); return retval; }
SHORT go_down(void) {  view_move_cursor(DOWN); return retval;}
SHORT go_right(void) {  view_move_cursor(RIGHT); return retval; }
SHORT go_left(void) {  view_move_cursor(LEFT);  return retval;}
SHORT go_next_word(void) { view_move_nextword (RIGHT); }
SHORT go_prev_word(void) { view_move_nextword (LEFT); }

SHORT go_bof(void){ view_gotoline (0); }
SHORT go_eof(void){  view_gotoline (0xffffffff); }    
SHORT go_bol(void) { view_gohome(); }
SHORT go_eol(void) { view_goend(); }
SHORT go_pgup(void) { view_pageup(); }
SHORT go_pgdown(void) { view_pagedown(); }
SHORT go_block_start(void) { block_jump(0); }
SHORT go_block_end(void) { block_jump(1); }

SHORT del_char(void) { view_delchar(); }
SHORT del_line(void) { view_zapline(); }
SHORT del_eol(void) { view_delete_to_eol(); }
SHORT del_bol(void) { view_delete_to_bol(); }
SHORT del_word_right(void) { view_delete_word(RIGHT); }
SHORT del_word_left(void)  { view_delete_word(LEFT); }

SHORT search_start(void) { view_ask_search (); }
SHORT search_next(void) { view_search_next(); }
SHORT search_prev(void) { view_search_prev(); }

SHORT macro_begin(void) { key_record(kAltF7); }
SHORT macro_end(void) 
{
 key_rec tempkeyrec;
 tempkeyrec.value = kAltF7;
 tempkeyrec.state = 0x0000;
 put_key_in_macro_buf(tempkeyrec); 
}
SHORT macro_play(void) { play_key_macro(); }


SHORT ked_shell(void) { shell ("", TRUE); }
SHORT ked_shell_run(void) { shell_and_run(); }
SHORT ked_exit(void) { if(tryquit()) 
                          return TRUE;         // all files saved, go quit
                       else 
                          return FALSE; }      // file not saved, no quit

SHORT ked_exit2(void) { 
  if(really_exit) { 
    if(tryquit()) 
       return TRUE;             // all files saved, go quit
    else 
       return FALSE;            // file not saved, no quit
  }
  return FALSE;
}

/***********************************************************************/

//SHORT wordstar_block(void)   // lead in key is Ctrl-K
//{
// USHORT key = NULL;
// mykey_rec = view_getkey();
// key = mykey_rec.value;

//   switch (key) {
//           case 1071:               // q


//   }
//}


#pragma warn +par      // turn warning back on
#pragma warn +rvl
