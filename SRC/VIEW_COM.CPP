/*****************************************************
file: VIEW_COM.C    Copyright 1989 by Dlugosz Software
   view commands

   for WORD_WRAP, need a function to reformat a paragraph.
   1. if user inserts chars in the middle of a paragraph,
      i need to "ripple" the insert down.
   2. if user deletes chars in the middle of a paragraph,
      i need to "ripple" the deletion down.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "keycodes.h"
#include "status.h"
#include "misc.h"                           
#include <string.h>                        /* need memcpy(), strlen() */
#include "getkey.h"                     /* need keyready() */
#include "setting.h"                      /* need view_page_mode */
#include <ctype.h>                  // isspace macro

#include "basewin.h"
#include "test.h"

#ifdef WORD_WRAP
static void view_format_paragraph(struct view_rec *v, const ULONG mrow);
int first_word_len(struct line_header *l, int max_len); // first word in line
            /* returns size of last word before len, incl rest of string*/
static int last_word_len(struct line_header *l, int len); 
static int next_word_len(const char *string, const int max);//sizeof next word
#endif

#ifdef WORD_WRAP
/*********************************************************************
          look at sheet lines and wrap downward if too long 
          do until a blank line

          Performs high-level wrap. 

          IF line > right margin
             sheet_wrapline

          NEED to SEPARATE LOGICAL "SPLIT" point from CURSOR !!

          added a cursor REF to sheet_wrapline() call so it can directly
          change the cursor position if necessary.

          rev 1/13/95
          this function always starts reformat from current cursor row.
          This causes a problem:
            if doc is long, lots of extra work checking lines already wrapped
                     
          ADD START POS (row,col) to PARAMETERS

**********************************************************************/
static void view_format_paragraph(struct view_rec *v, const ULONG mrow)
{
  int wrap_col = 0;
  int max_len = v->RightMargin - 1;
  struct mark_rec *m= get_mark (v->cursor);   // temp ??
  ULONG row = mrow;
  struct line_header *h = line_fetch (v->sheet, mrow); // get the line
  int line_len = h->length;
  char const* s = (char *)(h+1);                 // DEBUG ONLY
  struct mark_rec *ptr_eol = get_mark(v->eol);   // get a mark for wrap pos

//  DBG_SHEET(dout<< "view_format_paragraph org cursor row="<<(mrow)<<" col="<<(m->col)<<endl);
  DBG_SHEET(dout<< "view_format_paragraph row "<<row<<"  line length = "<<line_len<<" RightMargin = "<<max_len<<endl);

  if(line_len <= max_len){                    // no need to wrap
     DBG_SHEET(dout<< "view_format_paragraph  no need to wrap "<<endl);
     return;
  }                                              // else gotta do the work
  ptr_eol->row = mrow;

  do {                                        // wrap lines until blank line
       wrap_col = last_word_len(h, max_len);
       ptr_eol->col = wrap_col;            //set mark to split pos
       if(wrap_col == 0) {                 // line has no spaces
          ptr_eol->col = max_len;          // split at rmargin
       }
       DBG_SHEET(dout<<"view_format_paragraph wrapping"<<endl);
       DBG_SHEET(dout<<"line_length = "<<h->length<<" ptr_eol->row = "<<(ptr_eol->row)<<"  ptr_eol->col = "<<(ptr_eol->col));
       DBG_SHEET(dout<<"  line =±"<<((char *)(h+1))<<"°  string_len  = "<<strlen(s)<<endl);

     if(h->length > (2 * max_len)) {      // optimize
        sheet_splitline(v->eol);          // splitline at mark
     }
     else {
        if(h->length > max_len) {
           sheet_wrapline(*m, *ptr_eol);    // wrap line at mark
        //  DBG_SHEET(dout<< "return from sheet_wrap cursor row= "<<m->row<<" col= "<<m->col<<endl); 
        }
     }
     row++;
     ptr_eol->row = row;
     h = line_fetch (v->sheet, row);         // get next line
  }while(h->length != 0);           // line is blank, we're done
}

int last_word_len(struct line_header *l,int rmargin)
/************************************************************************
//   pass a line_header and right margin
//   returns number of chars in last word(s) in string that is < rmargin
//   used by view_add_char_w_margins
     ORG code returned length of str > rmargin
     NEW code returns length of whitespace str <= rmargin

     trap special condition here: if str is longer than rmargin
                                     return rmargin

     This means line will be split at rmargin in this case

     1/17/96
     mod return to increment i
     the old ver ended up putting space at start of next line
         new ver leaves space at end of 1st line

***********************************************************************/
{
int i = rmargin;
char *string = (char *)(l +1);
DBG_SHEET(dout<< "in last_word_len, string =°"<<string<<"±");
DBG_SHEET(dout<< "  rmargin = "<<rmargin);
DBG_SHEET(dout<< "  strlen = "<<strlen(string)<<endl);

    while ((i != 0) && (!isspace(string[i]))) {
       i--;
    }
    i++;

  DBG_SHEET(dout<< "end last_word_len  word_len = "<< i<<endl);
  if(i == 1)
     i = rmargin;    // line str had no spaces, wrap at rmargin

return i;
}
// #endif     // WORD_WRAP


/************************************************************************
   Find length of first word(s) on line, up to max_len

   ie: line = ±this is a test±
   if max_len = 9
      return 8

   because that is the largest chunk of words that can fit
   pass a line_header and max_len we can accept
   start search at end or str or max_len, it's a little easier
   used by view_del_char_w_margins    (to wrap UP)
***********************************************************************/
int first_word_len(struct line_header *l, int max_len)
{
int word_len = max_len;

char *string = (char *)(l +1);
DBG_SHEET(dout<<"first_word_len line len = "<<l->length<<" max_len = "<<max_len<<endl);
                      
  if(max_len > l->length) {
     max_len = l->length;
     DBG_SHEET(dout<<"first_word_len max_len now = "<<max_len<<endl);
  }
  while((!isspace(string[max_len])) && max_len) { 
     max_len--;
     word_len--;
  }
DBG_SHEET(dout<<"first_word_len returning word_len = "<<word_len<<endl);
return word_len;
}

/*********************************************************************

          starting from cursor row, look at current line length and
          first words on next line. If there is room on current line
          for words from next line, move them up

          repeat for rest of paragraph 
          (we assume other paragraphs have been wrapped)


      NEW 1/16/96 ver    ATTEMPT TO WORK ON PARAGRAPH

          del  a char in  view and implement margins 
          called by  del_char or backspace

      1/27/95  no paragraph, just current line and next line.
               let reform_doc_down take take of paragraph
**********************************************************************/
static void view_delchar_w_margins(struct view_rec *v, const ULONG mrow)
{
  DBG_SHEET(dout<<" in view_delchar_w_margins"<<endl);
//  struct mark_rec *m= get_mark (v->cursor);
//  ULONG  curr_row = m->row;
  ULONG  curr_row = mrow;
  ULONG  next_row = curr_row + 1;
  mark_t tmp_mark = new_mark();
  struct mark_rec* temp_cursor = get_mark(tmp_mark);
  
  struct line_header *h    = line_fetch (v->sheet, curr_row); // get cur line
  struct line_header *next = line_fetch (v->sheet, next_row); // get next line

  DBG_SHEET(dout<<"view_delchar_w_margin curr row "<<curr_row<<" = °"<<(char *)(h+1)<<"°"<<endl);
  DBG_SHEET(dout<<"view_delchar_w_margin next row "<<next_row<<" = °"<<(char *)(next+1)<<"°"<<endl);

  char* next_str = (char *)(next+1);               // next line
  int len = h->length;                      
  if(next->length == 0)   // no next line....
     return;
  int next_len = next_word_len(next_str, next->length);//1st word next line len

//while(next->length != 0) { 
  if(((v->RightMargin - len) >= next_len) && (next_len != 0)) { //room for next word ?
     DBG_SHEET(dout<<"view_delchar_w_margin space for next word next_len = "<<next_len<<endl);
     temp_cursor->row = curr_row;
     temp_cursor->col = len;     
     DBG_SHEET(dout<<"view_delchar_w_margin adding str = ±"<<next_str<<"±"<<endl);
     sheet_addstr(tmp_mark, next_str, next_len + 1);  // add to end of line
     temp_cursor->row = next_row;
     temp_cursor->col = next_len;
     DBG_SHEET(dout<<"view_delchar_w_margin delchar deleting 1st "<<next_len<<" chars in row "<<next_row<<endl);
     while(next_len >= 0) {
      // DBG_SHEET(dout<<"view_delchar_w_margin delchar in next_len"<<endl);
       sheet_delchar(tmp_mark);
       next_len--;
       temp_cursor->col--;
     }
  }
//  curr_row = next_row;
//  next_row = curr_row + 1;
//  h    = line_fetch (v->sheet, curr_row); // get cur line                               
//  next = line_fetch (v->sheet, next_row); // get next line
//  next_str = (char *)(next+1);               // next line
//  len = h->length;                      
//  next_len = next_word_len(next_str, next->length);//1st word next line len
//  DBG_SHEET(dout<<"view_delchar_w_margin curr row "<<curr_row<<" = °"<<(char *)(h+1)<<"°"<<endl);
//  DBG_SHEET(dout<<"view_delchar_w_margin next row "<<next_row<<" = °"<<(char *)(next+1)<<"°"<<endl);
//}
  cancel_mark(tmp_mark);
}


#ifdef FUCKUP        // ORG, works ok
/*********************************************************************

          starting from cursor row, look at current line length and
          first words on next line. If there is room on current line
          for words from next line, move them

          This only works on current and next line, not a whole paragraph

          del  a char in  view and implement margins 
          called by  del_char or backspace
**********************************************************************/
static void view_delchar_w_margins(struct view_rec *v)
{
  DBG_SHEET(dout<<" in view_delchar_w_margins"<<endl);
  struct mark_rec *m= get_mark (v->cursor);
  ULONG  curr_row = m->row;
  ULONG  next_row = curr_row + 1;
  mark_t tmp_mark = new_mark();
  struct mark_rec* temp_cursor = get_mark(tmp_mark);
  
  struct line_header *h    = line_fetch (v->sheet, curr_row); // get cur line
  struct line_header *next = line_fetch (v->sheet, next_row); // get next line

  DBG_SHEET(dout<<"view_delchar_w_margin curr row "<<curr_row<<" = °"<<(char *)(h+1)<<"°"<<endl);
  DBG_SHEET(dout<<"view_delchar_w_margin next row "<<next_row<<" = °"<<(char *)(next+1)<<"°"<<endl);

  char* next_str = (char *)(next+1);               // next line
  int len = h->length;                      
  if(next->length == 0)   // no next line....
     return;
  int next_len = next_word_len(next_str, next->length);//1st word next line len

  if(((v->RightMargin - len) >= next_len) && (next_len != 0)) { //room for next word ?
     DBG_SHEET(dout<<"view_delchar_w_margin space for next word next_len = "<<next_len<<endl);
     temp_cursor->row = curr_row;
     temp_cursor->col = len;     
     DBG_SHEET(dout<<"view_delchar_w_margin adding str = "<<next_str<<endl);
//     sheet_addchars_wordwrap(tmp_mark, next_str, next_len + 1); // works ok
     sheet_addstr(tmp_mark, next_str, next_len + 1);
     temp_cursor->row = next_row;
     temp_cursor->col = next_len;
     DBG_SHEET(dout<<"view_delchar_w_margin delchar deleting moved words"<<endl);
     while(next_len >= 0) {
      // DBG_SHEET(dout<<"view_delchar_w_margin delchar in next_len"<<endl);
       sheet_delchar(tmp_mark);
       next_len--;
       temp_cursor->col--;
     }
  }                           
  cancel_mark(tmp_mark);
}
#endif

static int next_word_len(const char *string, const int max)
/************************************************************************
   returns number of chars in next word in string
   does not include the trailing space after the word
   used by view_del_char_w_margins

   what about if I have 2 or more spaces at start ??
***********************************************************************/
{
int word_len = 0;
//int i  = strlen(string);
                         // ORG STUFF
  if(isspace(string[0]))        // is first char space?
     word_len++;

  while (!isspace(string[word_len]) && (word_len <= max)) //count chars in word
    word_len++;


//  while (isspace(string[word_len]))  // step through leading spaces
//     word_len++;

//  while(word_len <= max) {
//    if(!isspace(string[word_len]))    //count chars in word
//       word_len++;
//  }

return word_len;    
}

void reform_doc_down(int num_paragraphs)
/***********************************************************************
      reformat document downward to the end from current cursor.
      if num_paragraphs = 1, do one paragraph.
      if num_paragraphs = 0, do til end.

      get line length
      while line_length != 0
         if room for next word, add it.
         if line too long, wrap it.
         get line length

**************************************************************************/
{
  DBG_SHEET(dout<<"reform_doc_down"<<endl);
  bool DONE = FALSE;
  struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
  struct mark_rec *m= get_mark (v->cursor);
  ULONG org_curr_row = m->row;
  SHORT org_curr_col = m->col;

  ULONG curr_row = m->row;  

  struct line_header *h;
  int len;
  struct sheet *s= (struct sheet *)Mem::vmem_trans (v->sheet);
  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

  h = line_fetch (v->sheet, curr_row); // get cur line
  DBG_SHEET(dout<<"reform_doc curr row "<<curr_row<<" = °"<<(char *)(h+1)<<"°"<<endl);
  len = h->length;               

  while(!DONE) {
     if(len > v->RightMargin) {           // wrap this line down
        view_format_paragraph(v, curr_row);
     } 

     curr_row++;
     m->row++;
     h = line_fetch (v->sheet, curr_row); // get cur line
     DBG_SHEET(dout<<"reform_doc curr row "<<curr_row<<" len = "<<h->length<<" = °"<<(char *)(h+1)<<"°"<<endl);

     len = h->length;               
     if(num_paragraphs == 1) {
        if(len == 0) 
           DONE = TRUE;
     }        
     if(curr_row == s->line_count)
        DONE = TRUE;
  }                     // end while

  m->row = org_curr_row;       // restore org cursor
  m->col = org_curr_col;
  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}

#endif     // WORD_WRAP

/************************************************************************/
static void  auto_indent (struct view_rec *v)
{
                               /* skip leading spaces on previous line */
struct mark_rec *m= get_mark (v->cursor);
struct line_header *h;
char const* s;
SHORT col= 0;
LONG row = m->row;

//DBG_SHEET(dout<<"    IN auto_indent  "<<endl);

/* check above lines until one is found with something on it */
if (m->row == 0) 
   return;                   /* just in case */
do {
   h= line_fetch (v->sheet, --row);
   } while (h->length == 0 && row > 0);     // don't count CRLF lines
if (h->length == 0) 
   return;                   /* didn't find a line to use */

s= (char *)(h+1);                              // get the string

//DBG_SHEET(dout<< "auto_indent before add ' ' old str = %s\t h->length = %d\n", s, h->length);

while ((col < h->length) && (*s == ' ')) {    // count leading spaces in it
   col++;
   s++;
 }                                       /* 'col' is how much to indent */

//DBG_SHEET(dout<< "indenting  cols = %d\t h->length = %d\n", col, h->length);

//if(col == (h->length)) {                  // line is all spaces
//  return;
//}

h= line_fetch (v->sheet, m->row);
if (h->length == 0) 
   m->col= col;                             /* position cursor */
else {                                      /* type some spaces */
   while (col--) 
      sheet_addchar (v->cursor, ' ');
   }
// DBG_SHEET(dout<<"auto_indent  after add ' ' new str =  "<<(char *)(h +1)<<
//                                 "  h->length = "<< h->length<<endl);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  indent (struct view_rec *v)
{
struct mark_rec *m= get_mark (v->cursor);
struct line_header *h;
char const* s;
SHORT col;
LONG row;                                            

if (m->row == 0) return;
                         /* read line above cursor to find next indention */
row= m->row;
do {
   h= line_fetch (v->sheet, --row);
   } while (h->length == 0 && row > 0);
if (h->length == 0) return; /* didn't find a line to use */
s= (char *)(h+1);

col= m->col;
while (col < h->length && s[col] != ' ') 
       col++;                              /* skip word */
while (col < h->length && s[col] == ' ') 
       col++;                              /* skip whitespace */
if (v->flags&1 && h->length > 0) {                   /* insert mode? */
   SHORT old_col= m->col;
   while (old_col++ < col) 
      sheet_addchar (v->cursor,' ');
   }
else 
      m->col= col;           /* overwrite mode just moves cursor over text */
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_tab (struct view_rec *v)
{
if (v->flags&8) {  /* use tabs? */
   SHORT mycol = get_mark(v->cursor)->col;
   SHORT distance= v->tab_size-(mycol % v->tab_size);
   DBG_WIN1(dout<<"view_tab cursor col = "<<mycol<<" tab_size = "<<(v->tab_size)<<endl);
   if (v->flags & 1) {               /* ins mode */
      mark_t cursor= v->cursor;
      do {
          sheet_addchar (cursor,' ');
        mycol++;
      } while((mycol % v->tab_size) != 0);
   }
   else move_mark(v->cursor,RIGHT,distance);
   }
else indent (v);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef NO_COMMANDO                 // ie: defined COMMANDO
void view_addkey_any (vptr view_v, char c)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
vptr old_sheet= active_sheet;
active_sheet= v->sheet;
if (c == LOUCHAR(Enter)) {
   sheet_splitline (v->cursor);
   if (v->flags & 2) auto_indent (v);
   }
else if (c == LOUCHAR(kTAB)) 
         view_tab (v);
else if (c == 0x08)            // backspace
         view_backspace();
else {
   if (v->flags & 1)                         /* ins mode */ 
      sheet_addchar (v->cursor, c);
   else 
      sheet_ovwtchar (v->cursor, c);
   }
active_sheet= old_sheet;
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_addkey (char c)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);

DBG_VIEW1(dout<<"view_addkey adding char = "<<c<<endl);

switch (c) {
    case 0x0D:                       // ENTER
         sheet_splitline (v->cursor);
         if (v->flags & 2) 
            auto_indent (v);
         break;
    case 0x09:                       // TAB
         view_tab (v);
         break;
    case 0x08:                      // backspace
         DBG_VIEW1(dout<<"view_addkey backspace "<<endl);
         view_backspace();
         break;
    default:
         if(v->flags & 32) {                           /* word_wrap mode */
            DBG_VIEW1(dout<<"view_addkey about to WRAP "<<endl);
            sheet_addchar (v->cursor, c);
            #ifdef WORD_WRAP
            mark_rec *m = get_mark(v->cursor);  // argh, kludge...
      //      view_format_paragraph(v);   
            view_format_paragraph(v, m->row);   
            #endif
         }
         else if (v->flags & 1) {                          /* ins mode */ 
            DBG_VIEW1(dout<<"view_addkey not WRAPPING "<<endl);
            sheet_addchar (v->cursor, c);
         }
         else {
            DBG_VIEW1(dout<<"view_addkey overwrite "<<endl);
            sheet_ovwtchar (v->cursor, c);
         }                                    // end handle normal
}
#ifdef FUCKUP                 // this was org
if (c == LOUCHAR(Enter)) {
   sheet_splitline (v->cursor);
   if (v->flags & 2) 
      auto_indent (v);
}
else if (c == LOUCHAR(kTAB)) {                   // handle Tab
         view_tab (v);
}
else if (c == 0x08) {           // backspace
         view_backspace();
}                                      
else {                                   // handle "normal" chars
   if(v->flags & 32) {                           /* word_wrap mode */
     DBG_VIEW1(dout<<"view_addkey about to WRAP "<<endl);
     sheet_addchar (v->cursor, c);
     view_format_paragraph(v);     // this is better than reform_doc
   }
else if (v->flags & 1) {                          /* ins mode */ 
       DBG_VIEW1(dout<<"view_addkey not WRAPPING "<<endl);
       sheet_addchar (v->cursor, c);
   }
   else 
       sheet_ovwtchar (v->cursor, c);
   }                                    // end handle normal
#endif
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_addstring (const char* s)
{
struct view_rec *v = (struct view_rec *)Mem::vmem_trans (active_view);
sheet_addstr(v->cursor, s, strlen(s));
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef NO_COMMANDO
void view_addstring_any (vptr view_v, char const* s)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
vptr old_sheet= active_sheet;
active_sheet= v->sheet;
sheet_addstr(v->cursor, s, strlen(s));
active_sheet= old_sheet;
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
void view_move_cursor (enum udlr direction)
{
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct view_rec *v = my_active_view;             /* mod 6-19-92 */

   if(v->flags & 32) {                           /* word_wrap mode */
      view_stream_cursor(direction);
   }
   else {
      move_mark (v->cursor, direction, 1);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */                               
bool view_stream_cursor (enum udlr direction)
{
/* Moves cursor through file sequentially, wrapping at each line */
/* returns FALSE if cursor not moved */
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct sheet *sh= (struct sheet *)Mem::vmem_trans (active_sheet);// mod 011893
struct view_rec *v = my_active_view;             /* mod 8-27-93 */
struct sheet *cur = my_active_sheet;             // mod 8-27-93
struct mark_rec *m = get_mark(v->cursor);         // mod 8-27-93
if((v->flags & 32) && (direction == RIGHT)) {  /* word_wrap mode */
   DBG_VIEW1(dout<<"view_stream_cursor word wrap RIGHT  m->col="<<m->col<<endl);
   if(m->col >= v->RightMargin -1) {
      DBG_VIEW1(dout<<"view_stream_cursor wrap cursor "<<endl);
      m->row++;
      m->col = 0;
      return TRUE;
   }
}
            //return stream_mark (v->cursor, direction);
return stream_mark_fast (cur, m, direction);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_delchar(void)
{
DBG_SHEET(dout<<" in view_delchar"<<endl);
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
sheet_delchar (v->cursor);
#ifdef WORD_WRAP
  view_delchar_w_margins(v, get_mark_row(v->cursor));
  reform_doc_down(1);
#endif
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_backspace(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
DBG_SHEET(dout<<" in view_backspace"<<endl);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
if (move_mark(v->cursor,LEFT,1))  //  move left if not at left margin
   sheet_delchar (v->cursor);
else 
   sheet_joinlines(v->cursor);    // join current line to prev line
#ifdef WORD_WRAP
   view_delchar_w_margins(v,get_mark_row(v->cursor));
   reform_doc_down(1);
#endif

}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_pageup(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);

if (settings.viewer_defaults.view_page_mode == 1) {
         // move cursor one pagelength up, and scroll back to keep it in
         // the same spot on the display
DBG_WIN1(dout<<"view_pageup mode = 1 ");
DBG_WIN1(dout<<"m->row =  "<<m->row<<" v->firstrow = "<<v->firstrow<<endl);

if (m->row >= v->length) {                        /* move a page up */
   m->row -= v->length;
   view_scroll (DOWN, v->length);
   }
else {                                            /* move to the top */
   view_scroll (DOWN, v->firstrow);
//   m->row= 1;
   m->row= 0;                          // MOD 6-5-94
   }
}
else {                       // view_page_mode == 0
 //if cursor is at or above top scroll margin, move a window length up.
 //  otherwise, move cursor to top margin. 
SHORT windowrow= m->row - v->firstrow;

DBG_WIN1(dout<<"view_pageup mode = 0");
DBG_WIN1(dout<<"m->row =  "<<m->row<<" v->firstrow = "<<v->firstrow<<endl);
DBG_WIN1(dout<<"windowrow =  "<<windowrow<<" vert_scroll_margin = "<<v->vert_scroll_margin<<endl);

if (windowrow > v->vert_scroll_margin) {      /* move cursor to margin */
   m->row= v->firstrow + v->vert_scroll_margin;
   }
else {                                        /* move a page up */
   if (m->row >= (USHORT)v->length) 
       m->row -= v->length;
   else 
//       m->row= 1;
       m->row= 0;                // MOD 6-5-94
   }
}
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_pagedown(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);
struct sheet *cursheet= (struct sheet *)Mem::vmem_trans (v->sheet);
ULONG linecount= cursheet->line_count;

if (settings.viewer_defaults.view_page_mode == 1) {
      // move the cursor one pagelength down, and scroll the display to
      // keep the cursor in the same spot. 
  ULONG linesleft= (linecount-1) - m->row;
//  SHORT delta= (linesleft > v->length) ? v->length : linesleft;
  SHORT delta= (linesleft > v->length) ? v->length : 0;

DBG_WIN1(dout<<"view_pagedn mode = 1");
DBG_WIN1(dout<<"m->row =  "<<m->row<<" v->firstrow = "<<v->firstrow);
DBG_WIN1(dout<<"delta =  "<<delta<<endl);

  if(delta == 0)
    m->row = (linecount-1);
  else       
    m->row += delta;
  view_scroll (UP, delta);
}
else {                         // view_page_mode == 0
  // if cursor is at or below the bottom scroll margin, move a window length
  //   down.  otherwise, move cursor to bottom margin. 
  ULONG windowrow= m->row - v->firstrow;    /* mod 5-92 was signed int */
  ULONG newrow;

DBG_WIN1(dout<<"view_pagedn mode = 0");
DBG_WIN1(dout<<"m->row =  "<<m->row<<" v->firstrow = "<<v->firstrow<<endl);
DBG_WIN1(dout<<"windowrow =  "<<windowrow<<" vert_scroll_margin = "<<v->vert_scroll_margin<<endl);

  if (windowrow >= v->length-1 -v->vert_scroll_margin) {
   newrow= m->row + v->length;                  /* move cursor a page down */
   }
else {
   newrow= v->firstrow + v->length -1;           /* move to scroll margin */
   }
if (newrow >= linecount) 
   m->row= linecount-1;
else 
   m->row= newrow;
}                       // end ELSE page_mode = 0
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_gohome(void)
{
/* move the cursor to the first non-space on the line.  If it is already
   there or to the left, move to column 0 */
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);
struct line_header *l= line_fetch (v->sheet, m->row);
char const* s= (char *)(l+1);      /* string starts just after the header */
SHORT n;

for (n= 0; n < l->length && *s == ' '; n++) 
     s++;                             /* n is number of first non-space */
if (m->col <= n) m->col= 0;
else m->col= n;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_goend(void)
{
/* move the cursor 1 char beyond the end of the line */
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);
struct line_header *l= line_fetch (v->sheet, m->row);

m->col= l->length;  /* nothing to it. */
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_gotoline (ULONG line)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);
struct sheet *cursheet= (struct sheet *)Mem::vmem_trans (v->sheet);

if (line >= cursheet->line_count) {
    m->row= cursheet->line_count-1;
//    m->row= cursheet->line_count - v->length;    // MOD 9-9-94 for Ctrl-Pgdn
}
else 
    m->row= line;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_zapline(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
struct mark_rec *m= get_mark (v->cursor);
sheet_delete_lines (m->row, 1);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_switch_to_next(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
view_make_active (v->under);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_switch_to_prev(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
view_make_active (v->over);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_bring_to_front(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
if (top_view != active_view) {            /* make it so */
   view_unlink_twll();
   link_in_top(v, active_view);         /* make active_view the top_view */
   }
//display_on (v->w);                  /* bring my window to the front */
//display_on (status_window);         /* make sure it is not covered */
  vwptr[v->view_num]->show();

}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

vptr find_view (char const* name)
{
vptr p= top_view;
SHORT loop= viewer_count;
while (loop--) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (p);
   struct sheet *s= (struct sheet *)Mem::vmem_trans (v->sheet);
   if (!strcmp(name, s->filename)) return p;
   p= v->under;  /* try the next one down */
   }
return 0L;  /* not found */
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool view_block_active(void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
return (v->block_active == view_rec::bk_on);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT view_peek (void)
{
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct view_rec *v = my_active_view;             /* mod 6-19-92 */
return sheet_peek (active_sheet, v->cursor);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool view_insertmode (SHORT mode)
/*  0-off  1-on  2-toggle  3-ask only */
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
//struct view_rec *v = my_active_view;             /* mod 6-19-92 */
switch (mode) {
   case 0: v->flags &= ~1;   break;
   case 1: v->flags |= 1;    break;
   case 2: v->flags^= 1;     break;
   case 3: /* nothing */     break;
   }
//status_insertmode (v->flags&1);
status_redraw();
return (1 == (v->flags&1));
}

/****************************************************************/
void  select_view(void)
{
vptr v= view_pick();
if (v) {
   view_make_active (v);
   view_bring_to_front();
   }
}


/******************************************************************/
