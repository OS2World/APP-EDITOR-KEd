/*****************************************************
file: VIEW_ZM.C     Copyright 1989 by Dlugosz Software
   part of the sheet viewer.  This is the function
   view_zoom_driver() and other helper functions that
   manipulate the window's size and position.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "getkey.h"
#include "keycodes.h"
#include "help.h"
#include "status.h" /* need status_note() */
#include "setting.h"

#include "basewin.h"

#include "test.h"

bool view_is_zoomed= FALSE;
static int speed= 1;


//    vwptr[v->view_num]->

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static enum { T_move, T_size } T_mode= T_move;

static void m_s (enum udlr direction) /* move or size */
{
switch (T_mode) {
   case T_move:  view_move_window (direction, speed);  break;
   case T_size:  view_size_window (direction, speed);  break;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_movetoedge (enum udlr edge)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans(active_view);
int old_y,old_x,y_pos,x_pos,width,height,old_width,old_height;

//window_get_position (v->w, &old_y, &old_x);
//window_get_size (v->w, &old_height, &old_width);

old_height = vwptr[v->view_num]->window_core::height();
old_width = vwptr[v->view_num]->window_core::width();
old_y = vwptr[v->view_num]->window_core::y_pos();
old_x = vwptr[v->view_num]->window_core::x_pos();

y_pos= old_y;
x_pos= old_x;
height= old_height;
width= old_width;

if (T_mode == T_move) { /* move window to edge */
   switch (edge) {
      case UP:
         y_pos= 0; break;
      case DOWN:
//         y_pos= w_height-height-1; break;
         y_pos= Screen.height - height-1; break;
      case LEFT:
         x_pos= 0; break;
      case RIGHT:
//         x_pos= w_width-width; break;
         x_pos= Screen.width - width; break;
      }
   view_move_window (UP, old_y-y_pos);
   view_move_window (LEFT, old_x-x_pos);
   }
else { /* resize window border to edge */
   switch (edge) {
      case UP:
         height+= y_pos;
         y_pos= 0;
         break;
      case DOWN:
//         height+= w_height-(y_pos+height)-1;
         height+= Screen.height-(y_pos+height)-1;
         break;
      case LEFT:
         width+= x_pos;
         x_pos= 0;
         break;
      case RIGHT:
//         width+= w_width-(x_pos+width);
         width+= Screen.width-(x_pos+width);
         break;
      }
   view_move_window (UP, old_y-y_pos);
   view_move_window (LEFT, old_x-x_pos);
   view_size_window (DOWN, height-old_height);
   view_size_window (RIGHT, width-old_width);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//void view_reshape (int height, int width, int y_pos, int x_pos)
void view_reshape (SHORT height, SHORT width, SHORT y_pos, SHORT x_pos)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans(active_view);
int oldh, oldw, oldY, oldX;

//window_get_position (v->w, &oldY, &oldX);
//window_get_size (v->w, &oldh, &oldw);

oldh = vwptr[v->view_num]->window_core::height();
oldw = vwptr[v->view_num]->window_core::width();
oldY = vwptr[v->view_num]->window_core::y_pos();
oldX = vwptr[v->view_num]->window_core::x_pos();


view_move_window (UP, oldY-y_pos);
view_move_window (LEFT, oldX-x_pos);
view_size_window (UP, oldh-height);
view_size_window (LEFT, oldw-width);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

enum scale { whole, first, last };

static void position (enum scale vert, enum scale horz)
{
int y_pos, x_pos, height, width;
//int right= w_width;
int right= Screen.width;
//int bottom= w_height-1;  /* reserve last row for status line */
int bottom= Screen.height-1;  /* reserve last row for status line */
y_pos= (vert == last)? bottom/2 : 0;
x_pos= (horz == last)? right/2 : 0;
height= (vert == first)? bottom/2 : bottom-y_pos;
width= (horz == first)? right/2 : right-x_pos;
view_reshape (height, width, y_pos, x_pos);
view_is_zoomed= FALSE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void set_custom (int index)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans(active_view);

//window_get_position (v->w, &settings.zoom_defaults.position[index].y,
//                           &settings.zoom_defaults.position[index].x);
//window_get_size (v->w, &settings.zoom_defaults.position[index].h,
//                       &settings.zoom_defaults.position[index].w);
settings.zoom_defaults.position[index].h = vwptr[v->view_num]->window_core::height();
settings.zoom_defaults.position[index].w = vwptr[v->view_num]->window_core::width();
settings.zoom_defaults.position[index].y = vwptr[v->view_num]->window_core::y_pos();
settings.zoom_defaults.position[index].x = vwptr[v->view_num]->window_core::x_pos();

}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void call_custom (int index)
{
view_reshape (settings.zoom_defaults.position[index].h,
              settings.zoom_defaults.position[index].w,
              settings.zoom_defaults.position[index].y,
              settings.zoom_defaults.position[index].x);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static int zoom_driver_guts(void)
{
/* this lets the user manipulate the window based on other keys typed.  It
   returns when a non-used key is typed, and returns that key.  If a position
   key is pressed, it returns right away with a zero value.    */
int key;

for(;;) {
   key= getkey();
   switch (key) {
      /* arrow keys move or size */
      case kUp:   m_s(UP);    break;
      case kDown: m_s(DOWN);  break;
      case kLeft: m_s(LEFT);  break;
      case kRight:m_s(RIGHT); break;
      case kHOME: view_movetoedge (LEFT);  break;
      case kEND:  view_movetoedge (RIGHT); break;
      case PageUp: view_movetoedge (UP);   break;
      case PageDown: view_movetoedge (DOWN); break;
      case '+': T_mode= T_move;  break;
      case '-': T_mode= T_size;  break;
      case '*': view_bring_to_front (); break;
      case '/': view_switch_to_next (); break;
      case '[': if (--speed < 1) speed= 1; break;
      case ']': if (++speed > 9) speed= 9; break;
      /* numbers position window.  assignments are based on the positions of
         the number on the 10key pad  */
      case '8': /* top half */
         position (first, whole);   return 0;
      case '2': /* bottom half */
         position (last, whole);    return 0;
      case '4': /* left half */
         position (whole, first);   return 0;
      case '6': /* right half */
         position (whole, last);    return 0;
      case '7': /* upper left corner */
         position (first, first);   return 0;
      case '9': /* upper right corner */
         position (first, last);    return 0;
      case '1': /* lower left corner */
         position (last, first);    return 0;
      case '3': /* lower right corner */
         position (last, last);     return 0;
      case '5': /* full screen */
         position (whole, whole);   return 0;
      case '0':  /* zoom */
         view_zoom();  return 0;
      default:
         if (key <= 26) {
            set_custom (key-1);
            return 0;
            }
         else if (key >= 'a' && key <= 'z') {
            call_custom (key-'a');
            return 0;
            }
         else if (key >= 'A' && key <= 'Z') {
            call_custom (key-'A');
            return 0;
            }
         else return key;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT view_zoom_driver(void)
{
int retval;
DEF_AUTOHELP (ZOOM_HELP,1);

NEW_HELP;
status_note ("     Zoom Driver   (+)Move (-)Size (*)Front (/)Next   (F1)Help");
retval= zoom_driver_guts ();
status_note (NULL);
OLD_HELP;
return retval;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void view_zoom(void)
{
static int oldh, oldw, oldY, oldX;

if (view_is_zoomed) {
   view_reshape (oldh, oldw, oldY, oldX);  /* put it back */
   view_is_zoomed= FALSE;
   }
else {                             /* zoom it out to full screen */
                                   /* memorize former position */
   struct view_rec *v= (struct view_rec *)Mem::vmem_trans(active_view);
//   window_get_position (v->w, &oldY, &oldX);
//   window_get_size (v->w, &oldh, &oldw);
   oldh = vwptr[v->view_num]->window_core::height();
   oldw = vwptr[v->view_num]->window_core::width();
   oldY = vwptr[v->view_num]->window_core::y_pos();
   oldX = vwptr[v->view_num]->window_core::x_pos();
                                             /* zoom it */
   position (whole, whole);
   view_is_zoomed= TRUE;
   }
}

