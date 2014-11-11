/*****************************************************
file: VIEW_SCH.C    Copyright 1989 by Dlugosz Software
   view search command
*****************************************************/
#include "usual.h"
#include "menu.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "keycodes.h"
#include "status.h"
#include "search.h"
#include "getkey.h"
#include "misc.h"
#include "edx.h"                      /* needs upcase() */
#include <string.h>                      /* needs strlen() */
#include "setting.h"
#include "redbox.h"
#include "help.h"
#include "filelist.h"

#include "basewin.h"

#define MAX(a,b) (a)>(b) ? (a) : (b)
#define MIN(a,b) (a)<(b) ? (a) : (b)

#define Color settings.colors[DIALOG_COLORS]
#define locus 8
static SHORT file_findcount;  /* occurances in file (for view_multifile_search() only) */
static SHORT file_replacecount; /* number of replacements in file */

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT view_verify_replace (mark_t head, mark_t tail, char const *found_str, SHORT found_len, char const *replace_str, bool inblock)
{
SHORT r = 0, col, col2, width, replace_len;
SHORT row, row2;
byte color_on, color_off;
USHORT fullkey = 0x0000;
DEF_HELP (SEARCH_HELP,20,-1,-1);
struct view_rec *v = my_active_view; 
basewin *docw = vwptr[v->view_num];
basewin *searchw;

//v= (struct view_rec *)Mem::vmem_trans (active_view);

v->cursor= tail;
view_cursor_inrange (&v, &row2, &col2);  /* make end of string visible */
v->cursor= head;
view_cursor_inrange (&v, &row2, &col2);  /* then make start of str visible */

if (inblock) {
   color_on= v->colors[BLOCK_CURSOR_COLOR];
   color_off= v->colors[BLOCK_COLOR];
   }
else {
   color_on= v->colors[HIGHLIGHT_COLOR];
   color_off= v->colors[NORMAL_COLOR];
   }
                                       /* decide where to put the popup */
replace_len= strlen (replace_str);
width= MAX (found_len, replace_len);
if (width < 46) 
    width= 46;
if (width > Screen.get_screen_width()-2) 
    width= Screen.get_screen_width()-2;

row = vwptr[v->view_num]->x_pos();
col = vwptr[v->view_num]->y_pos();
row += row2 +2;
col += col2 -20;
if (row > Screen.get_screen_height()-7) 
   row -= 9;
if (col < 0) 
   col= 0;
else if (col > (Screen.get_screen_width()-(width+2))) 
         col= Screen.get_screen_width()-width-2;
                                           /* highlight string on window */
 docw->highlight(row2,col2, color_on, found_len);
                                                      /* do the popup */
searchw = new basewin(7,width+2, row,col);
searchw->clearwin((Color[NORMAL_COLOR]),' ');
searchw->border(window_core::Double,Color[BORDER_COLOR]);
searchw->show();

searchw->put(1,1,"Replace ");
searchw->put(2,1,found_str,MIN (width,found_len));
searchw->put(3,1,"with ");
searchw->put(4,1,replace_str,MIN (width,replace_len));
searchw->put(5,1,"Y-Yes  N-No  O-Once  A-Replace All  ESC-Cancel");
searchw->highlight (2,1,Color[BLOCK_CURSOR_COLOR],MIN (width,found_len));
searchw->highlight (4,1,Color[BLOCK_CURSOR_COLOR],MIN (width,replace_len));
searchw->highlight (5,1,Color[BLOCK_COLOR],width);

NEW_HELP;
while (r == 0) {
//   switch (ch = upcase(LOUCHAR(getkey ()))) {
   fullkey = getkey();
   if(fullkey == kESC) {
      r = 5;
   }
   else {
      switch (upcase(LOUCHAR(fullkey))) {
         case 'Y': 
                  r= 1; break;
         case 'N': 
                  r= 2; break;
         case 'O': 
                  r= 3; break;
         case 'A': 
                  r= 4; break;
 //      case kESC: 
 //               r= 5; break;
      }
   }
}
OLD_HELP;
delete searchw;
docw->highlight(row2,col2, color_off, found_len);  // unhighlight found str
return r;
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool view_search (const char* search_str, const char* repl_str,SHORT direction,USHORT flags, bool findnext)
{
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct view_rec *v;
v = my_active_view;               // mod 8-27-93
mark_t bstart_mark = 0, bend_mark = 0, tail_mark = 0, cursor_mark= v->cursor;
struct mark_rec *m;
struct mark_rec *foundm;              // highlight find
static char const *to_search= NULL;
bool retval= TRUE;
SHORT old_col;
SHORT count = 0;

static SHORT mcol, old_findcol;
static SHORT mrow, old_findrow;
static bool old_find = FALSE;
static SHORT found_length = 0;

#ifdef REGEXP
if (flags & REGEXP_SEARCH) {
   if (search_str)
      switch (regexp_compile (search_str)) {
         case FALSE:
            redbox (locus,200,eh_continue);
            return FALSE;
         case -1:
            redbox (locus,201,eh_continue);
            return FALSE;
         }
      }
else /* not regexp search */
#endif
   if (search_str) 
      to_search= search_str;   /* remember string */
   else 
      search_str= to_search;   // use old search string if search_str is NULL 

if (flags & SEARCH_BLOCK && v->block_active == view_rec::bk_on) {
   bstart_mark= v->block_begin;   /* use block marks if block is set */
   bend_mark= v->block_end;
   }
else flags &=~SEARCH_BLOCK;
tail_mark= new_mark ();
if (findnext && repl_str == NULL) {  /* move mark one char in search dir to search for next occurance */
   m= get_mark (cursor_mark);
   old_col= m->col;
   if (direction == 1 || m->col > 0)
      m->col+= direction;    /* don't move into col -1 if search backwards */
   }
count= search (search_str,repl_str,v->cursor,tail_mark,bstart_mark,
              bend_mark,direction,flags|v->block_mode);
file_findcount+= count;
file_replacecount+= replace_count;
if(count != 0) {                  // highlight find
   if(old_find) {                     // unhighlight the last find
      DBG_VIEW1(dout<<"view_search  UNHIGHLIGHT color = 0x"<<hex<<v->colors[NORMAL_COLOR]<<dec<<" row = "<<old_findrow<<" col ="<<old_findcol<<" len = "<<found_length<<endl);
      vwptr[v->view_num]->highlight(old_findrow, old_findcol,v->colors[NORMAL_COLOR],found_length+1);
      old_find = FALSE;
   }
   foundm = get_mark(tail_mark);
   m = get_mark(cursor_mark);
   old_findcol = m->col;
   mcol = m->col;
   mrow = m->row;
   found_length = (foundm->col - m->col);
   DBG_VIEW1(dout<<"view_search  found_length = "<<found_length<<" beg col="<<old_findcol<<endl);
   view_cursor_inrange (&v, &mrow, &mcol);  // make sure string is visible 
   old_findrow = mrow;
   DBG_VIEW1(dout<<"view_search  HIGHLIGHT color = 0x"<<hex<<v->colors[SEARCH_COLOR]<<dec<<endl);
   vwptr[v->view_num]->highlight(mrow, mcol,v->colors[SEARCH_COLOR],found_length);
   old_find = TRUE;
}

if (count == 0) {
   if(old_find) {                     // unhighlight the last find
      DBG_VIEW1(dout<<"view_search  find count = 0 "<<endl);
      vwptr[v->view_num]->highlight(old_findrow, old_findcol,v->colors[NORMAL_COLOR],found_length+1);
      old_find = FALSE;
   }
   if (findnext && repl_str == NULL) {
      m= get_mark (cursor_mark);
      m->col= old_col;
      }
   status_message ("--- Not found ---");
   retval= FALSE;
   }

if (repl_str) {
   if (replace_count > 0) {   /* show # of replacements */
      char str[30], image[7];
      strcpy (str,"Replaced ");
      strcpy (str+9, num2str (image,replace_count,6,'\0'));
      strcat (str," occurances");
      status_message (str);      // mod 1-28-94
      if(old_find) {                     // unhighlight the last find
         DBG_VIEW1(dout<<"view_search unhighlight last find "<<endl);
         vwptr[v->view_num]->highlight(old_findrow, old_findcol,v->colors[NORMAL_COLOR],found_length+1);
         old_find = FALSE;
      }
   }
   retval= FALSE;
   }
cancel_mark (tail_mark);
return retval;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
static enum {search_only, search_replace, search_notset} search_mode= search_notset;
static char search_str[81] = "", replace_str[81] = "";

void view_search_next (void)
{
if (search_mode == search_notset) 
    redbox (locus,202,eh_continue);
else view_search_multifiles (
   NULL,
   (search_mode == search_replace) ? replace_str : (char   *)NULL,
   1,  /* direction forward */
   settings.search_defaults.flags|SEARCH_BLOCK,
   TRUE);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_search_prev (void)
{
if (search_mode == search_notset) redbox (locus,202,eh_continue);
else view_search (
   NULL,
   (search_mode == search_replace) ? replace_str : (char   *)NULL,
   -1,
   settings.search_defaults.flags|SEARCH_BLOCK,
   TRUE);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  get_search_str (void)
{
return prompt_box ("Find:",search_str,80,NULL,64,
     settings.search_defaults.flags&REGEXP_SEARCH?4:3);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  get_replace_str (void)
{
return prompt_box ("Replace:",replace_str,80,NULL,
          64,settings.search_defaults.flags&REGEXP_SEARCH?6:5);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_ask_search (void)
{
if (!get_search_str ()) return;
search_mode= search_only;
file_findcount= 0;
view_search_multifiles (
   search_str,
   NULL,
   settings.search_defaults.direction,
   SEARCH_BLOCK|settings.search_defaults.flags,
   FALSE);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_ask_replace (void)
{
if (!get_search_str () || !get_replace_str ()) return;
search_mode= search_replace;
file_findcount= file_replacecount= 0;
view_search_multifiles (
   search_str,
   replace_str,
   settings.search_defaults.direction,
   SEARCH_BLOCK|settings.search_defaults.flags,
   FALSE);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef USE_FILELIST
static bool  multifile_search_prompt (bool search_and_replace)
{
//window_t w;
char s[32];  /* buffer for formatting info */
char *fname;
bool kept= FALSE;
SHORT ch;
DEF_HELP (SEARCH_HELP,30, 8,17);

filelist_next();
status_message (NULL); /* clear any search/replace messages */
if (file_findcount == 0) {
   fname= filelist_fetch ();
   if (fname) {
      if (!view_load_file (fname,NULL,NULL)) return FALSE;
      return TRUE;
      }
   }
//w= create_window (6,40,10,10);
//window_border (w,fat_border,Color[BORDER_COLOR]);
//clear_window (w,1,4,1,38,(Color[NORMAL_COLOR]<<8)|' ');
//highlight (w,4,1,Color[BLOCK_COLOR],38);
num2str (s,file_findcount,4,' ');
strcat (s," occurances found");
file_findcount= 0;
//put_string (w,1,1,s);
if (search_and_replace) {
   num2str (s,file_replacecount,4,' ');
   strcat (s," replacements");
//   put_string (w,2,1,s);
   }
//display_on (w);

NEW_HELP;
next_file_prepare:
   fname= filelist_fetch ();
   pad_window (w,3,12,' ',27);
   pad_window (w,4,1,' ',38);
   if (fname) {
      SHORT len= strlen (fname);
      put_string (w,3,1,"Next file: ");
      put_string (w,3,12,fname + (len > 27 ? (len-27) : 0));
      put_string (w,4,1," Next   Skip   Cancel   Keep file");
      }
   else {
      put_just_string (w,3,CenterJust,"--- END OF FILE LIST ---");
      put_just_string (w,4,CenterJust," Press any key to continue ");
      }

menu_choose:
   ch= upcase (getkey ());
   if (fname) {
      switch (ch) {
         case 'K':
            if (!kept) {  /* only do this once */
               new_sheet();
               new_standard_view (3);
               kept= TRUE;
               display_on (w);
               }
            goto menu_choose;
         case 'C':
         case kESC:
            filelist_prev ();  /* return to last row */
            destroy_window (w);
            OLD_HELP;
            return FALSE;
         case 'S':  /* skip next file in filelist */
            filelist_next ();
            goto next_file_prepare;
         case 'N':
            break;  /* this is the go-ahead */
//         default:
//            keymove_window (w,-1,ch); /* check arrow keys */
            goto menu_choose;  /* try again */
         }
      }

//destroy_window (w);
OLD_HELP;
if (file_replacecount > 0) {         /* save file if replacements made */
   file_replacecount= 0;
   if (!view_resave_file ()) return FALSE;
   }

if (fname) {                         /* load next file */
   if (!view_load_file (fname,NULL,NULL)) return FALSE;
   return TRUE;
   }
else return FALSE;  /* end of list */
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_search_multifiles (const char* search_str, const char* repl_str,SHORT direction,USHORT flags, bool findnext)
{
bool b;

for (;;) {
   b= view_search (search_str, repl_str, direction, flags, findnext);
   if (b) 
      return;             // found the string
#ifndef USE_FILELIST
//   if (!filelist_view)  // didn't find the string, return anyway
   else
      return; 
#else
   if (!filelist_view || !multifile_search_prompt(repl_str != NULL)) 
      return;             /* user says to stop */
#endif
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT searchmenu()
{
static char *choices[]= {"Search","Replace","Next","Previous",
   "Direction    xxxxxxxx", "Case sensitive    xxx", "Match words only  xxx",
    "Query replace     xxx" };
static SHORT lastchoice= 0;
SHORT pick;
DEF_HELP (SEARCH_HELP,2, 20,10);

NEW_HELP;
do {
   strcpy (choices[4]+13,settings.search_defaults.direction == -1 ? "BACKWARD" : " FORWARD");
   menu_yesno (choices[5],18,(settings.search_defaults.flags & SEARCH_IGNORE_CASE) ? 0:1);
   menu_yesno (choices[6],18,(settings.search_defaults.flags & SEARCH_WORDS_ONLY) ? 1:0);
   menu_yesno (choices[7],18,(settings.search_defaults.flags & QUERY_REPLACE) ? 1:0);
   switch (pick= menu (choices,8,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|SEARCH_LOCATION,21)) {
      case 0:                        /* search */
         view_ask_search ();  break;
      case 1:                        /* replace */
         view_ask_replace ();  break;
      case 2:                        /* next */
         view_search_next ();  break;
      case 3:                        /* previous */
         view_search_prev ();  break;
      case 4:                        /* direction */
         settings.search_defaults.direction*= -1;  break;
      case 5:                        /* case sensitive */
         settings.search_defaults.flags^= SEARCH_IGNORE_CASE;  break;
      case 6:                        /* match words only */
         settings.search_defaults.flags^= SEARCH_WORDS_ONLY;  break;
      case 7:                        /* query replace */
         settings.search_defaults.flags^= QUERY_REPLACE;  break;
      } /* switch */
   } while (pick >= 4);
OLD_HELP;
return 0;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*   packup & unpack                        */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

