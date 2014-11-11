/*****************************************************
file: STATUS.C      Copyright 1989 by Dlugosz Software
   functions to manage the display of status info
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "status.h"
#include "misc.h"
#include <string.h>  /* need strlen() */
#include "getkey.h"  /* need by message bar */
#include "setting.h"

#include "basewin.h"
#include "test.h"

//basewin* statwptr;
basewin* messwptr = NULL;

//basewin statw(1,Screen.width, Screen.height-1, 0);   // ok
//basewin statw(1,Screen.width-4, Screen.height-1, 2); // mod 6-3-94
basewin notew(1,Screen.width, Screen.height-2,0);

static SHORT name_field_len= 35;  /* length of filename field */

/* **************************************** */

void status_setup()
{
/* called at the beginning of the program, before the first view is created*/

//status_window= create_window (1,Screen.width, Screen.height-1, 0);
//clear_window (status_window, 0,0, 0,Screen.width()-1, (settings.colors[STATUS_COLORS][NORMAL_COLOR]<<8)|' ');
//  OLD //put_string (status_window, 0,62, "\x12" "       " "\x1d");
//put_string (status_window, 0,62, "R" "       " "C");

                      //checkmem("STATUS.C line 42");
statw.clearwin(YELLOW, _RED,' ');
            //  OLD //put_string (status_window, 0,62, "\x12" "       " "\x1d");
//statw.put(0,62, "R" "       " "C");
statw.put(0,62, "R             C");
                                 //checkmem("STATUS.C line 46");
//statw.show();
                                //checkmem("STATUS.C line 48");
                                //display_on (status_window);
                                //stat_w= status_window;
}

/* **************************************** */

void status_redraw()
{
struct view_rec *v;
struct mark_rec *m;
struct sheet *s;
//v= (struct view_rec *)Mem::vmem_trans (active_view);
//s= (struct sheet *)Mem::vmem_trans (active_sheet);

v = my_active_view;                      // mod 8-27-93
s = my_active_sheet;                     // mod 8-25-93
statw.clearwin(YELLOW, _RED,' ');
statw.put(0,62, "R       C");
status_name();
m= get_mark (v->cursor);
status_block (v->block_active==view_rec::bk_on ? v->block_mode : -1);
DBG_STATUS(dout<<"status redraw row = "<<(m->row)<<endl);
DBG_STATUS(dout<<"status redraw col = "<<(m->col)<<endl);
status_row (m->row);
status_col (m->col);
status_delta (s->changed);
status_insertmode (v->flags&1);
status_indentmode (v->flags&2);
status_numlockmode ();
                                                        /* other indicators */
statw.put(0, 43, "ESC=menus F1=Help");
                                                        /* update color */
statw.highlight(0,0,settings.colors[STATUS_COLORS][NORMAL_COLOR],statw.get_width());
//statw.show();
}

/* **************************************** */


void status_name()
{
SHORT length= name_field_len;  /* length of the filename field */
SHORT col= 6;                  /* column of the filename field */
SHORT len;
char *s;
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct sheet *cur_sheet;

//pad_window (stat_w, 0, col, ' ',length);  /* clear to spaces */
/*
if (v->number != 1) {
   char image[10];
   s= num2str (image, (SHORT)v->number, 2, ' ');
   len= strlen(s);
   s[len++]='\x07';
//   put_string_len (stat_w, 0, col, s, len);
   statw.put(0, col, s, len);
   length -= len;
   col += len;
   }
*/
//cur_sheet= (struct sheet *)Mem::vmem_trans (active_sheet);
cur_sheet = my_active_sheet;        // mod 8-25-93
s= cur_sheet->filename;
len= strlen(s);
if (len > length) 
   s+= (len-length);                /* show end of name only */
statw.put(0, col, s, length);
}

/* **************************************** */

void  status_row (ULONG row)
{
//static ULONG old_row= 0xffffffff;
static ULONG old_row= 1;
char image[12], *s;
//if (old_row == ++row) return;  /* no change in display */
if (old_row == row) 
   return;                       /* no change in display */
                    DBG_STATUS(dout<<"status_row ulong row = "<<row<<endl);
if (row < 0x8000) {
   s= num2str_fast (image, row, 5);
   left_just(s,7);
}
else {
  // s= longnum2str_fast (image, row, 7);
     s= num2str_fast (image, row, 7);
   left_just (s, 7);
}
DBG_STATUS(dout<<"status_row string row = "<<s<<endl);
statw.put(0, 63, s);
old_row= row;
}

/* **************************************** */

void  status_col (SHORT col)
{ 
//static SHORT old_col= -1;
static SHORT old_col = 1;
char image[12], *s;
//if (old_col == ++col) return;
if (old_col == col) 
    return;
DBG_STATUS(dout<<"status_col short col = "<<col<<endl);
s= num2str(image, col, 5, 0);
left_just (s, 5);
DBG_STATUS(dout<<"status_col string col = "<<col<<endl);
statw.put(0, 71, s);
old_col = col;     
}

/* **************************************** */

void  status_delta (bool b)
{
static bool current_state= -1;  /* must be wrong first time */
if (b != current_state) {
   current_state= b;
//   put_string (stat_w, 0, 4, b ? "\xEB" : " ");
   statw.put(0, 4, b ? "\xEB" : " " );
   }
}

/* **************************************** */

void status_block (SHORT mode_switch)
{
static char c[4][2]= { " ", "\xB1", "\xF0", "\xFE" };
/* different symbols for stream, lines, and columns mode */
static SHORT current_state= -1;  /* off */
if (mode_switch != current_state) {
   current_state= mode_switch;
//   put_string (stat_w, 0, 2, c[mode_switch+1]);
   statw.put(0,2,c[mode_switch+1]);
   }
}

/* **************************************** */

void status_key_recording (bool onoff)
{
static bool current_state= -1;
if (onoff != current_state) {
//   put_string (stat_w, 0,0, onoff?"R":" ");
//   statw.put(0,0, onoff?"R":" ");
   statw.put(0,1, onoff?"R":" ");
   }
}

/* **************************************** */

void status_insertmode (bool insert_on)
{
//put_string (stat_w,0,76,insert_on?"Ins":"Ovr");
//put_string (stat_w,0,78,insert_on?"I":"O");
//statw.put(0,78,insert_on?"I":"O");
statw.put(0,77,insert_on?"I":"O");
}

/**********************************************/

void status_numlockmode (void)
{
//put_string (stat_w,0,76,insert_on?"Ins":"Ovr");
//put_string (stat_w,0,77,settings.numlock_on?"N":" ");
//put_string (stat_w,0,77,(KeyState & 0x0020)?"N":" ");
//statw.put(0,77,(KeyState & 0x0020)?"N":" ");
statw.put(0,76,(KeyState & 0x0020)?"N":" ");
}


/* **************************************** */

void status_indentmode (bool indent_on)
{
//put_string (stat_w,0,3,indent_on?"\x14":" ");
statw.put(0,3,indent_on?"\x14":" ");
}

/* **************************************** */
/*   message bar                            */
/* **************************************** */

static bool (* OldFilter) (USHORT);
//static window_t mess_window = 0;

static bool message_clear (USHORT key)
{                                    /* called at next keypress */
//display_off (mess_window);             /* my mission */
messwptr->hide();
key_preprocess= OldFilter;            /* restore old vector */
if (OldFilter) return OldFilter(key);  /* jump to old routine */
else return TRUE;
}

/* **************************************** */
// display a message, and restore the normal screen when a key is pressed. 
void status_message (char const* s)
{
//if (!s && mess_window) {  /* clear message */
if (!s) {                   /* clear message */
if (key_preprocess == message_clear) 
      key_preprocess= OldFilter;
//   display_off (mess_window);
   messwptr->hide();
   return;
 }
//if (!mess_window) 
//   mess_window= create_window (1,Screen.width,Screen.height-1,0);
//clear_window (mess_window, 0,0, 0,Screen.width-1, (settings.colors[STATUS_COLORS][NORMAL_COLOR]<<8)|' ');
//put_string_len (mess_window, 0, 5, s, Screen.width-5);
//display_on (mess_window);
if(!messwptr)
//   messwptr = new basewin(1,messwptr->width(), messwptr->height()-1, 0);
   messwptr = new basewin(1,Screen.width, Screen.height-1, 0);
   messwptr->clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR]<<8);
   messwptr->put(0,5,s);
   messwptr->show();

if (key_preprocess != message_clear)
   OldFilter= key_preprocess;
key_preprocess= message_clear;
}

/* **************************************** */

void status_note (char const* s)
{
/* display a message, and keep it there */

//if (!s) {  /* turn off */
//   if (note_window) {
//      destroy_window (status_window);
//      status_window= note_window;
//      note_window= 0L;
//      }
//   delete notew;
//   return;
//   }
//if (!note_window) {
//   note_window= status_window;  /* remember regular window */
//   status_window= create_window (1,Screen.width,Screen.height-1,0);
//   }
if (s) {
//   basewin notew(1,Screen.width, Screen.height-1,0);

//clear_window (status_window, 0,0, 0,Screen.width-1, (settings.colors[STATUS_COLORS][NORMAL_COLOR]<<8)|' ');
//put_string_len (status_window, 0, 0, s, Screen.width);
//display_on (status_window);
notew.clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR], ' ');
notew.put(0, 0, s);
notew.show();
}
else {
   notew.hide();
 }
}

/* **************************************** */
/*    resume support                        */
/* **************************************** */
#ifdef PACKUP
struct sys_info {
   window_t stat_w, mess_window, note_window, status_window;
   /* if I vary the positioning based on screen width, store any
      new variables in here as well.  */
   };

SHORT status_packup (void  * buf)
{
struct sys_info *p= buf;

p->stat_w= stat_w;
p->mess_window= mess_window;
p->note_window= note_window;
p->status_window= status_window;
return sizeof (struct sys_info);
}

/* **************************************** */

SHORT status_unpack (void  * buf)
{
struct sys_info *p= buf;

stat_w= p->stat_w;
mess_window= p->mess_window;
note_window= p->note_window;
status_window= p->status_window;
return sizeof (struct sys_info);
}

#endif
/* **************************************** */
