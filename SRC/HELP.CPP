/*****************************************************
 file: HELP.C       Copyright 1989 by Dlugosz Software
    this module provides context-sensitive help
*****************************************************/
#include "usual.h"
#include "help.h"
#include "mylib.h"
#include "getkey.h"
#include "keycodes.h"
#include "setting.h"
#include "redbox.h"
#include "misc.h"
#include <string.h>

#include "basewin.h"

struct help_index {  /* help record as it appears in file */
   SHORT locus,screen,width,height,page_count;
   long pos;
   };

#define MAX_PAGES 10  /* maximum pages per screen */

static struct help_rec *help;  /* currently active help structure */
basewin *helpw;

static bool (* OldFilter) (SHORT);
static USHORT help_key;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void init_help (USHORT key)
{
if (key) {
   help_key= key;
   if (key_preprocess != do_help) 
      {
      OldFilter= key_preprocess;
      key_preprocess= do_help;
      }
   }
else
   key_preprocess= OldFilter;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

struct help_rec *set_help (struct help_rec *help_info)
/* change to new help data and return ptr to old help data */
{
struct help_rec *old= help;  /* remember currently active help */
help= help_info;
return old;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  show_page (mylib_file_t file, unsigned long pos, window_t w, SHORT page, struct help_index *data)
{
   SHORT row= 1;
   char len;
   char line[256];

//   clear_window (w,1,data->height-2,1,data->width-2,
//       ((settings.colors[HELP_COLORS][NORMAL_COLOR])<<8)|' ');
//   window_border (w,single_border,settings.colors[HELP_COLORS][BORDER_COLOR]);
   mylib_random_read (file,pos,&len,1);  /* read length of string */
   do {
      if (len == 0)
         row++;
      else {
         mylib_read_text (file,line,len); /* read line from help file */
         if (line[0] == '!') { /* Title preceeded by '!' */
            SHORT col= (data->width-(--len))/2;
         //   put_string_len (w,0,col,line+1,len);
         //   highlight (w,0,col,settings.colors[HELP_COLORS][HIGHLIGHT_COLOR],len);
            helpw->title(TCenter, line+1);
            }
         else if (line[0] != '$')
      //      put_string_len (w,row++,1,line,len);
            helpw->put(row++,1,line,len);
         }
      mylib_read_text (file,&len,1); /* read length of next line */
      } while (line[0] != '$');
   if (data->page_count > 1) {
      strcpy (line,"Page ");
      num2str (line+5,page+1,2,' ');
      strcat (line," of ");
      num2str (line+11,data->page_count,2,' ');
//      put_string (w,data->height-1,data->width-14,line);
      helpw->put(data->height-1,data->width-14,line);
      }
}

//static bool do_help (USHORT key)
bool do_help (USHORT key)
{
SHORT page;
//window_t w;
mylib_file_t file;
unsigned long page_pos[MAX_PAGES];
struct help_index index_data;
static inhelp= FALSE;
 
if (inhelp || key != help_key) {
   if (OldFilter) return OldFilter(key);  /* jump to old routine */
   else return TRUE;
   }
if (help->locus == 0) return FALSE; /* locus 0 means no help availiable */
inhelp= TRUE;

if (mylib_open (settings.help_path, &file, MYLIB_READONLY)) {
   do {
      if (mylib_read_text (file,(char *)&index_data,sizeof index_data) == 0 ||
          index_data.locus == -1) { /* unable to find help screen */
         redbox (13,200,eh_abort);
         mylib_close (file);
         inhelp= FALSE;
         return FALSE;
         }
   } while (index_data.locus != help->locus || index_data.screen != help->screen);

   index_data.height+= 2;
   index_data.width+= 2; /* account for window borders! */

   /* calculate window's x coordinate */
   if (help->x == -1) help->x= (w_width-index_data.width)/2;
   if (help->x+index_data.width > w_width) help->x=w_width-index_data.width;

   /* calculate window's y coordinate */
   if (help->y == -1) help->y= (w_height-index_data.height)/2;
   if (help->y+index_data.height > w_height) help->y= w_height-index_data.height;

   helpw = new basewin(index_data.height, index_data.width, help->y, help->x);
   helpw->clearwin((settings.colors[HELP_COLORS][NORMAL_COLOR]),' ');
   helpw->border(window_core::Single);
   helpw->show();
//   w= create_window (index_data.height,index_data.width,help->y,help->x);
//   clear_window (w,0,index_data.height-1,0,index_data.width-1,(settings.colors[HELP_COLORS][NORMAL_COLOR]<<8)|' ');
//   window_border (w,single_border,settings.colors[HELP_COLORS][BORDER_COLOR]);
//   display_on (w);

   page= 0;
   page_pos[0]= index_data.pos;
   show_page (file,page_pos[0],w,0,&index_data);
   for (;;)
      switch (key= getkey ()) {
         case PageUp:
            if (page > 0) {
               page--;
               show_page (file,page_pos[page],w,page,&index_data);
               }
            break;
         case PageDown:
            if (key == PageDown && page < index_data.page_count-1 && page < MAX_PAGES-1) {
               page_pos[++page]= mylib_tell (file)-1; /* -1 because last show_page read one char of page 2 */
               show_page (file,page_pos[page],w,page,&index_data);
               }
            break;
         default:
         //   if (keymove_window (w,-1,key)) break; /* arrow keys */
            /* ESC and helpkey will not be caught by next call to getkey() */
        //    if (key != kESC && key != help_key) {
        //        OldKey= key;
        //    }
            goto endhelp;
         } /* switch */
   endhelp:
//   destroy_window (w);
   delete helpw;
   mylib_close (file);
   }
else redbox_s (2,205,eh_continue,settings.help_path);

inhelp= FALSE;
return FALSE;
}
