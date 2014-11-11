/*****************************************************
file: VIEW_EXT.C    Copyright 1989 by Dlugosz Software
   extended view commands
*****************************************************/

#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "dirlist.h"
#include "setting.h"
#include <string.h> /* for strcpy() function */
#include <ctype.h>  /* for isalnum() macro and toupper, tolower */

#include "test.h"

#define ALTERNATE_WORDLEFT

#ifdef VIEW_EXTENDED_COM          // disable adv wp features 1-7-93

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  is_word_part (SHORT c)
{
return isalnum (c) || c == '_';
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  is_whitespace (SHORT c)
{
return c == ' ' || c == '\t';
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_move_nextword (enum udlr direction)
{
SHORT ch;
bool in_word;

if (direction == RIGHT) {
   in_word= is_word_part (view_peek ());
   if (view_peek () == '\n') { /* if at end of line, jump to start of next */
      view_stream_cursor (RIGHT);
      return;
      }
   while (view_stream_cursor (RIGHT)) {
      ch= view_peek ();
      if (ch == '\n') return; /* stop at end of line */
      if (!in_word && is_word_part (ch)) return;
      in_word= is_word_part (ch);
      }
   }
else { /* move left */
   struct view_rec *v;

#ifdef ALTERNATE_WORDLEFT
   in_word= FALSE;
   v= (struct view_rec *)Mem::vmem_trans (active_view);
   if (get_mark (v->cursor)->col == 0) { /* skip 1st char if cursor at col 0 */
      view_stream_cursor (LEFT);       /* so begin-of-line will not be caught */
      return;
      }
   while ((get_mark (v->cursor)->col > 0) && view_stream_cursor (LEFT)) {
      ch= view_peek ();
      if (in_word && !is_word_part (ch)) {
         view_stream_cursor (RIGHT); /* overshot; back up */
         return;
         }
      in_word= is_word_part (ch);
      }
   }
#elif
   in_word= TRUE;
   v= (struct view_rec *)Mem::vmem_trans (active_view);
   if (get_mark (v->cursor)->col == 0) { /* skip 1st char if cursor at col 0 */
      view_stream_cursor (LEFT);       /* so begin-of-line will not be caught */
      return;
      }
   while (view_stream_cursor (LEFT) && (get_mark (v->cursor)->col > 0)) {
      ch= view_peek ();
      if (!in_word && is_word_part (ch)) {
         view_stream_cursor (Right); /* overshot; back up */
         return;
         }
      in_word= is_word_part (ch);
      }
   }
#endif
}

#ifdef FUCKUP
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  match_char (mark_t m, enum udlr direction, SHORT find, SHORT stack_char)
{
char c;
SHORT level= 0;
//struct sheet *sh= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *sh = my_active_sheet;              // mod 8-25-93
struct mark_rec *mark= get_mark (m);

while (stream_mark_fast (sh, mark, direction)) {
//while (stream_mark (mark, direction)) {
      mark= get_mark (m);          // maybe need this line ?
//      }
   c= (char)sheet_peek_fast (sh, mark);
   if (c == (char)stack_char) level++;
   else if (c == (char)find && level-- == 0) return TRUE;
   }
return FALSE;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*         NOT USED NOW 8-26-93   
void view_match (void)
{
static char open[]= { '(','{','[','<', 0 },
            close[]={ ')','}',']','>', 0 };
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
mark_t temp_mark, cursor= v->cursor;
SHORT loop, c;

temp_mark= new_mark ();
mark_assign (temp_mark, cursor);
c= sheet_peek (active_sheet, temp_mark);
for (loop= 0; open[loop]; loop++)
   if (c == open[loop]) {
      if (match_char (temp_mark, Right, close[loop], c))
         mark_assign (cursor, temp_mark);
      break;
      }
   else if (c == close[loop]) {
      if (match_char (temp_mark, LEFT, open[loop], c))
         mark_assign (cursor, temp_mark);
      break;
      }
cancel_mark (temp_mark);
}
*/
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_delete_word (enum udlr direction)
{
SHORT ch;

if (direction == RIGHT) { /* delete word right */
   if ((ch= view_peek ()) == '\n') {
      if (view_stream_cursor (RIGHT) == FALSE) return;  /* wrap to next line */
      view_backspace (); /* and join the lines */
      return;
      }
   if (!is_word_part (ch) && ch != ' ') { /* delete delimiter */
      view_delchar ();
      return;
      }
   while (is_word_part (view_peek ()))   /* delete word */
      view_delchar ();
   while (is_whitespace (view_peek ()))  /* then trailing spaces */
      view_delchar ();
   }
else {  /* delete word left */
   if (view_stream_cursor (LEFT) == FALSE) /* start one char left of cursor */
      return;
   while (is_whitespace (view_peek ())) { /* delete whitespace */
      view_delchar ();
      if (view_stream_cursor (LEFT) == FALSE) return;
      }
   if (view_peek () == '\n') {   /* join lines if at end of line */
      view_stream_cursor (RIGHT);
      view_backspace ();
      return;
      }
   ch= view_peek ();
   if (!is_word_part (ch) && !is_whitespace (ch)) { /* delete delimiter */
      view_delchar ();
      return;
      }
   while (is_word_part (view_peek ())) {  /* delete word */
      view_delchar ();
      if (view_stream_cursor (LEFT) == FALSE) return;
      }
   view_stream_cursor (RIGHT); /* move where first char of deleted word was */
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_delete_to_eol (void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct mark_rec *m= get_mark (v->cursor);
vptr line_v= get_line (m->row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
struct sheet *cur;
SHORT gap= h->length - m->col;

if (gap <= 0) return;
//cur= (struct sheet *)Mem::vmem_trans (active_sheet);
cur = my_active_sheet;                  // mod 8-25-93
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);

while (gap--) view_delchar ();
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_delete_to_bol (void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct mark_rec *m= get_mark (v->cursor);
vptr line_v= get_line (m->row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
struct sheet *cur;

if (m->col >= h->length) {
   m->col= h->length;
   }
while (m->col != 0) {
   view_backspace ();
   m= get_mark (v->cursor);
   }
//cur= (struct sheet *)Mem::vmem_trans (active_sheet);
cur = my_active_sheet;            // mod 8-25-93
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);
}

#ifdef FUCKUP                                  // not used 2-4-94
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*        view_pickword function            */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
static char const *legalcharset;

static bool  is_wordchar (SHORT ch)
{
char const *p= legalcharset;

if (isalnum (ch) || ch == '_') return TRUE;
if (p) {
   while (*p)
      if (*p == (char)ch) return TRUE;
      else p++;
   }
return FALSE;
}

SHORT view_pickword (char *buffer, SHORT bufsize, char const *legalchars)
/* Copies word under cursor into buffer and terminates the buffer with a null.
   At most bufsize characters, including the null terminator, will be copied.
   legalchars is a null-terminated string of legal word characters
      (all letters, numbers, and the underscore are assumed to be legal chars)
   The return value is the length of the word */
{
SHORT ch, count= 0;
bool ok= TRUE;
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
SHORT cursor_mark= v->cursor;
struct mark_rec mark_save;

legalcharset= legalchars;
mark_save= *get_mark (cursor_mark);
#pragma warn -pia
                                         /* find start of word */
while ((ok= view_stream_cursor (LEFT)) && is_wordchar (view_peek ())) { }
if (ok) view_stream_cursor (RIGHT);
                                        /* copy word into buffer */
while (count < bufsize-1 && is_wordchar (ch= view_peek ())) {
   count++;
   *(buffer++)= (char)ch;
   view_stream_cursor (RIGHT);
   }
*buffer= '\0';
*get_mark (cursor_mark)= mark_save; /* return mark to original position */
return count;
}
#pragma warn +pia             // ok=view_stream_cursor gens warning
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*        view_changecase function          */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef FUCKUP                                 // not used 2-4-94
void view_changecase (SHORT mode)
/* Changes case of word under cursor.
   mode: 0 - switch case from upper to lower or vice-versa.
         1 - force uppercase
         2 - force lowercase
*/
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct sheet *cur;
struct mark_rec *m= get_mark (v->cursor);
struct line_header *h= (struct line_header *)Mem::vmem_trans (get_line (m->row));
char *s= (char *)(h+1);
SHORT col= m->col, len= h->length;
bool makelower = TRUE;

/* find start of word */
while ((col > 0) && (col-1 <= len) && (is_word_part (s[col-1])))
   col--;

if (mode == 0) {   /* alternate case */
   while (!isalpha (s[col]) && is_word_part (s[col]) && col < len)
      col++;
   makelower= isupper (s[col]);
   }
while (col <= len && is_word_part (s[col])) {
   switch (mode) {
      case 0: /* alternate */
         s[col]= makelower ? (char)tolower (s[col]) : (char)toupper (s[col]);
         break;
      case 1: /* uppercase */
         s[col]= (char)toupper (s[col]);
         break;
      case 2: /* lowercase */
         s[col]= (char)tolower (s[col]);
         break;
      }
   col++;
   }
//cur= (struct sheet *)Mem::vmem_trans (active_sheet);
cur = my_active_sheet;                // mod 8-25-93
cur->changed= TRUE;
view_linetouch (cur->viewer, m->row);
}
#endif

#endif       // end of ifdef VIEW_EXTENDED_COM
/*========================================================================*/
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_shift (enum udlr direction)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct mark_rec *m= get_mark (v->cursor);
struct sheet *sh;

switch (direction) {
   case UP:
      if (v->firstrow > 0) {
         if (m->row == v->firstrow + v->length-1) m->row--;
         view_scroll (DOWN,1);
         }
      else if (m->row > 0) m->row--;
      break;
   case DOWN:
      sh= (struct sheet *)Mem::vmem_trans (v->sheet);
      if (sh->line_count > v->firstrow+v->length) {
         if (m->row == v->firstrow) m->row++;
         view_scroll (UP,1);
         }
      else if (m->row < sh->line_count-1) m->row++;
      break;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool locate_view (char const* s, SHORT option)
{
char fname[FNAME_LEN];
vptr v;

strcpy (fname, s);
// normalize_filename (fname, cwd, settings.default_ext);
normalize_filename_2 (fname, cwd, settings.default_ext);
v= find_view (fname);
if (v) {
   DBG_LOAD(dout<<" locate_view, view found"<<endl);
   view_make_active (v);
   view_bring_to_front();
   if (option == 2)                    /* reload file */
      return view_load_file (s,cwd,settings.default_ext);
   return TRUE;
   }
if (option != 0) {                      /* not found, load it */
   DBG_LOAD(dout<<" locate_view, view NOT found"<<endl);
   new_sheet();
   new_standard_view (0);
   return view_load_file (s,cwd,settings.default_ext);
   }
return FALSE;
}
