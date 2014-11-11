/******************************************************
file: FILELIST.C     Copyright 1989 by Dlugosz Software
   file list buffer function primitives
******************************************************/

#include "usual.h"
#include "vmem.h"
#include "window.h"
#include "sheet.h"
#include "view.h"
#include "redbox.h"
#include "filelist.h"  /* mine */
#include <ctype.h>  /* for isalpha() */

#define locus 10

static char filename_buf[FNAME_LEN];
static vptr old_active_sheet, old_active_view;
static mark_t cursor;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void filelist_set (vptr view)
{
filelist_view= view;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  select (void)
{
struct view_rec *v;
old_active_sheet= active_sheet;  /* remember old sheet and view */
old_active_view= active_view;

active_view= filelist_view;      /* set to filelist view and sheet */
v= (struct view_rec *)Mem::vmem_trans (active_view);
active_sheet= v->sheet;
cursor= v->cursor;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  deselect (void)
{
active_sheet= old_active_sheet;  /* restore old sheet and view */
active_view= old_active_view;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool filelist_exists (bool create)
{
if (!filelist_view) {
   if (create) {
      vptr old_active_view= active_view;
      new_sheet ();
      new_standard_view (3);
      filelist_view= active_view;
      view_make_active (old_active_view);
      return TRUE;
      }
   else {
      redbox (locus, 201, eh_cancel);
      return FALSE;
      }
   }
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  fetch (mark_t cursor)
{
struct line_header *line_hdr= (struct line_header *)Mem::vmem_trans (get_line (get_mark (cursor)->row));
char *s= (char *)(line_hdr+1);
char *p= filename_buf;
SHORT max= line_hdr->length;
SHORT len= 0;
while (len++ < max && *s != ' ') {
   if (len == 80) {
      *s= '\0';
      if (redbox_s (locus, 200, eh_cancel|eh_continue,filename_buf) == eh_continue)
         return TRUE;   /* take it as it is */
      else return FALSE; /* error */
      }
   *p++=*s++;
   }
*p= '\0';
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  next_line (void)
{
view_gohome (); view_gohome ();  /* assure cursor is in first column */
if (!fetch(cursor)) return FALSE;
if (filename_buf[0] == '*') return FALSE;  /* at end of list */
do {
   if ((view_stream_cursor (DOWN) == FALSE) || (view_peek () == '*'))
      return FALSE;
   } while (view_peek () == ' ');
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  prev_line (void)
{
view_gohome (); view_gohome ();
do {
   if ((view_stream_cursor (UP) == FALSE) || (view_peek () == '*'))
      return FALSE;
   } while (!isalpha (view_peek ()));
if (!fetch(cursor)) return FALSE;
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

char *filelist_next (void)
{
bool b;
if (!filelist_exists (FALSE)) return NULL;
select();
b= next_line();
deselect();
return b ? filename_buf : NULL;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

char *filelist_prev (void)
{
bool b;
if (!filelist_exists (FALSE)) return NULL;
select();
b= prev_line();
deselect();
return b ? filename_buf : NULL;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void filelist_top (void)
{
char c= 0;
if (!filelist_exists (FALSE)) return;
select();
while (view_stream_cursor(UP) && (c=(char)view_peek()) != '*') {}
if (c=='*') view_stream_cursor(DOWN);
deselect();
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void filelist_bottom (void)
{
if (!filelist_exists (FALSE)) return;
select();
while (view_stream_cursor(DOWN) && view_peek() != '*') {}
deselect();
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

char *filelist_fetch (void)
{
char *s;

if (!filelist_exists (FALSE)) return NULL;
select ();
s= fetch (cursor) ? s= filename_buf : NULL;
deselect ();
if (*s == '*' || !*s || *s == ' ') return NULL;
return s;
}
