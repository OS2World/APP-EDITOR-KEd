/*****************************************************
file: VIEWER.C      Copyright 1989 by Dlugosz Software
   handle the sheet viewer
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "status.h"
#include "misc.h"                           /* need fillwords() */
#include <string.h>                            /* need memcpy() */
#include "setting.h"

#include "basewin.h"
#include "test.h"

#define viewset settings.viewer_defaults

basewin* vwptr[MAX_WINDOWS];                // viewer win ptr
basewin* statwptr[MAX_WINDOWS];             // status win ptr

#ifndef NO_SETTING
//extern SHORT which_view;            // holds view num, from 1 to 4
static SHORT which_view;            // holds view num, from 1 to 4
#else
static SHORT which_view;                    // if not linking edset.cpp
#endif

vptr active_view= 0L, top_view= 0L, bottom_view= 0L;
struct view_rec *my_active_view;
SHORT viewer_count= 0;
//SHORT viewer_number = 0;
static SHORT viewer_number = 0;

#ifdef USE_FILELIST
vptr filelist_view= 0L;
#endif
/*************************************************************************/

void link_in_top (struct view_rec *v, vptr view)
{                           /* makes "view" the new top_view   */
                            /* both top and bottom view point to this view */
   struct view_rec *v2;

   v->under= top_view;
   v2= (struct view_rec *)Mem::vmem_trans (top_view);    /* change top_view fields */
   v2->over= view;
   v->over= bottom_view;
   v2= (struct view_rec *)Mem::vmem_trans (bottom_view);  /* change bottom_view fields */
   v2->under= view;
   top_view= view;
}

/***********************************************************************/

void view_make_active (vptr new_view)
{
   struct view_rec *v;

if (active_view) {
   v= (struct view_rec *)Mem::vmem_trans (active_view);
   DBG_WIN1(dout<<" view_make_active calling view_border Single"<<endl);
   vwptr[v->view_num]->view_border(window_core::Single, v->colors[BORDER_COLOR]);
   }
active_view= new_view;
v= (struct view_rec *)Mem::vmem_trans (active_view);

my_active_view = v;            /* mod 6-19-92  */
v->flags |= 4;                 /* mark it for redraw   mod 7-92  */

DBG_WIN1(dout<<" view_make_active calling view_border Double"<<endl);
vwptr[v->view_num]->view_border(window_core::Double, v->colors[BORDER_COLOR]);
active_sheet= v->sheet;

my_active_sheet = (struct sheet *)Mem::vmem_trans(v->sheet);  // mod 8-25-93  
vwptr[v->view_num]->show();
status_redraw();               // TEMP MOD 6-4-94

DBG_VIEW1(dout<<" view_make_active  active_view = "<<active_view<<" active_sheet = "<<active_sheet<<endl);
}

/**************************************************************************/

vptr new_view_general (SHORT height, SHORT width, SHORT ypos, SHORT xpos, USHORT flags, vptr sheet)
{
/******************************************************************
*  MODIFIES:  active_sheet   
*             viewer_count
*             top_view
*             struct view_rec  for new view
*  NOTES:  calls create_window
*                clear_window
*                window_border
*                display_on

   SEE basewin sample code in .\tow\test5a.cpp
*******************************************************************/
       /* create a new viewer with the given window shape, attach to current
                          sheet, and (flag 1) make it the current view. */
struct view_rec *v;
struct sheet *cur_sheet;
vptr new_v;
static SHORT colorX= VIEW_DEF_COLORS;
vptr old_active_sheet= active_sheet;
viewer_count++;
viewer_number++;
//mark_rec *mycurp;

DBG_WIN2(dout<<" new view general passed params height = "<<height<<" width = "<<width<<endl);

active_sheet= sheet;          /* cursor functions require the active sheet */
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
v= (struct view_rec *)vmem_alloc (sheet_mem, sizeof (struct view_rec), &new_v);
v->view_num = viewer_number;
                                              /* attach it to the sheet */
v->sheet= sheet;
cur_sheet= (struct sheet *)Mem::vmem_trans (sheet);
v->next= cur_sheet->viewer;
cur_sheet->viewer= new_v;

//v->number= ++cur_sheet->view_count;   // mod 2-20-93
DBG_WIN1(dout<<" new view general line 133 win height = "<<height<<" width = "<<width<<endl);
v->firstrow= 0;
v->firstcol= 0;
                    //v->LeftMargin = 0;                   // mod 10-15-93
fillwords (v->touches, -1, 4);
v->length= height - (viewset.toplines + viewset.bottomlines);
v->width= width-2;         /* left & right is always one border character */
                  //v->RightMargin = v->width;           // mod 10-15-93
v->RightMargin = viewset.RightMargin;  // mod 2-5-94
v->QuotePrefix = viewset.QuotePrefix;
v->top_border= viewset.toplines;
v->cursor= new_mark();

//mycurp  = get_mark(v->cursor);
//v->mycursor.row = mycurp->row;
//v->mycursor.col = mycurp->col;

v->eol = new_mark();              // mod 2-4-94 for word wrap
                   //DBG_VIEW1(dout<<endl<<hex<<" new view  before VIEW FLAGS = "<<hex<<viewset.flags<<endl);
v->flags= viewset.flags | 4;                /* mark as needing redrawing */
                  //DBG_VIEW1(dout<<endl<<hex<<" new view  after VIEW FLAGS = "<<hex<<viewset.flags<<endl);
//if(v->flags & 32) {                          // word wrap mode
//   struct mark_rec *m= get_mark (v->cursor);
//   m->col = v->LeftMargin;
//}
DBG_WIN1(dout<<" new view general line 153 win height = "<<height<<" width = "<<width<<endl);
v->tab_size= viewset.tab_size;
v->vert_scroll_margin= viewset.vert_scroll_margin;
v->horz_scroll_margin= viewset.horz_scroll_margin;
active_sheet= old_active_sheet;           /* restore correct active sheet */
my_active_sheet = (struct sheet *)Mem::vmem_trans(active_sheet);//mod 8-25-93

memcpy (v->colors, settings.colors[colorX], sizeof(v->colors));
                                     /* cycle view color defaults so
                                        each window has one of 5 defaults */
if (++colorX > VIEW_DEF_COLORS+MAX_VIEW_COLORSETS-1)    // ie: colorX = 12
    colorX= VIEW_DEF_COLORS;                             // colorX = 8
else {
   which_view = colorX - 1;    // mod 2-19-93
   v->number = which_view;
}

#pragma warn -eas              // assign SHORT to enum warn
v->block_mode= viewset.block_mode;
v->block_active= view_rec::bk_undefined;
#pragma warn .eas                            
                                     /* create the window */
              //display_on (status_window);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_WIN1(dout<<" new view general line 182 win height = "<<height<<" width = "<<width<<endl);
vwptr[v->view_num] = new basewin(height,width,ypos,xpos);
vwptr[v->view_num]->clearwin(v->colors[0], ' ');
DBG_WIN1(dout<<" new view general calling view_border "<<endl);
vwptr[v->view_num]->view_border(window_core::Double,v->colors[BORDER_COLOR]);
vwptr[v->view_num]->show();

//statwptr[v->view_num] = new basewin(1, width - 4, height-1, 2); // OK
statwptr[v->view_num] = new basewin(*(vwptr[v->view_num]),1, width - 4, height-1, 2);
//v->statusw->clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR], ' ');
//statwptr[v->view_num]->clearwin(settings.colors[STATUS_COLORS][NORMAL_COLOR], ' ');

//statwptr[v->view_num]->show();

DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
if (flags & 1) 
    my_active_view = v;            
    view_make_active (new_v);
                                   /* link it into twll of displayed views */
if (!top_view) {                   /* is this the first view created ?  */
   top_view= bottom_view= new_v;
   v->under= v->over= new_v;
   }
else link_in_top (v,new_v);        /* make it the top_view */

status_setup();
statwptr[v->view_num]->show();
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
return new_v;
}

/*************************************************************************/

void new_view (SHORT height,SHORT width,SHORT ypos,SHORT xpos)
{
   height = Screen.get_screen_height();
   width = Screen.get_screen_width();
DBG_WIN2(dout<<" new view win height = "<<height<<" width = "<<width<<endl);
new_view_general (height, width, ypos, xpos, 1, active_sheet);
}

/*************************************************************************/

void new_standard_view (SHORT index)
{
DBG_WIN1(dout<<" new_standard_view win zoom defaults= "<<endl);
DBG_WIN1(dout<<" new_standard_view  height= "<<settings.zoom_defaults.position[index].h<<" width = "<<settings.zoom_defaults.position[index].w<<endl);

new_view (settings.zoom_defaults.position[index].h,
          settings.zoom_defaults.position[index].w,
          settings.zoom_defaults.position[index].y,
          settings.zoom_defaults.position[index].x);
}

/*************************************************************************/
          /* the sheet has been deleted.  Delete all views attached to it */
void view_delete_all (vptr view_v)
{       
while (view_v) {
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
   vptr next= v->next;
   #ifdef USE_FILELIST
   if (view_v == filelist_view) 
       filelist_view= 0L;
   #endif
   if (active_view == view_v) 
       view_switch_to_next();
   if (view_v == top_view) top_view= v->under;
   if (view_v == bottom_view) bottom_view= v->over;
   //destroy_window (v->w);
   delete vwptr[v->view_num];
   vmem_free (view_v);
   viewer_count--;
   view_v= next;
   }
}

/**************************************************************************/

void delete_view(void)
{
vptr view_v= active_view;
//struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
struct view_rec *v = my_active_view;        // mod 8-27-93 
#ifdef USE_FILELIST
if (view_v == filelist_view) 
   filelist_view= 0L;
#endif
cancel_mark (v->cursor);
//v->mycursor.row = 0;
//v->mycursor.col = 0;

if (v->block_active != view_rec::bk_undefined) {
   cancel_mark (v->block_begin);
   cancel_mark (v->block_end);
   }
view_unlink_twll();
if (active_view)
   view_switch_to_next();                        /* skip if no more views */
//destroy_window (v->w);
delete vwptr[v->view_num];
sheet_delete_view (v->sheet, view_v);         // tell the sheet about it 
vmem_free (view_v);
viewer_count--;
}

/*******************************************************************/

void view_scroll (enum udlr direction,ULONG udistance)
{
/******************************************************************
*  PURPOSE: Scrolls the text in the viewer's window in any of 4 directions. 
*           The distance may be negative.  Parameters are as in
*           scroll_window().
*  MODIFIES:  struct view_rec  is modified for active_view
*             mods v->firstrow, v->firstcol
*  CALLS:     scroll_window (only needs win struct)
*             view_redraw
*******************************************************************/

struct view_rec *v;
USHORT distance = LOUSHORT(udistance);
//ULONG distance = udistance;
if(distance == 0)
   return;
//v= (struct view_rec *)Mem::vmem_trans (active_view);
v = my_active_view;         // mod 8-27-93
basewin* wptr = vwptr[v->view_num];
DBG_WIN1(dout<<"view_scroll passed distance = "<<udistance<<" derived distance = "<<distance<<endl);
DBG_WIN1(dout<<"view_scroll org v->firstrow = "<<(v->firstrow)<<" org v->firstcol = "<<(v->firstcol)<<endl);
                //DBG_WIN2(dout<<"view_scroll before flush v->col = " <<v->firstcol<<endl);
view_flush (active_view);         
               //DBG_WIN2(dout<< "view_scroll after flush v->firstcol = "<< v->firstcol<<endl);

switch (direction) {
   case LEFT:
      v->firstcol += distance;
      DBG_WIN3(dout<<dec << "view_scroll LEFT "<<distance << " cols"<<endl);
      wptr->scroll(Left, distance);
      view_redraw (active_view, 0, v->length, v->width-distance, distance);
      break;
   case RIGHT:
//      v->firstcol -= distance;
      v->firstcol  = (distance > v->firstcol) ? 0 : (v->firstcol - distance);
      DBG_WIN3(dout<<dec << "view_scroll RIGHT "<<distance << " cols"<<endl);
      fillwords (v->touches, 0xffff, 4);
      wptr->scroll(Right, distance);
      v->flags |= 4;
      break;
   case UP:
//      v->firstrow += distance;
      v->firstrow += udistance;
      DBG_WIN3(dout<<dec << "view_scroll UP "<<udistance << " lines"<<endl);
      wptr->scroll(Up, distance);
//      view_redraw (active_view, v->length-distance, distance, 0, v->width);
//      view_redraw (active_view, v->length-udistance, v->length, 0, v->width);
      view_redraw (active_view, v->length-udistance, distance, 0, v->width);
      break;
   case DOWN:
//      v->firstrow  = (distance > v->firstrow) ? 0 : (v->firstrow - distance);
      v->firstrow  = (udistance > v->firstrow) ? 0 : (v->firstrow - udistance);
      DBG_WIN3(dout<<dec <<"view_scroll DOWN "<<distance << " lines"<<endl);
      DBG_WIN3(dout <<"view_scroll v->firstrow =  "<< v->firstrow <<endl);
      wptr->scroll(Down, distance);
      view_redraw (active_view, 0, distance, 0, v->width);
      break;
   }
}

/***********************************************************************/

void view_unlink_twll(void)
{
/* unlink the active view from the twll of all views.
   The under pointer is left intact so a subsequent call to switch_to_next()
   will go through.
   This is called by delete_view() and view_bring_to_front() */
struct view_rec *v;

//struct view_rec *v= Mem::vmem_trans (active_view);

v = my_active_view;

if (active_view == v->under) {       /* if it's the last one, 0 out fields*/
   v->under= 0L;
   v->over= 0L;
   top_view= bottom_view= active_view= 0L;
   }
else {                                          /* the normal way */
   struct view_rec *v2;
   v2= (struct view_rec *)Mem::vmem_trans (v->under);  /* unlink from twll */
   v2->over= v->over;
   v2= (struct view_rec *)Mem::vmem_trans (v->over);
   v2->under= v->under;
                                            
   if (bottom_view == active_view)           /* take care of moving bottom */
        bottom_view= v->over;
   if (top_view == active_view)                /* and top */
        top_view= v->under;
   }
}
              
