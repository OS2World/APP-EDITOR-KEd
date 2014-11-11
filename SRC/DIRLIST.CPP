/*****************************************************
File: DIRLIST.C     Copyright 1989 by Dlugosz Software
   this is a disk directory screen
*****************************************************/
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "dirlist.h"
#include "getkey.h"
#include "keycodes.h"
#include "string.h"
#include "misc.h"
#include "redbox.h"
                        //#include "commando.h"
#include "edx.h"
#include "menu.h"
#include "setting.h"
#include "status.h"  /* need status_note() */
#include "help.h"
#include "mylib.h"

#include "basewin.h"
#include "test.h"

basewin *dirw;
basewin *pathw;
extern basewin *prmptw;          // usually created in edx.cpp
static SHORT px,py,pwidth;       // path win x,y and window width

#define locus 3
#define colors settings.colors[DIRLIST_COLORS]
static struct dos_buffer_type dos_buffer;
static ULONG srchcount = 1;      
static HDIR  hdir      = 1;         // handle to use current dir (SYSTEM) 
static FILEFINDBUF3 findbuf;             

/***********************************************************************/
/*   DosFind* replaces functions in UTIL.ASM *************************** */
/*       fills in global dos_buffer struct,
       returns 0  FALSE for success,
               1  TRUE  for failure
*/
SHORT assembly_findfirst (struct dos_buffer_type  *dos_buf,
                          CHAR  const* name,USHORT dirflags)
{
   srchcount = 1;
   hdir = 1;    // reset this, it gets messed up on error

   rc = DosFindFirst((PSZ)name, &hdir, dirflags , (PVOID)&findbuf,
                     sizeof findbuf, &srchcount, 1L);
   if(rc == 0){
          strcpy(dos_buf->name,  findbuf.achName);
          dos_buf->attribute   = (UCHAR)findbuf.attrFile;        // ushort
          memcpy(&(dos_buf->time), &findbuf.ftimeLastWrite, 2);  // ftime
          memcpy(&(dos_buf->date), &findbuf.fdateLastWrite, 2);  // ftime
          dos_buf->size        = findbuf.cbFile;                 // ulong
     return 0;
    }
  else
    if(rc != 0) {
      if(examine_error(rc, dos_buf->name) == TRUE)
         return 0;
     }
     return 1;
}

SHORT assembly_findnext (struct dos_buffer_type  * dos_buf)
{
  static int count = 0;
   srchcount = 1;              
  rc = DosFindNext(hdir, (PVOID)&findbuf, sizeof findbuf, &srchcount);
  if(rc == 0){
          strcpy(dos_buf->name,  findbuf.achName);
          dos_buf->attribute   = (UCHAR)findbuf.attrFile;        // ushort
          memcpy(&(dos_buf->time), &findbuf.ftimeLastWrite, 2);  // ftime
          memcpy(&(dos_buf->date), &findbuf.fdateLastWrite, 2);  // ftime
          dos_buf->size        = findbuf.cbFile;                 // ulong
          count++;
//          DBG_DIR(dout<<" mylib findnext count = "<<count<<" name = "<<(findbuf.achName)<<endl);
     return 0;
  }
  else {
   if(rc == 18) 
      return TRUE;           // err no_more_files
   if(rc != 0) {
       DosBufReset(0xFFFF);           // reset internal file buffers....
       return 1;                      // err
    }
  }
  return 1;
}

/**********************************************************************
          find number of files in named dir
          used to alloc an array of num_files size in dirlist_core

*********************************************************************/
USHORT find_num_files(byte const* name)
{
   srchcount = 1;
   hdir = 1;    // reset this, it gets messed up on error
   USHORT file_count = 0;
//   USHORT dirflags = 55;    // should get all.... 1+2+4+16+32   decimal
   USHORT dirflags = 0x37;    // should get all.... 1+2+4+10h+20h

   DBG_DIR(dout<<" mylib find_num_files name = "<<name<<endl);


   rc = DosFindFirst((PSZ)name, &hdir, dirflags , (PVOID)&findbuf,
                     sizeof findbuf, &srchcount, 1L);
  if(rc == 0) {
     file_count++;
     while(rc == 0) {
          rc = DosFindNext(hdir, (PVOID)&findbuf, sizeof findbuf, &srchcount);
          file_count++;
     }
  }
  else {
      if(examine_error(rc, "") == TRUE)
         file_count = 0;
  }
         
  rc = DosFindClose(hdir);
  DBG_DIR(dout<<"find_num_files file_count = "<<file_count<<endl);
  DBG_DIR(dout<<"find_num_files DosFindClose rc = "<<rc<<endl);
return file_count;
}

/************************************************************************/
static void  make_entry (struct dir_entry *e)
{
strcpy (e->name, dos_buffer.name);
e->flags= dos_buffer.attribute;
e->time= dos_buffer.time;
e->date= dos_buffer.date;
e->size= dos_buffer.size;
e->mark= FALSE;                      /* and the last one was not read in */
}

/* **************************************** */

bool file_findfirst (char const* name, struct dir_entry *e, USHORT dirflags)
{
retry:                             /* in MYLIB.C  */
if (0 == assembly_findfirst (&dos_buffer, name, dirflags)) {
   DBG_DIR(dout<<" file_findfirst = "<<(dos_buffer.name)<<" hex flags = "<<hex<<dirflags<<dec<<endl);
   make_entry (e);
   return TRUE;
   }
   DBG_DIR(dout<<" file_findfirst returning FALSE "<<endl);
return FALSE;
}

/* **************************************** */

bool file_findnext (struct dir_entry *e)
{
if (0 == assembly_findnext(&dos_buffer)) {
   DBG_DIR(dout<<" file_findnext = "<<(dos_buffer.name)<<endl);
   make_entry (e);
   return TRUE;
   }
   DBG_DIR(dout<<" file_findnext returning FALSE "<<endl);
return FALSE;
}

/* **************************************** */
#ifndef NO_COMMANDO
void show_bytesfree (struct dirlist_data *d)
{
unsigned long bytes_free;
SHORT height, width;
char s[15];

retry:
bytes_free= get_freespace (d->name[0]);
//window_get_size (d->w,&height,&width);      // no commando
strcpy (s,"Free:");
num2str (s+5,bytes_free,9,' ');
put_string (d->w,height-1,(width-14)/2,s);
}
#endif
/* **************************************** */

static void  destroy_dirlist (struct dirlist_data *d)
{
//dirw->hide();
delete dirw;
dirw = 0;
prmptw = 0;
//pathw->hide();
delete pathw;
pathw = 0;
delete d->filelist;            
}

/* **************************************** */
void select_attr_flags (USHORT *attr)
/* allow dirlist to only show files with certain attributes */
{
static char *attr_name[]= {"Read-only xxx","Archive   xxx","System    xxx","Hidden    xxx"};
static SHORT attr_mask[]= { FA_RD_ONLY, FA_ARCHIVED, FA_SYSTEM, FA_HIDDEN };
static SHORT choice= 0;
SHORT pick;
#ifndef NO_HELP
DEF_HELP (DIRLIST_HELP,3,10,10);
NEW_HELP;
#endif

for (pick= 0; pick < 4; pick++)
   menu_onoff (attr_name[pick],10,*attr & attr_mask[pick]);
while ((pick= menu (attr_name, 4, &choice, MENU_USE_INITIALS|COMMANDO_LOCATION, 13)) != -1) {
   *attr^= attr_mask[pick];
   menu_onoff (attr_name[pick], 10, *attr & attr_mask[pick]);
   }

#ifndef NO_HELP
OLD_HELP;
#endif
}

/* ********************************************************************
    this actually puts the filenames into the window
**********************************************************************/
void display_entry (struct dirlist_data *d, struct dir_entry *e,SHORT row)
{
SHORT j, attr_pos, size_pos, date_pos;
//const int WIN_SIZE  = 70;                       // overall size of window
const int WIN_SIZE  = 66;      // left 4 spaces blank on right
int ATTRIB_POS = 35;                      // pos of attrib char 
 
char buf[WIN_SIZE], *s;                   // max size of any string
//char buf[WIN_SIZE];                   // max size of any string
char sbuf[WIN_SIZE];

DBG_DIR(dout<<"dirlist display_entry passed e->name = "<<(e->name)<<endl);

memset (buf,' ',WIN_SIZE-1);    
memset (sbuf,0x00,WIN_SIZE-1);    
s = &sbuf[0];
strncpy(s, e->name, WIN_SIZE-1);

if (e->mark)
    *buf= 0xfe;                 /* place bullet by filename if file marked */
else if (e->flags & 16)
         *buf= 0x1a;            /* symbol to denote directory */

       /* copy the filename, justifing the base name and extension seperatly */
//s= e->name;
if(strlen(s) >= ATTRIB_POS - 2) {
   s[ATTRIB_POS-2] = 0x00;         // cut it short
}
j= 1;                           /* start position for destination string */
if (*s != '.') {                /* skip if . or .. */
   while (*s && *s != '.')      /* copy up to (not including) dot */
      buf[j++]= *s++;
   j= 9;                        /* skip to extension column */
   }
while (*s)
   buf[j++]= *s++;
                                       // mod 12-6-93
//attr_pos= size_pos= date_pos= 15;    /* adjust positions for file info */
attr_pos= size_pos= date_pos= ATTRIB_POS;    /* adjust positions for file info */
if (d->flags & SHOWATTR) {
   size_pos+= 5;
   date_pos+= 5;                              // flag to show attribute?
   if (e->flags & 1) buf[attr_pos]= 'r';      // read-only
   if (e->flags & 32) buf[attr_pos+3]= 'a';   // archive
   if (e->flags & 4) buf[attr_pos+2]= 's';    // system
   if (e->flags & 2) buf[attr_pos+1]= 'h';    // hidden
   }
if (d->flags & SHOWSIZE) {                    /* show file size */
   date_pos+= 11;
   if (!(e->flags & 16)) {                    /* don't show if directory */
//      longnum2str (buf+size_pos, e->size, 8, 0);
      num2str (buf+size_pos, e->size, 8, 0);
      buf[size_pos+8]= ' ';
      }
   }
if (d->flags & SHOWDATE) {
                    /* the date:  dos format is bits 0-4 day of month
                                                     5-8 month
                                                     9-15 years after 1980 */
   num2str (buf+date_pos, (e->date >> 5) & 15, 2, 0);
   buf[date_pos+2]= '-';
   num2str (buf+date_pos+3, (e->date & 31), 2, '0');
   buf[date_pos+5]= '-';
   num2str (buf+date_pos+6, (e->date >> 9) + 80, 2, '0');
   buf[date_pos+8]= ' ';
                    /* the time:  dos format is bits 0-4  2 second increments
                                                     5-10 minutes
                                                    11-15 hours */
   num2str (buf+date_pos+9, (e->time >> 11), 2, 0);
   buf[date_pos+11]= ':';
   num2str (buf+date_pos+12, (e->time >> 5) & 63, 2, '0');
   buf[date_pos+15]= ' ';                 /* seconds not printed. */
   }
buf[d->width-2]= '\0';

if (e->flags & 16) {                       // directory?
   if (d->flags & SHOWATTR)
      memcpy (buf+attr_pos,"(DIR)",5);
   else if (d->flags & SHOWSIZE)
      memcpy (buf+size_pos+4,"(DIR)",5);
}
dirw->put(row + d->startrow,1,buf);         // print the buf

}

/* ******************************************************************** */
//static SHORT px,py,pwidth;

void redisplay_dirlist (struct dirlist_data *d)
{
SHORT end, i;
struct dir_entry *list;
DBG_DIR(dout<<"redisplay_dirlist  filelist_size = "<<(d->filelist_size)<<endl);

end= d->filelist_size-d->first_entry;
DBG_DIR(dout<<"redisplay_dirlist  d->first_entry = "<<(d->first_entry)<<" end = "<<end<<endl);
list = d->filelist;

list += d->first_entry;
for (i= 0; i < d->rows; i++)
   if (i < end) {
       DBG_DIR(dout<<"redisplay_dirlist entry = °"<<(list[i].name)<<"°"<<endl);
       display_entry (d, &list[i], i);
   }
   else
//       dirw->pad(i+d->startrow, 1, ' ', d->width-2);
       dirw->put_fill(i+d->startrow, 1,"", d->width-2);
}

/* **************************************** */
#ifndef NO_COMMANDO
void update_entry (struct dirlist_data *d,SHORT index)
{
           /* redraw entry on screen if it has not been scrolled off-screen */
struct dir_entry *list= (struct dir_entry *)Mem::vmem_trans (d->filelist);
if (index >= d->first_entry && index < d->first_entry+d->rows)
   display_entry (d,list+index,index-d->first_entry);
}
#endif
/* **************************************** */

/* ----- fit string in window ----- */
//void display_path (char const *filename,SHORT y,SHORT x,SHORT width)
void display_path (const char *filename,SHORT y,SHORT x,SHORT width)
{
SHORT len;
//USHORT pwidth = pathw->get_width();
pwidth = pathw->get_width();
len= strlen (filename);                   /* display the name */
//pathw->clearwin((colors[NORMAL_COLOR]), ' ');


DBG_PROMPT(dout<<"display_path, x = "<<x<<" y = "<<y<<endl);
DBG_DIR(dout<<"display_path  filename = °"<<filename<<"°"<<endl);

if (len <= pwidth) {
//   pad_window (pwnd, y,x, ' ',width);
   pathw->put_fill(x+1,y+1,"",pwidth);
//   put_string (pwnd, y,x, filename);
//   pathw->put_fill(x+1,y+1, filename, pwidth);
   pathw->put(x+1,y+1, filename);
   }
else  {                                   /* name is too long to fit */
//   put_string (pwnd, y,x+1, filename + (len-width)+1);
//   put_string (pwnd, y,x, "\x11\0");      /* show that display is truncated */
   pathw->put(y+1,x+2, filename + (len-width)+1);
   pathw->put(y+1,x+1, "\x11\0");      /* show that display is truncated */
   }
}

/* ******************************************************************** */
void read_list (struct dirlist_data *d, char *filename)
{
SHORT count= 0;
struct dir_entry *list;
char allfiles[FNAME_LEN];

strcpy (d->name, filename);
display_path (filename,py,px,pwidth);
                                         /* get ready to read the list */
list = d->filelist;

if (d->flags & SHOWDIRS) {         //Pass 1 finds all subdirs
   strcpy (allfiles, filename);
   replace_basename (allfiles, "*.*");
//   if (file_findfirst(allfiles, list, 16|7)) {
   if (file_findfirst(allfiles, list, MUST_HAVE_DIRECTORY)) {
      if (list->flags & 16) {            /* is a dir */
         list++;
         count= 1;
         }
      while (file_findnext (list)) {   
         if (list->flags & 16) {        /* is a dir */
            count++;
            list++;
         }
      }
   }
   else {
        goto end;
   }
}
if (d->flags & SHOWFILES) {       //Pass 2 finds regular files.
  SHORT filestart;
   d->current_entry= filestart= count;      /* cursor on first file */
//   if (file_findfirst(filename, list, 7)) { // find "system|rd_only|hidden
//   if (file_findfirst(filename, list, 0)) { // find "normal"
   if (file_findfirst(filename, list, FA_ATTRIB_BITS)) { // find "all"
      if (((list->flags&FA_ATTRIB_BITS) &(settings.dirlist_defaults.attr_flags^FA_ATTRIB_BITS)) == 0) {
         list++;
         count++;
      }
      while (file_findnext (list)) {
         if (((list->flags&FA_ATTRIB_BITS) &(settings.dirlist_defaults.attr_flags^FA_ATTRIB_BITS)) == 0) {
            count++;
            list++;
            }
         }
      }
   else 
      d->current_entry= 0;            /* no first file */
   if (filestart == count) 
      d->current_entry= count-1;
   }
else {
end:
   d->current_entry= 0;               /* no files, dirs only */
}
DBG_DIR(dout<<"dirlist_core  count = "<<count<<endl);
d->filelist_size= count;  
d->first_entry= 0;
d->tagcount= 0;
DBG_DIR(dout<<"redisplay_dirlist calling sort_dirlist "<<endl);
sort_dirlist (d);         
}
/******************************* **************************************** */

static SHORT  get_relrow (struct dirlist_data *d)
{
/* compute relrow of the bouncebar, and make sure it is on the screen. 
   only called by getkey_dirlist()    */
SHORT count;
struct dir_entry *list;
SHORT relrow= d->current_entry - d->first_entry;  /* first try */
DBG_DIR(dout<<"dirlist   get_relrow   relrow = "<<relrow<<endl);
list = d->filelist;

if (relrow < 0) {                               /* scroll down */
   d->first_entry += relrow;
   count= -relrow -1;
   if (-relrow < d->rows-1)
      dirw->scroll(Down, -relrow);
   else
      count= d->rows-1;
   list += d->first_entry;
   relrow= d->rows - count;
   while (count >= 0) {
      display_entry (d, list + count, count);
      count--;
      }
   relrow= 0;
   }
else if (relrow >= d->rows) {                  /* scroll up */
   count= 1 + (relrow - d->rows);
   d->first_entry += count;
   if (count < d->rows)
      dirw->scroll(Up, count);
   else
      count= d->rows;
   list += d->first_entry;
   relrow= d->rows - count;
   while (count--) {
      display_entry (d, list + relrow, relrow);
      relrow++;
      }
   relrow--;
   }
           /* relrow is now set to the offset from the start of the display */
relrow += (d->flags & PATHWINDOW) ? 1 : 3; /* if top of window used */
                                           /* now relrow is the window row */
return relrow;
}

/* **************************************** */

static USHORT getkey_dirlist (struct dirlist_data *d)
{
                      /* this gets a key, and does UP and DOWN arrows etc. */
USHORT key;
SHORT relrow;

if (d->filelist_size == 0)                   /* safety check */
   return kTAB;
for (;;) {
   relrow= get_relrow(d);
   dirw->highlight(relrow, 1, d->altcolor, d->width-2);
   key= getkey();
   DBG_PROMPT(dout<<" in getkey_dirlist, got key = "<<hex<<key<<dec<<endl);
   dirw->highlight (relrow, 1, d->color, d->width-2);
   switch (key) {
      case kUp:
         if (d->current_entry > 0) 
             d->current_entry--;
         break;
      case kDown:
         if (++d->current_entry == d->filelist_size) 
             d->current_entry--;
         break;
      case PageUp:
         if (d->current_entry == d->first_entry) {
            d->current_entry-= d->rows;
            if (d->current_entry < 0)
               d->current_entry= 0;
            }
         else
            d->current_entry= d->first_entry;
         break;
      case PageDown:
         if (d->current_entry == d->first_entry + d->rows-1)
            d->current_entry+= d->rows;
         else
            d->current_entry= d->first_entry + d->rows-1;
         if (d->current_entry > d->filelist_size-1)
            d->current_entry= d->filelist_size-1;
         break;
      case kHOME:
         if (d->current_entry == d->first_entry)
            d->current_entry= 0;
         else
            d->current_entry= d->first_entry;
         break;
      case kEND:
         if (d->current_entry == d->first_entry + d->rows-1)
            d->current_entry= d->filelist_size-1;
         else
            d->current_entry= d->first_entry + d->rows-1;
         if (d->current_entry > d->filelist_size-1)
            d->current_entry= d->filelist_size-1;
         break;
      default:  return key;
      }
   }
}

/* **************************************** */
#ifndef NO_COMMANDO
window_t make_pathwind (SHORT y,SHORT x,SHORT height,SHORT width, const char *border,SHORT path_y,SHORT path_x,SHORT path_w)
{
pwnd= create_window (height,width,y,x); 
//clear_window (pwnd,0,height-1,0,width-1,((colors[NORMAL_COLOR]<<8) | ' '));
window_border (pwnd, border, colors[BORDER_COLOR]);
display_on (pwnd);
pwidth= path_w;  /* width of path field (not window width) */
px= path_x;  /* coords for placement of pathname in window */
py= path_y;
return pwnd;
}

/* **************************************** */

void set_pathwind (window_t w,SHORT path_y,SHORT path_x,SHORT path_w)
{
pwnd= w;
pwidth= path_w;  /* width of path field (not window width) */
px= path_x;  /* coords for placement of pathname in window */
py= path_y;
}
#endif
/* **************************************** */

static SHORT  change_filename (struct dirlist_data *d, char* filename)
{
/* TAB swaps between filename edit box and directory box */
USHORT lastkey;
#ifndef NO_HELP
DEF_HELP (DIRLIST_HELP,4,10,10);
NEW_HELP;
#endif

strcpy (filename, d->name);
DBG_PROMPT(dout<<" in change filename, passed filename = "<<filename<<endl);
//edit_string (pwnd,py,px,pwidth,filename,79,colors,1+16+32);

//lastkey = edit_string (py,px,pwidth,filename,40,colors,1+16+32);
//lastkey = edit_string (1,1,strlen(filename)+1, filename,40,colors,16+32);
//lastkey = edit_string (1,1,strlen(filename)+1, filename,80,colors,16+32);
lastkey = edit_string (1,1,pwidth, filename,80,colors,16+32);

//lastkey= getkey();  /* find out if it is a <ENTER> <ESC> <F10> or whatever */
                        /* normalize the filename, based on the old name */
//normalize_filename (filename,cwd,"*");
normalize_filename_2 (filename,cwd,"*");
#ifndef NO_HELP
OLD_HELP;
#endif
return lastkey;
}

/* **************************************** */

bool dirlist_core (char const *title, char *filename, enum dirlist_flags flags,SHORT y,SHORT x,SHORT height)
/* flags:  SHOWDIRS - display files         SHOWDIRS - display directories
           SHOWATTR - display attributes    SHOWDATE - display date
           SHOWSIZE - display size          SHOWFREE - display bytes free
           COMMANDO - enable commando mode
           PATHWINDOW - use seperate window for path name (set with dirwind) */
{
USHORT key;
USHORT num_files = 0;
struct dirlist_data d;
struct dir_entry *list = 0;
char allfiles[FNAME_LEN];
int screen_height = Screen.get_screen_height();
//int screen_width = Screen.get_screen_width();
                                                    /* make the window */
d.rows= (height < 0) ? screen_height : height; 
if (d.rows + y > screen_height) {
//   d.rows= screen_height-y - 1;    /* keep height of window in range */
   d.rows= screen_height-y - 3;    /* keep height of window in range */
}
//d.width= 40;                       // width of file name field
d.width= 36;                       // width of file name field ??
if (flags & SHOWATTR) d.width+= 6;
if (flags & SHOWSIZE) d.width+= 10;
if (flags & SHOWDATE) d.width+= 15;

//if (x+d.width > screen_width)
//   x= screen_width-d.width;   /* make sure window is on screen */
d.flags= flags;

dirw = new basewin (d.rows, d.width, y, x);
d.altcolor= colors[HIGHLIGHT_COLOR];
d.color= colors[NORMAL_COLOR];
dirw->clearwin((d.color), ' ');
dirw->border (window_core::Double);

pathw = new basewin (3, d.width, y - 3, x);
prmptw = pathw;                         // this shouldn't work, need assign

//d.altcolor= colors[HIGHLIGHT_COLOR];
//d.color= colors[NORMAL_COLOR];
pathw->clearwin((d.color), ' ');
pathw->border (window_core::Single);


if (!(flags&PATHWINDOW)) {
//   window_border_size (d.w, border_width);
   d.startrow= 3;
   }
else {
   d.startrow= 1;
   }
if (title && strlen(title) < (USHORT)d.width+1)
   dirw->title(TCenter, title);

dirw->show();
pathw->put(1,1, filename);
pathw->show();

                   // path goes on top of this window, not in separate window
if (!(flags & PATHWINDOW)) {
   d.rows-= 2;
//   pwnd= d.w;
   px= py= 1;
   pwidth= d.width-2;
}
d.rows-= 2;                /* make rows reflect the interior of the window */
                           /* take care of the dir list */
/* size, first, and current entry will be reset when a list is read in,
   and do not need to be initialized. */

start_over:
   strcpy (allfiles, filename);
   replace_basename (allfiles, "*.*");      // find_num_files will get ALL
   DBG_DIR(dout<<"dirlist_core start_over "<<endl);
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
   num_files = find_num_files(allfiles);
   if(num_files == 0) {
      return FALSE;
   }
   if(list) {
      delete list;                          // if not the 1st time..
      DBG_DIR(dout<<"dirlist_core deleted  "<<endl);
   }
   list = new dir_entry[num_files];
   DBG_DIR(dout<<"dirlist_core allocated "<<num_files<<" files buffer"<<endl);
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
   d.filelist = list;                // only place where d.filelist is LHS
if(num_files > 0) {
   read_list (&d, filename);
}
                         //   if (d.flags & SHOWFREE) show_bytesfree (&d);
retry:
   DBG_DIR(dout<<"dirlist_core retry "<<endl);
   key= getkey_dirlist (&d);

switch (key) {
   case kTAB:
      DBG_PROMPT(dout<<"dirlist_core, got a TAB "<<endl);
      DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
      key= change_filename (&d, filename);  /* returns exited key */
      switch (key) {
         case kESC:
            if (d.filelist_size == 0) {
               destroy_dirlist (&d);
               return FALSE;
               }
            break;
         case FileList:
            destroy_dirlist (&d);
            return TRUE;
         }
      goto start_over;
   case kESC:
      destroy_dirlist (&d);
      DBG_DIR(dout<<"dirlist_core  line "<<__LINE__<<" returning FALSE "<<endl);
      return FALSE;
   case kBS:
      if (strlen (filename) > 3) {
         add_subdir (filename, "..");
         strcpy (d.name,filename);
         goto start_over;
         }
      goto retry;
   case GREY_Enter:
   case Enter:
      if (d.filelist_size == 0)
         goto retry;
      list= (dir_entry *)(d.filelist);
      if (list[d.current_entry].flags & 16) {   // go into this directory 
         add_subdir (filename, list[d.current_entry].name);
         strcpy (d.name,filename);
         goto start_over;
      }
      DBG_LOAD(dout<<"dirlist_core line "<<__LINE__<<"current list.name = "<<(list[d.current_entry].name)<<endl);
      replace_basename (filename, list[d.current_entry].name);
                                                         /* falls through */
      DBG_LOAD(dout<<"dirlist_core line "<<__LINE__<<"filename = "<<filename<<endl);
   case FileList:
      destroy_dirlist (&d);
      return TRUE;
   case kF3:
#ifndef NO_COMMANDO
      cmdo_sort (&d);                /* select method of sorting filenames */
#endif
      break;
   case kF4:
      select_attr_flags (&settings.dirlist_defaults.attr_flags);
      read_list (&d, filename);
      break;
   }
   goto retry;

}

/* **************************************** */

bool dirlist (char const *title, char *filename, enum dirlist_flags flags,SHORT y,SHORT x,SHORT height)
{
bool b;
#ifndef NO_HELP
DEF_HELP (7,1,10,10);
NEW_HELP;
#endif

//status_note (" F1-Help   F3-Sort method   F4-Attribute filter   TAB-Edit pathname");
status_note (" F1-Help   F4-Attribute filter   TAB-Edit pathname");
b= dirlist_core (title,filename,flags,y,x,height);
status_note (NULL);

#ifndef NO_HELP
OLD_HELP;
#endif
DBG_DIR(dout<<"dirlist  line "<<__LINE__<<" returning "<<b<<endl);
DBG_LOAD(dout<<"dirlist line "<<__LINE__<<" filename = "<<filename<<endl);
return b;
}

























