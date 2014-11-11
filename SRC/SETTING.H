/* setting.h */

#define MENU_LOCATION       1                /* location assignments */
#define COMMANDO_LOCATION   2
#define ASCIIBOX_LOCATION   3
#define COLORBOX_LOCATION   4
#define DIALOG_LOCATION     5
#define PICKBOX_LOCATION    6
#define CALCULATOR_LOCATION 7
#define FILE_LOCATION       8             /*  pulldown menu locations */
#define EDIT_LOCATION       9             /* block menu */
#define VIEW_LOCATION      10
#define SEARCH_LOCATION    11
#define OPTION_LOCATION    12
#define HELP_LOCATION      13
#define PRINT_LOCATION     14
#define LOCATION_COUNT     16       /* add 8 locations for pulldown menus */

#define DIRLIST_COLORS     0        /* color set assignments */
#define MENU_COLORS        1
#define REDBOX_COLORS      2
#define STATUS_COLORS      3
#define DIALOG_COLORS      4
#define HELP_COLORS        5
#define ASCIIBOX_COLORS    6
#define CALCULATOR_COLORS  7
#define VIEW_DEF_COLORS    8        /* 8 through 11 inclusive */
#define MAX_VIEW_COLORSETS 4        /* 4 colorsets allocated for views */

#define COLORSET_COUNT 12           /* total number of colorsets */

#define NORMAL_COLOR       0         /* indexes into color set */
#define HIGHLIGHT_COLOR    1
#define BLOCK_COLOR        2
#define BLOCK_CURSOR_COLOR 3
#define BORDER_COLOR       4
#define SEARCH_COLOR       5         // highlight "found" in search

struct setting_info {
   struct { SHORT x,y; } location[LOCATION_COUNT];
   byte colors[COLORSET_COUNT][6];  /* 12 sets of 6 colors.  */
   struct {                        /* viewer descript, used by new_view() */
      SHORT toplines, bottomlines;   /* lines to reserve not showing text */
      USHORT flags;              /* default flags */
                                /* flags: 1-insert on         2-indent on
                                          4-needs update      8-tabs    
                                          16-save tabs        32-word_wrap */
      SHORT vert_scroll_margin, horz_scroll_margin;
      bool view_page_mode;         /* diff  pagedn/pageup in view_com.c */                                 
      SHORT block_mode;                               /* enum block_mode_t */
      SHORT color;                    /* index of one of the colors, above */
      SHORT tab_size;
      SHORT RightMargin;
      char QuotePrefix[4];
      }  viewer_defaults;
   char backup_path[FNAME_LEN];
   char tempfile_path[FNAME_LEN];
   char help_path[FNAME_LEN];
          // flags:  1=autobackup  2=safe save  4=expand tabs on file_load
          //         8=compress tabs on file save
   USHORT flags;
                          /* 1-autobackup  2-safe save  4-save tabs */
   bool write_eof;
   bool numlock_on;
   struct {
      struct {SHORT h,w,x,y;} position[26];
      }  zoom_defaults;
   struct {
      SHORT flags, direction;
      }  search_defaults;
   char default_ext[4];
   struct {
      SHORT sort_type;
      SHORT direction;            /* sort direction */
      USHORT attr_flags;          /* dirlist file search attributes */
      }  dirlist_defaults;
   unsigned char printer_setup_string[40];
   char printer[FNAME_LEN];       /* filename for printer */
   struct {
      char s[80];                 /* command name to run */
      SHORT flags;                /* 1-dosscreen 2-logfile 4-load logfile
                                     8-save file  16-save all files */
      } shell_command;
   char user_name[41];
   };

extern struct setting_info settings;
extern char setting_filename[FNAME_LEN];

bool setting_load (char *name);
bool setting_save (char *name);
void setting_defaultcolors (SHORT n);

#ifdef PACKUP
SHORT setting_packup (byte * buf);
SHORT setting_unpack (byte * buf);
#endif
