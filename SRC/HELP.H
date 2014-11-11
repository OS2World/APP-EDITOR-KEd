#ifndef HELP_H
#define HELP_H

/* Help loci */
#define EDIT_HELP     1    /* editor help */
#define FILE_HELP     2    /* file menu help */
#define BLOCK_HELP    3    /* block menu help */
#define WINDOW_SIZE   4    /* window size help */
#define SETTINGS_HELP 5    /* settings menu help */
#define SEARCH_HELP   6    /* search menu help */
#define DIRLIST_HELP  7    /* dirlist/commando help */
#define ASCII_HELP    8    /* ascii box help */
#define PRINT_HELP    9    /* print menu help */
#define COMMANDO_HELP 10   /* commando help */
//#define FILELIST_HELP 11   /* filelist menu help */
#define VIEWPICK_HELP 11   /* view picklist help */
#define DIALOG_HELP   12   /* help for all prompt boxes */
#define SHELL_HELP 13      /* help for the DOS shell menu */
//#define CALCULATOR_HELP 14 /* help for the expression evaluator */
#define ABOUT_HELP 14 /* help for the expression evaluator */
#define WINDOW_MOVE     15   // window move
//#define SHELL_HELP 15      /* help for the DOS shell menu */

struct help_rec {
   SHORT locus,screen,x,y;
   };

void init_help (USHORT key);
//struct help_rec *set_help (struct help_rec *help_rec);

bool do_help (USHORT key);    /* do_help() is called from within get_key() */

#ifndef NO_HELP
 #define DEF_AUTOHELP(l,s) static struct help_rec help={l,s,-1,-1}; struct help_rec *old_help
 #define DEF_HELP(l,s,x,y) static struct help_rec help={l,s,x,y}; struct help_rec *old_help
 #define NEW_HELP old_help=set_help(&help)
 #define OLD_HELP set_help(old_help)
 struct help_rec *set_help (struct help_rec *help_rec);
#else
 #define DEF_HELP(l,s,x,y) /* Nothing */
 #define DEF_AUTOHELP(l,s) /* Nothing */
 #define NEW_HELP /* Nothing */
 #define OLD_HELP /* Nothing */

// void set_help(void);
 inline struct help_rec *set_help (struct help_rec *help_rec)
  {
   struct help_rec *help; //dummy
   return help;
  }
#endif

#endif  //def HELP_H
