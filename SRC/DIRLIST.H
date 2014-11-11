#ifndef DIRLIST_H
#define DIRLIST_H

extern char cwd[FNAME_LEN];
extern char progname[FNAME_LEN];
extern char exe_dir[FNAME_LEN];
  

/* filename manipulation */
char *find_basename (char const *filename);
char *find_extension (char *filename);
void replace_basename (char *filename, char const* newname);
char *find_basesubdir (char *filename);
bool filename_iswild (char const* filename);
void normalize_filename (char *filename, char const* default_path, char const* default_extension);
void add_subdir (char *filename, char const* newnode);
void delta_filename (char *buf, char const* original, char const* delta);
bool filename_samedir (char const* name1, char const* name2);
bool get_directory (SHORT drive_letter, char *s);
bool get_current_directory (char *s);
bool dot_dir (char const* s);
bool dotdot_dir (char const* s);
                                           // OS/2 Long File Names
void normalize_filename_2 (char *filename, char const* default_path,
                         char const* default_extension);


/* DOS COMMANDO */
enum dirlist_flags {SHOWFILES=1,SHOWDIRS=2,SHOWATTR=4,SHOWDATE=8,SHOWSIZE=16,
                    SHOWALL=1+2+4+8+16,COMMANDO=32,PATHWINDOW=64,SHOWFREE= 128};

bool dirlist (char const *title, char *filename, enum dirlist_flags flags, SHORT y, SHORT x, SHORT height);
/* dirlist_core() does not set up a help window as dirlist does */
bool dirlist_core (char const *title, char *filename, enum dirlist_flags flags, SHORT y, SHORT x, SHORT height);
void commando (char *path);
//window_t make_pathwind (SHORT y, SHORT x, SHORT height, SHORT width, const char *border, SHORT py, SHORT px, SHORT pw);
//void set_pathwind (window_t w, SHORT py, SHORT px, SHORT pw);

/* file searching */

struct dir_entry {
//   char name[13];
   char name[FNAME_LEN];        // mod 2-7-93 support HPFS
   byte flags;
   bool mark;
   unsigned long size;
   USHORT date;
   USHORT time;
   };

struct dirlist_data {
//   window_t w;  /* the window the display is in */
   SHORT width;   /* window's inner width */
   SHORT rows;    /* how many entries can be displayed */
//   vptr filelist;  /* all the directory entries for this dir */
   dir_entry* filelist;  // all found files for this dir
   SHORT filelist_size;  /* how many entries there are */
   SHORT first_entry;    /* which one is at the top of the window */
   SHORT current_entry;  /* which one is the cursor over */
   char name[FNAME_LEN];
   byte color;         /* used in clearing the window */
   byte altcolor;      /* used to display a cursor */
//   USHORT flags;     /* display flags (see enum type above) */
   unsigned int flags;     /* display flags (see enum type above) */
   SHORT tagcount;       /* # of files currently tagged for operations */
   SHORT startrow;       /* window row where list starts */
   };

struct dos_buffer_type {
   byte reserved[21];
   byte attribute;  /* better make sure the compiler lets you align this */
   USHORT time;
   USHORT date;
   unsigned long size;
//   char name[13];
   char name[FNAME_LEN];
   };


enum file_attribs { FA_RD_ONLY= 1, FA_HIDDEN= 2, FA_SYSTEM= 4, FA_ARCHIVED= 32,
   FA_ATTRIB_BITS= 1+2+4+32 };

bool file_findfirst (char const* name, struct dir_entry *e, USHORT dirflags);
bool file_findnext (struct dir_entry *e);
void display_entry (struct dirlist_data *d, struct dir_entry *e, SHORT row);
void redisplay_dirlist (struct dirlist_data *d);
void display_path (char const *filename,SHORT y,SHORT x,SHORT width);
void read_list (struct dirlist_data *d, char *filename);
void show_bytesfree (struct dirlist_data *d);
void select_attr_flags (USHORT *flags);
void update_entry (struct dirlist_data *d, SHORT index);

void sort_dirlist (struct dirlist_data *d);
                           /* in MYLIB.C              */

//extern SHORT assembly_findfirst (struct dos_buffer_type * DTA, char const * name,USHORT dirflags);
//extern SHORT assembly_findnext (struct dos_buffer_type * DTA);

//#define NO_COMMANDO 1        /* comment this out to include COMMANDO  */

#endif
