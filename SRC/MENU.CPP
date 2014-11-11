/*****************************************************
file MENU.C         Copyright 1989 by Dlugosz Software

*****************************************************/
#define INCL_SUB
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "getkey.h"
#include "keycodes.h"
#include "menu.h"
#include "setting.h"
#include "help.h"
#include <string.h>         /* need strcpy() */
#include "mymenu.h"         /* need my custom menus  */

#include "basewin.h"

#include "test.h"

static SHORT pull_menuchoice (char *list[],SHORT list_size,SHORT *start,
                      struct menu_datapack *data);

static basewin *topw;
static basewin *subw;

#define NEW_MENU 1

static bool submenu = FALSE;
static SHORT direction = 0;
#define LEFT  -2;
#define RIGHT -3;
#define pos(n) settings.location[data->location].n

/***********************************************************************
     Scan list of menu items for "char key", return TRUE if found,
     otherwise return FALSE. Used for single key menu selection
*************************************************************************/
//static bool scan (char *list[],SHORT *len, char key, SHORT *num)
static bool scan (char *list[],SHORT *len, char key)
{                    // num = position in list[]
SHORT item= 0;
while (item <= *len && **list != key) {
   item++;
   list++; }
if (item <= *len) {
   *len= item;
//   *num = item;
   return TRUE; 
}
return FALSE;
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void menu_onoff (char *s,SHORT col, bool status)
{                   /* copy ON or OFF into a menu item string */
static char *status_str[]= {"OFF","ON"};
strcpy (s+col, status_str[status != 0]);
}

/**************************************************************/

void menu_yesno (char *s,SHORT col, bool status)
{                              /* copy YES or NO into a menu item string */

static char *status_str[]= {"NO ","YES"};
strcpy (s+col, status_str[status != 0]);
}

/*********************************************************************
   this is a generic menu driver
   returns 1 for 1st choice, etc
***********************************************************************/
SHORT menuchoice (char *list[],SHORT list_size,SHORT *start,
                struct menu_datapack *data)
{
SHORT x_size, rows, choice;
USHORT key;
UCHAR lokey;
SHORT x,y;
SHORT const columns= data->col_count -1;
//static vptr w;
bool done= FALSE;
   SHORT loop, row=1,col=1;

#ifndef NO_HELP
struct help_rec *oldhelp, help;
if (data->options & MENU_ITEMHELP) {
   oldhelp= set_help (NULL);  /* get help record */
   help= *oldhelp;
   }
#endif

x_size= (columns+1)*data->col_size+2;
rows= (list_size+columns)/(columns+1);
x= *start/rows;
y= *start%rows;
//w= create_window (rows+2,x_size,pos(y),pos(x));
//clear_window (w,0,rows+1,0,x_size-1, (data->norm_color<<8)|' ');
//window_border (w, data->border, data->border_color);
subw = new basewin(rows+2,x_size,pos(y),pos(x));
subw->clearwin((data->norm_color),' ');
subw->border(window_core::Single, data->border_color);

{  
//   SHORT loop, row=1,col=1;
   char **s= list;
   for (loop=0; loop < list_size; loop++) {
//      put_string (w,row,col,*s);
      subw->put(row,col,*s);
      if (++row > rows) {
         row=1;
         col += data->col_size; }
      s++;
      }
   }
//display_on (w);
subw->show();
while (!done) {
//   highlight (w,y+1,x*data->col_size+1,data->high_color,data->col_size);
   subw->highlight(y+1,x*data->col_size+1,data->high_color,data->col_size);
#ifndef NO_HELP
   if (data->options & MENU_ITEMHELP) {
      help.screen= oldhelp->screen + x*rows+y;
      set_help (&help);
      }
#endif
   key= getkey();
   lokey= LOUCHAR(key);
//   highlight (w,y+1,x*data->col_size+1,data->norm_color,data->col_size);
   subw->highlight(y+1,x*data->col_size+1,data->norm_color,data->col_size);
   if ((lokey >= 'a') && (lokey <= 'z')) 
       lokey &=~32;                   /* bump to upper case */
   switch (key) {
   case kESC:
      choice= -1;
      done= TRUE;
      submenu = FALSE;
      direction = 0;
      break;
   case GREY_Enter:
   case Enter:
      choice= x*rows+y;
      done= TRUE;
      break;
   case kUpe:
   case kUp:
      if (rows == 1) {
         if (--x < 0) x= list_size-1;
         }
      else {                     /* general case */
         if (--y < 0) {          /* wrap to bottom of column to left */
            y= rows-1;
            if (--x < 0) {
               x= columns;       /* wrap around to right side */
               while (x*rows+y >= list_size) y--;
               }
            }
         }
      break;
   case kDowne:
   case kDown:
      if (++y > rows-1 || x*rows+y >= list_size) {
                                      /* wrap to top of next column */
         if (++x > columns) x= 0;       /* wrap to left size */
         y= 0;
         }
      break;
   case kRighte:
   case kRight:
      if(submenu == TRUE) {
          direction = RIGHT;
          choice = -1;
          done = TRUE;
          break;
       }
      if (++x > columns || x*rows+y >= list_size) {
         x = 0;                        /* wrap around to next row */
         if (++y > rows-1) y= 0;      /* wrap to bottom */
         }     
      break;
   case kLefte:
   case kLeft:
      if(submenu == TRUE) {
          direction = LEFT;
          choice = -1;
          done = TRUE;
          break;
       }
      if (--x < 0) {
         x= columns;
         if (--y < 0) {
            y= rows-1;             /* wrap to top */
            while (x*rows+y >= list_size) 
               x--;
            }
         }
      break;
   case kHOMEe:
   case kHOME:
      x= 0;                        /* falls through */
   case PageUpe:
   case PageUp:
      y= 0;
      break;
   case kENDe:
   case kEND:
      x= columns;
      while (x*rows+y >= list_size) /* falls through */
         x--;
   case PageDowne:
   case PageDown:
      y= rows-1;
      while (x*rows+y >= list_size) y--;
      break;
   default:
      if (data->options & MENU_USE_INITIALS) {
         SHORT q= list_size-1;
//         if (lokey <= 255 && scan (list,&q, lokey)) {
         if (scan (list,&q, lokey)) {
            choice= q;
            done= TRUE; }
         }
      } /* switch */
   } /* while */
*start= x*rows+y;
//destroy_window (w);
delete subw;

#ifndef NO_HELP
if (data->options & MENU_ITEMHELP) 
   set_help (oldhelp);
#endif

return choice;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT menu (char *list[],SHORT listsize,SHORT *start,
          USHORT options,SHORT col_width)
{
static struct menu_datapack data = { 1,0,0,window_core::Single,0,0,0 };
//                                   ^
//                                 col_count = number of columns
data.col_size= col_width;
data.norm_color= settings.colors[MENU_COLORS][NORMAL_COLOR];
data.high_color= settings.colors[MENU_COLORS][HIGHLIGHT_COLOR];
data.border_color= settings.colors[MENU_COLORS][BORDER_COLOR];
data.options= options;
if (options & MENU_MULTI_UP) {
    data.col_count = listsize;              // how many columns
}
else 
   data.col_count= 1;

data.location= (SHORT)(options & 255);               // clears high bit ??
return menuchoice (list,listsize, start, &data);
}

/* *********************************************************************
      The following is a hack to get pulldown menus working while still
          using as much as possible of the DLUGOSZ toolbox API

           1-25-92

      mods:  returns -2 for left arrow
                     -3 for right arrow

      submenus are hardcoded here
**************************************************************************/

static SHORT pull_menuchoice (char *list[],SHORT list_size,SHORT *start,
                      struct menu_datapack *data)
{
SHORT x_size, rows, choice = 0;
USHORT key = 0x0000;
UCHAR lokey;
SHORT x,y;
SHORT const columns= data->col_count -1;
//vptr pw;
bool done= FALSE;
SHORT loop, row=1,col=2;
SHORT newchoice = 0;

x_size= ((columns+1)*data->col_size)+3;
rows = 1;
x= *start/rows;
y= *start%rows;
topw = new basewin(rows+2,x_size,pos(y),pos(x));
topw->clearwin((data->norm_color),' ');
topw->border(window_core::Single, data->border_color);

{ // SHORT loop, row=1,col=1;                        // this brace is WIERD !
   char **s= list;
   for (loop=0; loop < list_size; loop++) {
      topw->put(row,col,*s);
      col += data->col_size; 
      s++;
      }             // end for
   }                // end braces
topw->show();
while (!done) {
//   key = 0;
   topw->highlight(y+1,x*data->col_size+1,data->high_color,data->col_size);
   if (submenu == TRUE) {
     if (direction != 0) {
       direction = 0;
       key = Enter;
     }
     else {                // direction == 0
       if(newchoice != MENU_DISMISS)         // mod 6-18-95
          key = Enter;
     }
   }
   else {
      key= getkey();
      lokey = LOUCHAR(key);
      if ((lokey >= 'a') && (lokey <= 'z')) 
         lokey &=~32;                   /* bump to upper case */
   }
   topw->highlight(y+1,x*data->col_size+1,data->norm_color,data->col_size);

switch (key) {
   case kESC:
      if ( submenu == FALSE) {
         choice = 0;
      }
      else {
         choice= -1;
      }
      done= TRUE;
      submenu = FALSE;
      break;
   case GREY_Enter:
   case Enter:
      submenu = TRUE;
      choice= x*rows+y;
      switch (choice) {
   //      case  0: if (filemenu() ==1) {
         case  0: newchoice = filemenu();
                  if (newchoice == MENU_APP_EXIT) {
                     submenu = FALSE;
                     done = TRUE;
                     choice = -1;
                  }
                  break;
          case 1: newchoice = blockmenu();
                  break;
#ifndef NO_SEARCH
          case 2: searchmenu();
                  break;
#endif
          case 3: newchoice = printmenu();
                  break;
          case 4: aboutmenu();
                  break;
#ifndef NO_SETTING
          case 5: edset();
                  break;
#endif
          default:
                break;
//      if(newchoice == MENU_DISMISS)
//         done = TRUE;
      }                          // end case 13
      if(newchoice == MENU_DISMISS) {
         done = TRUE;
         submenu = FALSE;
         break;
      }
      if(submenu == TRUE) {
         done = FALSE;
         if (direction == -2) goto left;
         if (direction == -3) goto right;
      }
      else
         done= TRUE;
      break;
right:
   case kRighte:
   case kRight:
      if (++x > columns ) {
         x= 0;
        }
      break;
left:
   case kLefte:
   case kLeft:
      if (--x < 0) {
         x= columns;
         }
      break;
   default:
//      break;
         if (data->options & MENU_USE_INITIALS) {
              SHORT q= list_size-1;
         if (scan (list,&q, lokey)) {
               choice= q;
               x = q;
             //  done= TRUE;
               break;
            }
         }         /* MENU_USE_INITIALS */
      }           /* switch */
   }              /* while */
*start= x*rows+y;
delete topw;
return choice;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
SHORT pull_menu (char *list[],SHORT listsize,SHORT *start,
          USHORT options,SHORT col_width)
{
/*   i could probably use the original menu() instead of this
      special function 

        1-25-92
*/
static struct menu_datapack data = { 1,0,0,window_core::Single,0,0,0 };
//                                   ^                     ^
//                                   |                     options
//                                 col_count = number of columns
//
//   do i need a function here to find the longest menu item ??
//

data.col_size= col_width;

data.norm_color= settings.colors[MENU_COLORS][NORMAL_COLOR];
data.high_color= settings.colors[MENU_COLORS][HIGHLIGHT_COLOR];
data.border_color= settings.colors[MENU_COLORS][BORDER_COLOR];
data.options= options;            
//data.col_size= sizeof(list[0]) + 3;     // set width = 1st menu item + 3
data.col_count= listsize;

data.location= (SHORT)(options & 255);              // clears high bit ??

return pull_menuchoice (list,listsize, start, &data);
}
