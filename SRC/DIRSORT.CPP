/*****************************************************
File: DIRSORT.C     copyright 1989 by Dlugosz Software
   Sort entries in a dirlist
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "dirlist.h"
#include "setting.h"
#include "string.h"
                      //#include "menu.h"     // needed only for COMMANDO
#include "help.h"
#include <stdlib.h>              // need qsort

#include "test.h"

#define FTYPE _stdcall          /* BCOS2 */

/* =============== Sorting Functions =============== */

/* ----- sort entries by filename ----- */
static int FTYPE cmp_name (const void *v, const void *w)
{
struct dir_entry *x = (struct dir_entry *)v;
struct dir_entry *y = (struct dir_entry *)w;
return strcmp (x->name,y->name) * settings.dirlist_defaults.direction;
}

/* ---- reverse the order of name and extension
      (allows sorting by extension followed by name) ---- */
static void strchrcpy (const char *src, char *dest)
{
SHORT i= 3;
char *d= dest+3;

while (*src && *src != '.') *d++= *src++;   /* copy filename */
*d= '\0';
if (*src == '.') src++;  /* skip dot */
while (i--)              /* copy extension */
   *dest++= *src ? *src++ :(char)' ';
}

/******************sort by extension **************************/
static int FTYPE cmp_ext (const void *v, const void *w)
{
struct dir_entry *x = (dir_entry *)v;
struct dir_entry *y = (dir_entry *)w;
char name1[13],name2[13];

          //to sort by extension then by name, use extension followed by name
strchrcpy (x->name,name1);
strchrcpy (y->name,name2);
return strcmp (name1,name2) * settings.dirlist_defaults.direction;
}

/* ----- sort by date and time ----- */
static int FTYPE cmp_date (const void *v, const void *w)
{
struct dir_entry *x = (dir_entry *)v;
struct dir_entry *y = (dir_entry *)w;
unsigned long date1= ((unsigned long)x->date << 16) | x->time,
              date2= ((unsigned long)y->date << 16) | y->time;
if (date1 == date2) return 0;
return ((date1 < date2) ? 1 : -1) * settings.dirlist_defaults.direction;
}

/* ----- sort by filesize ----- */
static int FTYPE cmp_size (const void *v, const void *w)
{
struct dir_entry *x = (dir_entry *)v;
struct dir_entry *y = (dir_entry *)w;
if (x->flags & 16)
   return strcmp (x->name,y->name);  /* compare dirs by name, not size */
if (x->size == y->size) return 0;
return ((x->size > y->size) ? 1 : -1) * settings.dirlist_defaults.direction;
}

/* ----- sort directories by name and files by chosen method ----- */
void sort_dirlist (struct dirlist_data *d)
{
typedef int FTYPE (*sorter)(const void*, const void*);
//typedef int FTYPE (*sorter)(const struct dir_entry *, const struct dir_entry *);
static sorter(sortcmp[])   = { cmp_name,
                                cmp_ext,
                                cmp_date,
                                cmp_size};
//struct dir_entry *e= (struct dir_entry *)Mem::vmem_trans (d->filelist);
struct dir_entry *e= (dir_entry *)(d->filelist);
SHORT i= 0, s= 0;
DBG_DIR(dout<<"sort_dirlist filelist_size = "<<(d->filelist_size)<<endl);
DBG_DIR(dout<<"sort_dirlist first e->name = "<<(e->name)<<endl);
DBG_DIR(dout<<"sort_dirlist second e->name = "<<(e[1].name)<<endl);

                                               // first sort the directories 
while ((i < d->filelist_size) && (e[i].flags & 16)) {
    DBG_DIR(dout<<"sort_dirlist checking dir_flag  i = "<<i<<endl);
    i++;
}
DBG_DIR(dout<<"sort_dirlist about dot_dir "<<endl);

if (i != 0) {                                 // no directories 
   DBG_DIR(dout<<"sort_dirlist about dot_dir e->name = "<<(e->name)<<endl);
   if (dot_dir (e->name)) {
      s++;
      if (dotdot_dir (e[1].name))             //skip the . and .. dir entries 
         s++;
      }
   DBG_DIR(dout<<"sort_dirlist about to qsort dirs "<<endl);
   qsort (e+s,i-s,sizeof(struct dir_entry),
            sortcmp[settings.dirlist_defaults.sort_type]);
   }
              /* now sort file entries using sorting method chosen by user */
DBG_DIR(dout<<"sort_dirlist sorting filenames "<<endl);
s= d->filelist_size-i;
if (s) 
  qsort (e+i,s,sizeof (struct dir_entry),sortcmp[settings.dirlist_defaults.sort_type]);

//#ifdef TESTING
//struct dir_entry *tmp= (dir_entry *)(d->filelist);
//for(int j = 0; j < d->filelist_size; j++) {
//   DBG_DIR(dout<<"qsorted dir list entry "<<j<<" = "<<(tmp[j].name)<<endl);   
//}
//#endif

redisplay_dirlist (d);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef NO_COMMANDO
static char *choices[]= { "Name","Extension","Date","Size","Reverse OFF" };

void cmdo_sort (struct dirlist_data *d)
   /* the sort menu */
{
SHORT lastchoice= settings.dirlist_defaults.sort_type;
SHORT sort;
DEF_HELP (7,2,10,10);

NEW_HELP;
menu_onoff (choices[4],8,settings.dirlist_defaults.direction == -1);
if ((sort= menu(choices,5,&lastchoice,MENU_USE_INITIALS|COMMANDO_LOCATION,11)) == -1) {
   OLD_HELP;
   return;
   }
if (sort == 4) 
   settings.dirlist_defaults.direction*= -1;
else 
   settings.dirlist_defaults.sort_type= sort;  /*sort_type global to commando.c */
sort_dirlist (d);
OLD_HELP;
}
#endif
