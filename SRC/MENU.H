#ifndef MENU_H
#define MENU_H

#ifndef TOW_H
#include "tow.h"
#endif

struct menu_datapack {
   USHORT col_count, col_size;
   SHORT location;
   char *border;
   byte norm_color, high_color, border_color;
   USHORT options;
   };

extern SHORT menuchoice(
   char *list[],  /* array of choices */
   SHORT list_size, /* number of elements in the array */
   SHORT *start,    /* cursor position- input and output */
   struct menu_datapack *data);

#define MENU_USE_INITIALS 4096
#define MENU_MULTI_UP     8192
#define MENU_ITEMHELP    16384
#define MENU_PULLDOWN    32768

#define MENU_DISMISS     255    // return from func, dismiss the menus
#define MENU_APP_EXIT   1024    // "menu" "file" "quit" application

SHORT menu (
   char *list[],  /* array of strings to display */
   SHORT listsize,  /* number of strings */
   SHORT *start,    /* which one to start with, and remember last */
   USHORT options,
   SHORT col_width);

void menu_onoff (char *s,SHORT col, bool status);
void menu_yesno (char *s,SHORT col, bool status);

extern SHORT blockmenu();
extern SHORT filemenu();
extern SHORT printmenu();
extern SHORT aboutmenu();       
extern SHORT searchmenu();
extern SHORT edset ();

SHORT pull_menu (char *list[],SHORT listsize,SHORT *start, USHORT options,
                                                        SHORT col_width);

#endif
