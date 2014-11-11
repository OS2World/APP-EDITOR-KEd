/*****************************************************
File: SEARCH.C      copyright 1989 by Dlugosz Software
   This is the search and replace module.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "search.h"
#include "block.h"
#include "status.h"
#include "getkey.h"
#include "keycodes.h"
                           //#include "misc.h" /* for upcase() */
#include <string.h>
#include <ctype.h>

#include "basewin.h"

#ifdef REGEXP
static char    *start_sub[1],
               *end_sub[1];  /* to mark substrings */
#endif

static bool ignore_case, words_only;

/* **************************************** */

inline static char upcase(char _c)
{
  return islower(_c) ? (_c&~32) : (_c);   // mod 12-24-92
}

inline static bool is_wordpart(SHORT ch)
{
return (ch >= 'a' && ch <= 'z') || 
       (ch >= 'A' && ch <= 'Z') || 
       ch == '_';
}

/* **************************************** */

static bool  searchstr (char const *s, SHORT startcol, char const *pattern, char const **start, char const **end, SHORT len)
/* returns pointers around string match */
{
char const *p;
char const *leftmar= s;
char first= (char)(ignore_case ? upcase(*pattern) : *pattern);
SHORT patlen= strlen (pattern);

s += startcol;
len -= patlen-1;
while (len > 0) {                   /* look for first character in pattern */
   if (ignore_case) {
      if (pattern[1] == '\0' && upcase (*s) == first) {
         *start= s;
         *end= s+1;
         return TRUE;               /* single char pattern matched */
         }
      while (upcase (*s) != first && len) { s++; len--; }
      }
   else {
      if (pattern[1] == '\0' && *s == first) {
         *start= s;
         *end= s+1;
         return TRUE;               /* single char pattern matched */
         }
      while (*s != first && len) { 
         s++; len--; 
      }
   }

   if (len == 0) break;
   if (words_only && s != leftmar && is_wordpart (s[-1])) {
      s++; len--;
      continue;                     /* not word beginning, reject */
      }

   *start= s;                       /* mark beginning of find */
   s++; len--;
   if (words_only && is_wordpart((*start)[patlen])) continue;
                                  
   p= pattern+1;                   /* match remaining pattern */
   if (ignore_case)
      while (*p) {
         if (upcase (*s) != upcase (*p)) break;
         s++; p++;
         }
   else while (*p) {
      if (*s != *p) break;
      s++; p++;
      }
   if (*p == '\0') {                 /* was entire pattern matched? */
      *end= s;
      return TRUE;
      }
   s= *start+1;                    /* not found; continue where we left off */
   }                               /* end while len */
return FALSE;
}

/* **************************************** */

static bool  searchstr_backwards (char const *s, SHORT startcol, char const *pattern, char const **start, char const **end, SHORT len)
/* returns pointers around string match */
{
char const *p;
char const *leftmar= s;
char first= (char)(ignore_case ? upcase (*pattern) : *pattern);
SHORT patlen= strlen (pattern);

s += startcol;
len -= patlen-1;
while (len > 0) {
                                 /* look for first character in pattern */
   if (ignore_case) {
      if (pattern[1] == '\0' && upcase (*s) == first) {
         *start= s;
         *end= s+1;
         return TRUE;            /* single char pattern matched */
         }
      while (upcase (*s) != first && s >= leftmar) { s--; len++; }
      }
   else {
      if (pattern[1] == '\0' && *s == first) {
         *start= s;
         *end= s+1;
         return TRUE;            /* single char pattern matched */
         }
      while (*s != first && s >= leftmar) { s--; len++; }
      }
   if (s < leftmar) return FALSE;  /* didn't match first character of pattern */
   if (words_only && s > leftmar && is_wordpart (s[-1])) {
      s--; len++;
      continue;  /* not word beginning, reject */
      }

   *start= s;  /* mark beginning of find */
   s++; len--;
   if (words_only && is_wordpart((*start)[patlen])) continue;
   p= pattern+1;                          /* match remaining pattern */
   if (ignore_case)
      while (*p) {
         if (upcase (*s) != upcase (*p)) break;
         s++; p++;
         }
   else while (*p) {
      if (*s != *p) break;
      s++; p++;
      }
   if (*p == '\0') {  /* was entire pattern matched? */
      *end= s;
      return TRUE;
      }
   s= *start-1;  /* not found; continue where we left off */
   len-= 2;
   } /* end while */
return FALSE;
}

/* ******************************************************************* */

/* Globals needed by search functions */
static USHORT search_flags;              
static struct mark_rec cursor, *tail;
static mark_t cursor_m, tail_m;
static char const *search_s;
static char const *replace_s;
static SHORT (*verify_func)(mark_t, mark_t, char const *, SHORT, char const *, bool);
static USHORT row_subcount;  // was ULONG, only need 0-10 for this
static SHORT found_count;
SHORT  replace_count;            // needed by view_sch.cpp
static ULONG row_count;       
static SHORT last_repl_choice;

/* **************************************** */
#ifdef REGEXP
static void  create_replacestr (char const *str, char *buffer, bool real)
{
SHORT len, n;
while (*str) {
   switch (*str) {
      case '\\':  /* take next character literally */
         *buffer++= *++str;
         break;
      case '&':
      case '#': /* copy found substring */
         if (*str == '&') n= 0;
         else n= *++str - '0';
         if (n >= 0 && n <= 9 && start_sub[n]) {
            len= (SHORT)(end_sub[n] - start_sub[n]);
            strncpy (buffer, start_sub[n], len);
            buffer+= len;
            }
         break;
      case '$': /* carriage return */
         if (real) *buffer++= '\r';
         else {
            *buffer++= '\\';
            *buffer++= 'n';
            }
         break;
      case '^': /* place cursor */
         *buffer++= (char)((real) ? 0x1b : '^'); /* ESC represents ^ (place cursor) */
         break;
      case '%': /* delete character */
         *buffer++= (char)((real) ? '\b' : '%'); /* BACKSPACE represents % (delchar) */
         break;
      default:
         *buffer++= *str;
      } /* switch */
   if (*str) str++; /* don't advance past end of string */
   } /* while */
*buffer= '\0';
}
#endif


/**************************************************************************


***************************************************************************/
static bool  do_replace (char *found_str, SHORT len)
{
#ifdef REGEXP
char reg_replbuf[256]; /* need safeguard  overflow of reg_replace[] buffer */
#endif

char const *replace_str = replace_s;
int repl_str_len = strlen(replace_str);
bool replace_once= FALSE;

#ifdef REGEXP
               /* create string to show what replacement will look like */
if((search_flags&(REGEXP_SEARCH|QUERY_REPLACE))==(REGEXP_SEARCH|QUERY_REPLACE)) {
   create_replacestr (replace_s, reg_replbuf, FALSE);
   replace_str= reg_replbuf;
   }
else replace_str= replace_s;
#endif

DBG_SHEET(dout<<"search.cpp do_replace "<<endl);
if ((search_flags & QUERY_REPLACE) && verify_func)
   switch (last_repl_choice=
          (*verify_func)(cursor_m, tail_m, found_str, len, replace_str, search_flags & SEARCH_BLOCK)) {
      case 1: /* YES, replace */
         break;
      case 2: /* NO, don't replace but search for next occurance */
         return TRUE;
      case 3: /* replace ONCE then stop */
         replace_once= TRUE;
         break;
      case 4: /* replace ALL occurances, and don't ask again */
         verify_func= NULL;
         search_flags &= ~QUERY_REPLACE;
         break;
      case 5: /* STOP; don't continue */
         return FALSE;
      }
#ifdef REGEXP
if (search_flags & REGEXP_SEARCH) {
   ULONG row;
   SHORT col;
   bool cursor_saved= FALSE;
   char *s= reg_replbuf;
   struct mark_rec cursor_save;

   create_replacestr (replace_s, reg_replbuf, TRUE); /* recreate string before pointers are lost */
   while (*s) {
      switch (*s) {
         case '\r':
            sheet_splitline (cursor_m);
            break;
         case '\x1b':
            cursor_saved= TRUE;
            cursor_save= *get_mark (cursor_m);
            break;
         case '\b':
            row= get_mark (cursor_m)->row;
            col= get_mark (cursor_m)->col;
            if (col == ((line_header *)(Mem::vmem_trans (get_line (row))))->length-1)
               sheet_joinlines (cursor_m);
            else 
               sheet_delchar (cursor_m);
            break;
         default:
            sheet_addchar (cursor_m, *s);
         } /* switch */
      s++;
      } /* while */
   if (cursor_saved) 
      *get_mark (cursor_m)= cursor_save;
   }        /* end if search_flags... */
#endif

while (len--) {
 //  DBG_SHEET(dout<<"search.cpp do_replace  sheet_delchar"<<endl);
   sheet_delchar (cursor_m);
}

if (!(search_flags & REGEXP_SEARCH)) {
   sheet_addstr(cursor_m,replace_str, repl_str_len);
}

if (verify_func) 
   view_flush (active_view);
replace_count++;
return !replace_once;
}

/* **************************************** */

static bool search_backwards (ULONG row, SHORT cur_col, SHORT start_col, SHORT end_col)
{
/* Search backwards a row starting at cur_col up to start_col. The pattern */
/* match may go no further than end_col */
/* -- Returns TRUE to tell caller to stop searching; FALSE if search should advance to next line */

SHORT length;
char *str, *start, *end;
struct line_header *line_hdr;

if (key_ready () && getkey () == kESC) return TRUE;

/* display row currently being searched if search is prolonged */
if (++row_count >= 100)
   if (++row_subcount >= 10) {  /* only display every 10 lines to save time */
      row_subcount= 0;
      status_row (row);
      }
if (cur_col != -1) cur_col-= start_col;

search_loop:
line_hdr= (struct line_header *)Mem::vmem_trans (get_line (row));
str= (char *)(line_hdr+1);
length= line_hdr->length;
if (start_col >= length) return FALSE;   /* starting past end of existing line */
if (cur_col == -1) cur_col= length-1;
if (end_col == -1 || end_col >= length) end_col= length-1; /* don't look past end of string */
if (cur_col >= length) cur_col= length-1;

#ifdef REGEXP
if (search_flags & REGEXP_SEARCH) {
   if (!regexp_search (str,cur_col, &start, &end, end_col-cur_col+1, search_flags)) {
      if (cur_col-- > 0) goto search_loop;  /* special repeat for regexp search */
      else return FALSE;
      }
   }
else
#endif
   if (!searchstr_backwards (str+start_col,cur_col,search_s,(char const **)&start,(char const **)&end,cur_col+1))
      return FALSE;
                             /* string was found */
found_count++;
tail= get_mark(tail_m);
cursor.row= tail->row= row;  /* move marks to delimit string found */
cursor.col= (SHORT)(start-str);
*get_mark(cursor_m)= cursor;
tail->col= (SHORT)(end-str);
if (replace_s) {
   if (!do_replace (start, (SHORT)(end-start))) return TRUE;
   if (--cur_col < 0) return FALSE;
   goto search_loop; /* look for another occurance on the same line */
   }
else return TRUE;
}

/***************************************************************************
 Search a row starting at start_col and ending at end_col 
 Returns TRUE  to tell caller to stop searching; 
         FALSE if search should advance to next line 

************************************************************************/
static bool  search_line (ULONG row, SHORT cur_col, SHORT start_col, SHORT end_col)
{
SHORT length, orig_end_col= end_col;
char *str, *start, *end;
struct line_header *line_hdr;
//struct view_rec *v;
//v = my_active_view;               // mod 5-26-94
//basewin *win = vwptr[v->view_num];
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
            /* display row currently being searched if search is prolonged */
   if (++row_subcount >= 10) {  /* only display every 10 lines to save time */
//      if (key_ready()) {                  // don't block if no key waiting
//          if (getkey () == kESC)        
//            return TRUE;
//      }
      row_subcount= 0;
      status_row (row);
   }

cur_col-= start_col;

search_loop:
line_hdr= (struct line_header *)Mem::vmem_trans (get_line (row));
str= (char *)(line_hdr+1);
length= line_hdr->length;
if (start_col >= length) 
   return FALSE;                     /* starting past end of existing line */
if (end_col == -1 || end_col >= length) 
    end_col= length-1;                   /* don't look past end of string */

#ifdef REGEXP
if (search_flags & REGEXP_SEARCH) {
   if (!regexp_search (str+start_col,cur_col, &start, &end, end_col-(start_col+cur_col)+1, search_flags)) return FALSE;
   }
else
#endif

if (!searchstr (str+start_col,cur_col,search_s,(char const **)&start,(char const **)&end, end_col-(start_col+cur_col)+1)) 
    return FALSE;               // not found
                                /* else, string was found */
found_count++;
//tail= get_mark(tail_m);
tail= get_mark_fast(tail_m);    // mod 3-14-93
cursor.row= tail->row= row;     /* move marks to delimit string found */
cursor.col= (SHORT)(start-str);
tail->col= (SHORT)(end-str);
//win->highlight();
*get_mark(cursor_m)= cursor;
  if (replace_s) {
     if (!do_replace (start, (SHORT)(end-start))) 
       return TRUE;
     else {
        cursor= *get_mark_fast(cursor_m);
        cur_col= cursor.col - start_col;
        end_col= orig_end_col; /* if end_col = -1,new end_col will be recomputed when the line is retrieved */
        if (last_repl_choice == 2) 
           cur_col++;
        goto search_loop;    // look for another occurance on the same line 
     }                        /* never gets here */
  }
  else 
    return TRUE;
}

/*********************************************************************/
SHORT search (const char *search_str, const char *replace_str,
    mark_t cursor_mark, mark_t tail_mark, mark_t start_mark, mark_t end_mark,
    SHORT direction, USHORT flags)
                               /* returns number of times string was found */
{
SHORT cur_col;
ULONG start_row, end_row, cur_row;
struct mark_rec cursor;

                        // globals accessed by searching functions 
search_s= search_str;
replace_s= replace_str;
cursor_m= cursor_mark;
tail_m= tail_mark;
verify_func= view_verify_replace;
search_flags= flags;
ignore_case= flags & SEARCH_IGNORE_CASE;
words_only= flags & SEARCH_WORDS_ONLY;

found_count= 0;
replace_count= 0;
cursor= *get_mark (cursor_mark);

if (direction == 1) { /* Search FORWARD */
   if (flags & SEARCH_BLOCK) {
      struct mark_rec bk_start, bk_end;
      SHORT start_col;

      bk_start= *get_mark (start_mark);
      bk_end= *get_mark (end_mark);
      if (cursor_in_block (&cursor,&bk_start,&bk_end,flags&15)) {
         start_row= cursor.row;
         start_col= cursor.col;
         }
      else { /* start at top of block */
         start_row= bk_start.row;
         start_col= bk_start.col;
         }
      end_row= bk_end.row;

      switch (flags&15) { /* blockmode */
         case view_rec::bk_columns:
            while (start_row <= end_row) {
               if (search_line (start_row++,start_col,bk_start.col,bk_end.col-1))
                  return found_count;
               start_col= bk_start.col;
               }
            break;
         case view_rec::bk_lines:
            while (++start_row < end_row) {
               if (search_line (start_row++,start_col,0,-1)) return found_count;
               start_col= 0; /* after first row, start at col 0 in succeeding lines */
               }
            break;
         case view_rec::bk_stream:
            if (start_row == end_row) { /* search one line only */
               if (search_line (start_row,start_col,bk_start.col,get_mark(end_mark)->col-1))
                  return found_count;
               break;
               }
                                          /* first line: tail end only */
            if (start_row == bk_start.row)
               if (search_line (start_row++,start_col,bk_start.col,-1))
                  return found_count;
               else start_col= 0;   /* search next lines starting at col 0 */
                                         /* all middle lines */
            while (start_row < end_row)
               if (search_line (start_row++,start_col,0,-1)) return found_count;
               else start_col= 0;
                                        /* last line: first part only */
            if (search_line (start_row,start_col,0,get_mark(end_mark)->col-1))
               return found_count;
            break;
         }
                            /* did not match in block */
      return found_count;  /* return "found" since replace may have found one time but not the next time */
      }
   else {                              /* Non-block search */
//      end_row= ((struct sheet *)Mem::vmem_trans (active_sheet))->line_count-1;
      end_row= (my_active_sheet)->line_count-1;
      if (search_line (cursor.row,cursor.col,0,-1)) return found_count;
      while (++cursor.row <= end_row)
         if (search_line (cursor.row,0,0,-1)) 
            return found_count;
      }
   return found_count;
   }
else if (direction == -1) { /* Search BACKWARD */
   long row; /* must be long for signed comparisons (normally row is unsigned) */
   if (flags & SEARCH_BLOCK) {
      struct mark_rec bk_start, bk_end;

      bk_start= *get_mark (start_mark);
      bk_end= *get_mark (end_mark);
      if (cursor_in_block (&cursor, &bk_start, &bk_end, flags&15)) {
         cur_row= cursor.row;
         cur_col= cursor.col;
         }
      else {
         cur_row= bk_end.row;  /* start at end of block if cursor is outside it */
         cur_col= bk_end.col;
         }
      switch (flags&15) {
         case view_rec::bk_columns:
            while (cur_row >= bk_start.row) {
               if (search_backwards (cur_row--,cur_col,bk_start.col,bk_end.col-1))
                  return found_count;
               cur_col= bk_end.col-1;
               }
            break;
         case view_rec::bk_lines:
            while (cur_row >= bk_start.row) {
               if (search_backwards (cur_row,cur_col,0,-1)) return found_count;
               cur_row--;
               cur_col= -1;
               }
            break;
         case view_rec::bk_stream:
            if (cur_row == bk_start.row && cur_row == bk_end.row) {
               search_backwards (cur_row,cur_col,bk_start.col,bk_end.col-1);
               break;
               }
                                      /* last line:  first part only */
            if (cur_row == bk_end.row) {
               if (search_backwards (cur_row,cur_col,0,bk_end.col-1)) return found_count;
               cur_row--;
               cur_col= -1;
               }
                                     /* middle lines */
            if (cur_row > bk_start.row)
               while (cur_row > bk_start.row) {
                  if (search_backwards (cur_row,cur_col,0,-1)) return found_count;
                  cur_row--;
                  cur_col= -1;
                  }
                                   /* first line: end part only */
            if (search_backwards (cur_row,cur_col,bk_start.col,-1)) return found_count;
         }
      return found_count;
      }
   row= cursor.row;                /* Non-block search */
   if (cursor.col == -1) row--;
   if (search_backwards (row--,cursor.col,0,cursor.col)) return found_count;
   while (row >= 0)
      if (search_backwards (row--,-1,0,-1)) return found_count;
   return found_count;
   }
return 0;                         /* in case direction was not 1 or -1 */
}
