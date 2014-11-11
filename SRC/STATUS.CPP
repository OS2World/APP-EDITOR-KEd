/*****************************************************
file: STATUS.C      Copyright 1989 by Dlugosz Software
   functions to manage the display of status info
   10-13-94
   KEd v2.15 the new status line is nominally 76 chars wide.
   The status line is laid out like:
         1         2         3         4         5         6         7
123456789012345678901234567890123456789012345678901234567890123456789012345678
ȹ                                       ESC=menus F1=help R       C     NIǽ
                                                           ^       ^     ^^
                                                           |       |     ||
                                                           ROW     COL   |INS
                                                                         NUM
****************************************************************************/                   
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

static basewin* messwptr = NULL;

//basewin statw(1,Screen.width-4, Screen.height-1, 2); // mod 6-3-94
static basewin notew(1,Screen.get_screen_width()-4, Screen.get_screen_height()-1,2);

static USHORT name_field_len= 35;  /* length of filename field */


/******************************** **************************************** */

void status_setup()
{
                          /* called after the "owner" view is created*/
struct view_rec *v = my_active_view;

statwptr[v->view_num]->clearwin(YELLOW, _RED,' ');
//statwptr[v->view_num]->put(0,61, "R        C");           // TEMP 10-13
statwptr[v->view_num]->put(0, 42, "ESC=menus F1=Help");
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
status_name();
m= get_mark (v->cursor);
status_block (v->block_active==view_rec::bk_on ? v->block_mode : -1);
DBG_STATUS(dout<<"status redraw row = "<<(m->row)<<endl);
DBG_STATUS(dout<<"status redraw col = "<<(m->col)<<endl);
status_quick_redraw(m->row, m->col, s->changed);
//statwptr[v->view_num]->put(0,75,(v->flags&1)?"I":"O");       // insert status
//status_numlockmode ();
                                                        /* other indicators */
//statwptr[v->view_num]->put(0, 43, "ESC=menus F1=Help");
                                                        /* update color */
statwptr[v->view_num]->highlight(0,0,settings.colors[STATUS_COLORS][NORMAL_COLOR],statwptr[v->view_num]->get_width());
//statwptr[v->view_num]->show();
}

/***********************************************************************
         special function for quick status update from view_drw
**********************************************************************/
void status_quick_redraw(ULONG row, SHORT col, bool b)
{
struct view_rec *v;
v = my_active_view;                      // mod 8-27-93

static ULONG old_row= 0;
char row_image[12], *row_s;
static SHORT old_col = 0;
char col_image[12], *col_s;
static bool changed= -1;  /* must be wrong first time */

statwptr[v->view_num]->freeze();      // no output until we're done

statwptr[v->view_num]->put(0,60, "R");
statwptr[v->view_num]->put(0,68, "C");               // was 71 TEMP
statwptr[v->view_num]->put(0, 42, "ESC=menus F1=Help");

if (old_row != row) {                             // row
   if (row < 0x8000) {
      row_s= num2str_fast (row_image, row, 5);
      left_just(row_s,7);
   }
   else {
      row_s= num2str_fast (row_image, row, 7);
      left_just (row_s, 7);
   }
   statwptr[v->view_num]->put(0, 61, row_s);
old_row= row;
}

if (old_col != col) {                                    // col
//   col_s= num2str(col_image, col, 5, 0);
   col_s= num2str_fast(col_image, col, 5);
   left_just (col_s, 5);
   statwptr[v->view_num]->put(0, 69, col_s);      // was 72
   old_col = col;     
}

if (b != changed) {                               // changed state
   changed = b;
   statwptr[v->view_num]->put(0, 4, b ? "\xEB" : " " );
   }
                       // TEMP - must change !!! 10-12-94
statwptr[v->view_num]->put(0,74,(KeyState & 0x0020)?"N":" ");//numlock status
statwptr[v->view_num]->put(0,75,(v->flags&1)?"I":"O");       // insert status
statwptr[v->view_num]->put(0,3,(v->flags&2)?"\x14":" ");  // auto indent status

statwptr[v->view_num]->thaw();              // update window
}

/***********************************************************************/
void status_name()
{
SHORT length= name_field_len;  /* length of the filename field */
SHORT col= 6;                  /* column of the filename field */
SHORT len;
//char *s;
byte *s;          // mod 7-27-94
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct view_rec *v = my_active_view;
struct sheet *cur_sheet;

/*
if (v->number != 1) {
   char image[10];
   s= num2str (image, (SHORT)v->number, 2, ' ');
   len= strlen(s);
   s[len++]='\x07';
//   put_string_len (stat_w, 0, col, s, len);
   statwptr[v->view_num]->put(0, col, s, len);
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
statwptr[v->view_num]->put(0, col, s, length);
}

/* **************************************** */
void  status_row (ULONG row)
{
struct view_rec *v = my_active_view;
static ULONG old_row= 0;
char image[12], *s;
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
statwptr[v->view_num]->put(0, 61, s);
old_row= row;
}

/* **************************************** */
#ifdef FUCKUP
void  status_col (SHORT col)
{
struct view_rec *v = my_active_view; 
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
statwptr[v->view_num]->put(0, 71, s);
old_col = col;     
}
#endif
/* **************************************** */
#ifdef FUCKUP
void  status_delta (bool b)
{
struct view_rec *v = my_active_view;
static bool current_state= -1;  /* must be wrong first time */
if (b != current_state) {
   current_state= b;
//   put_string (stat_w, 0, 4, b ? "\xEB" : " ");
   statwptr[v->view_num]->put(0, 4, b ? "\xEB" : " " );
   }
}
#endif
/* **************************************** */

void status_block (SHORT mode_switch)
{
struct view_rec *v = my_active_view;
static char c[4][2]= { " ", "\xB1", "\xF0", "\xFE" };
/* different symbols for stream, lines, and columns mode */
static SHORT current_state= -1;  /* off */
if (mode_switch != current_state) {
   current_state= mode_switch;
//   put_string (stat_w, 0, 2, c[mode_switch+1]);
   statwptr[v->view_num]->put(0,2,c[mode_switch+1]);
   }
}

/* **************************************** */

void status_key_recording (bool onoff)
{
struct view_rec *v = my_active_view;
static bool current_state= -1;
if (onoff != current_state) {
//   put_string (stat_w, 0,0, onoff?"R":" ");
//   statw.put(0,0, onoff?"R":" ");
   statwptr[v->view_num]->put(0,1, onoff?"R":" ");
   }
}

/* **************************************** */
#ifdef FUCKUP
void status_insertmode (bool insert_on)
{
struct view_rec *v = my_active_view;
//put_string (stat_w,0,76,insert_on?"Ins":"Ovr");
//put_string (stat_w,0,78,insert_on?"I":"O");
//statw.put(0,78,insert_on?"I":"O");
statwptr[v->view_num]->put(0,77,insert_on?"I":"O");
}

/**********************************************/

void status_numlockmode (void)
{
struct view_rec *v = my_active_view;
//put_string (stat_w,0,76,insert_on?"Ins":"Ovr");
//put_string (stat_w,0,77,settings.numlock_on?"N":" ");
//put_string (stat_w,0,77,(KeyState & 0x0020)?"N":" ");
//statw.put(0,77,(KeyState & 0x0020)?"N":" ");
statwptr[v->view_num]->put(0,76,(KeyState & 0x0020)?"N":" ");
}

/* **************************************** */

void status_indentmode (bool indent_on)
{
struct view_rec *v = my_active_view;
//put_string (stat_w,0,3,indent_on?"\x14":" ");
statwptr[v->view_num]->put(0,3,indent_on?"\x14":" ");
}

#endif
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
if (OldFilter) 
    return OldFilter(key);  /* jump to old routine */
else return TRUE;
}

/* **************************************** */
// display a message, and restore the normal screen when a key is pressed. 
void status_message (char const* s)
{
if (!s) {                   /* clear message */
if (key_preprocess == message_clear) 
     key_preprocess= OldFilter;
     messwptr->hide();
   return;
 }
if(!messwptr) {
   messwptr = new basewin(2,Screen.get_screen_width()-4, Screen.get_screen_height()-1, 2);
   messwptr->clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR]);
   messwptr->put(0,5,s);
   messwptr->show();
}

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
//notew.clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR], ' ');
notew.clearwin(attr(BLUE,_LGREY), ' ');
notew.put(0, 3, s);
notew.show();
}
else {
   notew.hide();
 }
}


