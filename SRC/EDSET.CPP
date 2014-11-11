/*****************************************************
File: EDSET.C       copyright 1989 by Dlugosz Software
   this lets the user edit the settings structure
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "getkey.h"
#include "keycodes.h"
#include "menu.h"
#include "setting.h"
#include "edset.h"
#include "sheet.h"
#include "view.h"
#include "misc.h"
#include "help.h"
#include "redbox.h"
#include "askfile.h"
#include "mylib.h"
#include "status.h"
#include "misc.h"
#include "edx.h"
#include "dirlist.h"   /* need cwd */
#include "ascii.h"
#include <string.h>

#include "basewin.h"
static basewin edcolorw (18,51,4,3);

//SHORT which_view;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef WORD_WRAP
static char *default_menu[]= {
   "Insert mode    xxx", "Auto-indent    xxx", "Use tabs       xxx",
   "Tab size       xxx", "Expand tabs    xxx", "Compress tabs  xxx",
   "Write EOF      xxx", "Backup files   xxx", "Safe Save      xxx", 
   "Def extension  xxx", "Printer file"      , "Printer Setup Str" , 
   "Old PageUp/Dn  xxx", "Numlock ON ?   xxx", "Quote Prefix   xxx", 
   "Use WordWrap ? xxx", "RightMargin    xxx"
   };

static char *view_menu[]= {
   "Insert mode    xxx", "Auto-indent    xxx", "Use tabs       xxx",
   "Tab size       xxx", "Use WordWrap ? xxx", "RightMargin    xxx"
   };
#endif
#ifndef WORD_WRAP
static char *default_menu[]= {
   "Insert mode    xxx", "Auto-indent    xxx", "Use tabs       xxx",
   "Tab size       xxx", "Expand tabs    xxx", "Compress tabs  xxx",
   "Write EOF      xxx", "Backup files   xxx", "Safe Save      xxx", 
   "Def extension  xxx", "Printer file"      , "Printer Setup Str" , 
   "Old PageUp/Dn  xxx", "Numlock ON ?   xxx", "Quote Prefix   xx "
   };

static char *view_menu[]= {
   "Insert mode    xxx", "Auto-indent    xxx", "Use tabs       xxx",
   "Tab size       xxx"
   };
#endif


static void  change_curview_defaults (void)
{
struct view_rec *v = (struct view_rec *)Mem::vmem_trans (active_view);
static SHORT lastchoice= 0;
SHORT choice;
DEF_HELP (SETTINGS_HELP,101,10,10);

NEW_HELP;
for (;;) {
   menu_onoff (view_menu[0],15,v->flags&1);  /* insert */
   menu_onoff (view_menu[1],15,v->flags&2);  /* auto-indent */
   menu_onoff (view_menu[2],15,v->flags&8);  /* use tabs */
   num2str    (view_menu[3]+15,v->tab_size,3,' ');
#ifdef WORD_WRAP
   menu_onoff (view_menu[4],15,v->flags&32);  /* use word wrap */
   num2str    (view_menu[5]+13,v->RightMargin,5,' ');
   choice= menu(view_menu,6,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|6,18);
#else
   choice= menu(view_menu,4,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|6,18);
#endif
   switch (choice) { 
      case 0: /* insert mode */
         v->flags^= 1;
//         status_insertmode (v->flags&1);
         status_redraw();
         break;
      case 1: /* auto-indent */
         v->flags^= 2;
//         status_indentmode (v->flags&2);
         status_redraw();
         break;
      case 2: /* use tabs */
         v->flags^= 8;
         break;
      case 3: /* tab size */
         prompt_int ("Tab size:",&v->tab_size,0,255,8);
         break;
#ifdef WORD_WRAP
      case 4:     // use word_wrap ?
         v->flags^= 32;
         break;
      case 5:     // rightmargin
         prompt_int ("Right Margin for word wrap:",(SHORT *)&v->RightMargin,0,255,8);
#endif
         break;
      case -1:
         OLD_HELP;
         return;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  change_editor_defaults (void)
{
static SHORT lastchoice= 0;
DEF_HELP (SETTINGS_HELP,81, 10,10);

NEW_HELP;
for (;;) {
   menu_onoff (default_menu[0],15,settings.viewer_defaults.flags&1);  /* insert */
   menu_onoff (default_menu[1],15,settings.viewer_defaults.flags&2);  /* auto-indent */
   menu_onoff (default_menu[2],15,settings.viewer_defaults.flags&8);  /* use tabs */
   num2str (default_menu[3]+15,settings.viewer_defaults.tab_size,3,' ');
   menu_onoff (default_menu[4],15,settings.flags&4);  // expand tabs 
   menu_onoff (default_menu[5],15,settings.flags&8);  // compress tabs 
   menu_onoff (default_menu[6],15,settings.write_eof);   /* write eof */
   menu_onoff (default_menu[7],15,settings.flags&1); /* backup files */
   menu_onoff (default_menu[8],15,settings.flags&2); /* safe save */
   strcpy (default_menu[9]+15,settings.default_ext);
   menu_onoff (default_menu[12],15,settings.viewer_defaults.view_page_mode); /* PageUp/Dn */
   menu_onoff (default_menu[13],15,settings.numlock_on); /* Numlock auto on */
   strcpy (default_menu[14]+15,settings.viewer_defaults.QuotePrefix);
#ifdef WORD_WRAP
   menu_onoff (default_menu[15],15,settings.viewer_defaults.flags&32);  /* use word wrap */
   num2str (default_menu[16]+13,settings.viewer_defaults.RightMargin,5,' ');
   switch (menu(default_menu,17,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|MENU_LOCATION,18)) {
#else
   switch (menu(default_menu,15,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|MENU_LOCATION,18)) {
#endif
      case 0: /* insert mode */
         settings.viewer_defaults.flags^= 1;
         break;
      case 1: /* auto-indent */
         settings.viewer_defaults.flags^= 2;
         break;
      case 2: /* use tabs */
         settings.viewer_defaults.flags^= 8;
         break;
      case 3: /* tab size */
         prompt_int ("Tab size:",&settings.viewer_defaults.tab_size,0,255,8);
         break;
      case 4: /* expand tabs */
         settings.flags^= 4;
         break;
      case 5: /* compress spaces to tabs */
         settings.flags^= 8;
         break;
      case 6: /* write eof */
         //file_write_eof= !file_write_eof;
         settings.write_eof= !settings.write_eof;
         break;
      case 7: /* backup files? */
         settings.flags^= 1;
         break;
      case 8: /* safe save? */
         settings.flags^=2;
         break;
      case 9: /* default extension */
         prompt_box ("Default file extension:",settings.default_ext,3,NULL,1|64,12);
         break;
      case 10: /* printer */
         prompt_box ("Printer file:",settings.printer,79,NULL,1,13);
         break;
      case 11: /* printer setup string */
         prompt_box ("Printer Setup String:",(char *)settings.printer_setup_string,39,NULL,0,13);
         break;
      case 12:             /* new or old PageUp/Dn function (for John D!)  */
         settings.viewer_defaults.view_page_mode = !settings.viewer_defaults.view_page_mode;
         break;
      case 13:
         settings.numlock_on = !settings.numlock_on;
         numlock_set(settings.numlock_on);
         DBG_INI(dout<<"setting numlock_on now = "<<settings.numlock_on<<endl);
         break;
      case 14:
         prompt_box ("Block Quote Prefix:",settings.viewer_defaults.QuotePrefix,3,NULL,64,13);
         break;
#ifdef WORD_WRAP
      case 15: /* use word wrap */
         settings.viewer_defaults.flags^= 32;
         break;
      case 16: /* Right Margin setting */
         prompt_int ("Right Margin for word wrap:",&settings.viewer_defaults.RightMargin,0,255,8);
         break;
#endif
      case -1:
         OLD_HELP;
         return;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  change_directories (void)
{
static char *choices[]= {"Backup file spec", "Temporary file spec", "Help path"};
static SHORT lastchoice;
DEF_HELP (SETTINGS_HELP,121, 10,10);

NEW_HELP;
for (;;) {
   switch (menu(choices,3,&lastchoice,MENU_USE_INITIALS|MENU_ITEMHELP|MENU_LOCATION,19)) {
      case 0: /* backup path */
         prompt_box ("Backup file spec:",settings.backup_path,79,NULL,1,9);
         break;
      case 1: /* temporary file path */
         prompt_box ("Temporary file spec:",settings.tempfile_path,79,NULL,1,10);
         break;
      case 2: /* help file */
         prompt_box ("Help file name:",settings.help_path,79,NULL,1,11);
         break;
      case -1:
         OLD_HELP;
         return;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool edit_color (byte* c)
{
SHORT y,x,selected= FALSE,digit= 0;
byte attrib= 0;
char s[2];
static char hex_char[]= "0123456789ABCDEF";
#define pos(n) settings.location[COLORBOX_LOCATION].n
DEF_HELP (SETTINGS_HELP,60,10,10);

NEW_HELP;
edcolorw.clearwin((settings.colors[MENU_COLORS][NORMAL_COLOR]),_BLACK, ' ');
//edcolorw.clearwin(WHITE, _BLACK, ' ');
edcolorw.border(window_core::Double, attr(WHITE,_BLUE));
s[2] = 0x00;

for (y= 1; y <= 16; y++)
   for (x= 2; x <= 47; x+=3) {
      s[0]= hex_char[y-1];
      s[1]= hex_char[(x-2)/3];
//      put_string_len (w, y,x, s, 2);
//      highlight (w, y, x, attrib++, 2);
      edcolorw.put(y,x, s,2);
      edcolorw.highlight(y, x, attrib++, 2);
      }
edcolorw.show();

if (c) {
   SHORT xp,yp,key;

   x= *c & 15; y= *c >> 4;
   for (;;) {
      xp= x*3+1;
      yp= y+1;
//      put_string (w, yp,xp,"\x10");
//      put_string (w, yp,xp+3,"\x11");
//      highlight (w, yp,xp,(byte)((digit == 0) ? settings.colors[MENU_COLORS][NORMAL_COLOR] | 128 : settings.colors[MENU_COLORS][NORMAL_COLOR] & 127),1);
//      highlight (w, yp,xp+3,(byte)((digit == 1) ? settings.colors[MENU_COLORS][NORMAL_COLOR] | 128 : settings.colors[MENU_COLORS][NORMAL_COLOR] & 127),1);
      edcolorw.put(yp,xp,"\x10");
      edcolorw.put(yp,xp+3,"\x11");
      edcolorw.highlight (yp,xp,(byte)((digit == 0) ? settings.colors[MENU_COLORS][NORMAL_COLOR] | 128 : settings.colors[MENU_COLORS][NORMAL_COLOR] & 127),1);
      edcolorw.highlight (yp,xp+3,(byte)((digit == 1) ? settings.colors[MENU_COLORS][NORMAL_COLOR] | 128 : settings.colors[MENU_COLORS][NORMAL_COLOR] & 127),1);

      key= getkey ();
//      put_string (w,yp,xp," ");
//      put_string (w,yp,xp+3," ");
      edcolorw.put(yp,xp," ");
      edcolorw.put(yp,xp+3," ");
      switch (key) {
         case kUp:
            if (--y < 0) y= 15;
            break;
         case kDown:
            if (++y > 15) y= 0;
            break;
         case kLeft:
            if (--x < 0) x= 15;
            break;
         case kRight:
            if (++x > 15) x= 0;
            break;
         case kHOME:
            x= 0; break;
         case kEND:
            x= 15; break;
         case PageUp:
            y= 0; break;
         case PageDown:
            y= 15; break;
         case GREY_Enter:
         case Enter:
          //  *c= y << 4;                // org
            *c = (byte)(y << 4);         //       mod 2-92
            *c |= x;
            selected= TRUE;                         /* falls through */
         case kESC:
            goto exit;
         default:
            if (key >= '0' && key <= '9') key-= 48;
            else if (key >= 'a' && key <= 'f') key-= 87;
            else if (key >= 'A' && key <= 'F') key-= 55;
            else break;
            if (++digit == 1)
               y= key;
            else {
               x= key;
               digit= 0;
               }
         } /* switch */
      } /* while */
   } /* if */
else getkey ();
exit:
//destroy_window (w);
edcolorw.hide();
OLD_HELP;
return selected;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void edit_colorset (byte c[])
{
static char *choices[]= {"A:normal text","B:normal cursor","C:block text",
   "D:block cursor","E:border","F:other","Restore"};
static SHORT lastchoice= 0;
byte oldcolors[6];
SHORT i;
DEF_HELP (SETTINGS_HELP,40,10,10);

NEW_HELP;
basewin edsetw (9,17,settings.location[MENU_LOCATION].y,
                       settings.location[MENU_LOCATION].x+18);

memcpy (oldcolors, c, sizeof oldcolors);  /* save original */
for (;;) {
//   clear_window (w,0,8,0,16, (c[NORMAL_COLOR] << 8) | ' ');
//   edsetw.clearwin(c[BORDER_COLOR], ' ');
   edsetw.clearwin(c[NORMAL_COLOR], ' ');
//   window_border (w, fat_border, c[BORDER_COLOR]);
   edsetw.border(window_core::Double,c[BORDER_COLOR]);
   for (i= 0; i < 4; i++)
//      put_string (w,2+i,2,choices[i]+2);
//   put_string (w,6,2,choices[5]+2);
      edsetw.put(2+i,2,choices[i]+2);
   edsetw.put(6,2,choices[5]+2);

//   highlight (w,2,2,c[NORMAL_COLOR],13);
//   highlight (w,3,2,c[HIGHLIGHT_COLOR],13);
//   highlight (w,4,2,c[BLOCK_COLOR],13);
//   highlight (w,5,2,c[BLOCK_CURSOR_COLOR],13);
//   highlight (w,6,2,c[OTHER_COLOR],13);
   edsetw.highlight (2,2,c[NORMAL_COLOR],13);
   edsetw.highlight (3,2,c[HIGHLIGHT_COLOR],13);
   edsetw.highlight (4,2,c[BLOCK_COLOR],13);
   edsetw.highlight (5,2,c[BLOCK_CURSOR_COLOR],13);
   edsetw.highlight (6,2,c[SEARCH_COLOR],13);

//   display_on (w);
   edsetw.show();
   switch (i=menu(choices,7,&lastchoice, MENU_USE_INITIALS|MENU_LOCATION, 15)) {
      case 6: /* restore */
         memcpy (c, oldcolors, sizeof oldcolors);
         break;
      case -1: 
//         destroy_window (w);
         OLD_HELP;
         return; /* done editing */
      default:  /* edit one of them */
         if (i != BORDER_COLOR) 
//             put_string (w,2+i-(i==OTHER_COLOR),1,"\x10");
             edsetw.put(2+i-(i==SEARCH_COLOR),1,"\x10");
         edit_color (c+i);
         if (i != BORDER_COLOR) 
//             put_string (w,2+i-(i==OTHER_COLOR),1," ");
             edsetw.put(2+i-(i==SEARCH_COLOR),1," ");
         break;
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static SHORT  get_defaultnum (void)
{
/* allows user to select a default colorset number and returns it */
static SHORT lastchoice= 0;
static char *viewset_str[]= {"1:Color set 1","2:Color set 2","3:Color set 3",
                      "4:Color set 4","5:Color set 5"};

return menu(viewset_str,MAX_VIEW_COLORSETS,&lastchoice,
       MENU_USE_INITIALS|MENU_LOCATION,13);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  select_colorset (void)
{
static char *choices[]= {"This view","Dialog","Filelist","Help","Menu",
                         "Redbox","Status line","ASCII box",
                         "View defaults","Get default"};
static SHORT setnum[]= {DIALOG_COLORS,DIRLIST_COLORS,HELP_COLORS,MENU_COLORS,
                      REDBOX_COLORS,STATUS_COLORS,ASCIIBOX_COLORS };
static SHORT lastchoice= 0;
struct view_rec *v;
SHORT i;
DEF_HELP (SETTINGS_HELP,21,10,10);

NEW_HELP;
for (;;)
   switch (i= menu(choices,9,&lastchoice,
                   MENU_USE_INITIALS|MENU_ITEMHELP|MENU_LOCATION, 13)) {
      case 0:                                              /* current view */
         v= (struct view_rec *)Mem::vmem_trans (active_view);
         edit_colorset (v->colors);
         memcpy (settings.colors[v->number], v->colors, sizeof v->colors);
         edcolorw.border(window_core::Double);
//         window_border (v->w, double_border, v->colors[BORDER_COLOR]);
//         view_redraw_block (active_view);
         shade_block (active_view, TRUE,0,0);
         break;
      case 9:                                            /* view defaults */
         i= get_defaultnum ();
         if (i >= 0) edit_colorset (settings.colors[VIEW_DEF_COLORS+i]);
         break;
      case 10:
         i= get_defaultnum ();
         if (i >= 0) {
            v= (struct view_rec *)Mem::vmem_trans (active_view);
            memcpy (v->colors,settings.colors[VIEW_DEF_COLORS+i],sizeof v->colors);
//            window_border (v->w, double_border, v->colors[BORDER_COLOR]);
       //     view_redraw_block (active_view);
         edcolorw.border(window_core::Double);
            shade_block (active_view, TRUE,0,0);
            }
         break;
      case -1: 
         OLD_HELP;
         return;                                         /* done editing */
      default:
         edit_colorset (settings.colors[setnum [i-1]]);
         if (setnum[i-1] == STATUS_COLORS) status_redraw ();
      }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT edset ()
{
static char *choices[]= {"Load","Save","Colors","Editor","View","Path"};
static SHORT lastchoice= 2;
static char fname[FNAME_LEN];
DEF_HELP (SETTINGS_HELP,2,10,10);
SHORT retval = 0;

NEW_HELP;
for (;;)
   switch (menu(choices,6,&lastchoice,
                MENU_USE_INITIALS|MENU_ITEMHELP|OPTION_LOCATION,6)) {
      case 0:                                            /* load settings */
         if (get_load_name ("ked.ini", fname, cwd, "SET")) {
            if (!setting_load (fname))
               redbox (11,1,eh_continue);
            }
         OLD_HELP;
         return retval;
      case 1:                                       /* save settings */
         strcpy (fname, setting_filename);
         if (get_save_name (fname, cwd, "SET"))
            if (!setting_save (fname))
               redbox_s (11,200,eh_abort,fname);
            else 
               strcpy (setting_filename, fname);  /* remember the name */
         OLD_HELP;
         return retval;
      case 2:                                       /* colors */
         select_colorset ();
         break;
      case 3:                                       /* editor defaults */
         change_editor_defaults ();
         break;
      case 4:                                  /* current editor defaults */
         change_curview_defaults ();
         break;
      case 5:                                       /* directory paths */
         change_directories ();
         break;
      case -1:
         OLD_HELP;
         return retval;
      }
return retval;
}
