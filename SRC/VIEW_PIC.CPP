/*****************************************************
file: VIEW_PIC.C    Copyright 1989 by Dlugosz Software
   a picklist for available viewers
*****************************************************/

#include "usual.h"
#include "vmem.h"
                                  //#include "window.h"
#include "sheet.h"
#include "view.h"
#include "setting.h"
#include "picklist.h"
#include "misc.h"
#include "help.h"
#include <string.h>  /* need strcpy() and strlen() */

#include "basewin.h"

basewin *picw;

static vptr cur_view;
static SHORT cur_index;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static struct view_rec *indexit (SHORT index)
{
struct view_rec *v;

if (index >= viewer_count) return NULL;
v= (struct view_rec *)Mem::vmem_trans (cur_view);
while (index > cur_index) {
   /* head down in the list */
   cur_view= v->under;
   v= (struct view_rec *)Mem::vmem_trans (cur_view);
   assert (cur_view != 0);
   cur_index++;
   }
while (index < cur_index) {
   cur_view= v->over;
   v= (struct view_rec *)Mem::vmem_trans (cur_view);
   cur_index--;
   }
/* v points to current view */
return v;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static char const* fetch_choice (SHORT index)
{   /* this is passed as a functional parameter to picklist driver. */
struct view_rec *v;
struct sheet *s;
static char fname[FNAME_LEN];
char *name= fname;

memset(fname, 0x00, FNAME_LEN);
v= indexit (index);
if (!v) return NULL;
//if (v->number != 1) {  /* same format as in STATUS.C status_name() */
//   char image[10], *im;
//   im= num2str (image, (SHORT)v->number, 5, 0);
//   strcpy (fname, im);
//   name= fname+strlen(im);
//   *name++= '\x07';
//   }
s= (struct sheet *)Mem::vmem_trans (v->sheet);
strcpy (name, s->filename);
return fname;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

vptr view_pick (void)
{
static struct picklist_portal p= { 1, 1, 10, 40 };
static struct picklist_thunk t= {NULL, fetch_choice, NULL};
//window_t w;
SHORT x;
SHORT rows= viewer_count;
DEF_HELP (VIEWPICK_HELP,1,10,10);

NEW_HELP;
if (rows > 15) rows= 15;  /* maximum window size */
p.rows= rows;
cur_view= top_view;
cur_index= 0;
p.normal_color= settings.colors[MENU_COLORS][NORMAL_COLOR];
p.cursor_color= settings.colors[MENU_COLORS][HIGHLIGHT_COLOR];
//w= create_window (2+rows, 42,
//   settings.location[PICKBOX_LOCATION].y,settings.location[PICKBOX_LOCATION].x);
//clear_window (w, 0,1+rows, 0,41, (settings.colors[MENU_COLORS][NORMAL_COLOR]<<8)|' ');
//window_border (w, nq_fat_border, settings.colors[MENU_COLORS][BORDER_COLOR]);
//display_on (w);
picw = new basewin (2+rows, 42,
   settings.location[PICKBOX_LOCATION].y,settings.location[PICKBOX_LOCATION].x);
picw->clearwin(settings.colors[MENU_COLORS][NORMAL_COLOR],' ');
picw->border(window_core::Double);
picw->show();

x= picklist (&p,&t, viewer_count, PICKBOX_LOCATION);  /* here it goes */
//destroy_window (w);
delete picw;
OLD_HELP;
if (x == -1) return 0L;
indexit (x);
return cur_view;
}
