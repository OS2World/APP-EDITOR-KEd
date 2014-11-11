/*****************************************************
File: HYPRHELP.C    Copyright 1990 by Dlugosz Software
   The context-sensitive hypertext help system
*****************************************************/
#define INCL_DOS
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "help.h"
#include "mylib.h"
#include "getkey.h"
#include "keycodes.h"
#include "setting.h"
#include "redbox.h"
#include "sheet.h"  /* for sheet_mem */
#include "misc.h"
#include <string.h>
#include <stdlib.h>

#include "basewin.h"

#include "test.h"

basewin *hyhelpw = 0;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#define BACKUP_LIMIT 30
#define PAGE_LIMIT 16

struct index_rec {
   byte locus, screen;
   USHORT location;
   };

struct help_loc {
       unsigned width  : 5;
       unsigned col    : 6;
       unsigned row    : 5;
       unsigned unused : 16;
};

struct link_rec {
   USHORT location;
//   help_loc data;
   USHORT data; /* bits 0-4 = width   5-10 = column   11-15 = row */
   };

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
          
//static window_t w;
static bool done, error_flag;
static mylib_file_t helpfile;
static USHORT next_location;
static byte page_count;
static SHORT backup_count;
static USHORT *backup;
static byte *backup_link;
static byte *backup_page;
static vptr link_v;
static struct link_rec *links;
static byte linkcount;
static byte width, height;
static byte curlink;
static byte pagenum;

static unsigned long page_locations[PAGE_LIMIT];  /* assume no more than 20 pages */

static struct help_rec *help;  /* currently active help structure */

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  remember_backup (void)
{
if (backup_count == BACKUP_LIMIT) {  /* filled to capacity */
   memmove (backup, backup+1, 2*(BACKUP_LIMIT-1));
   memmove (backup_link, backup_link+1, BACKUP_LIMIT-1);
   memmove (backup_page, backup_page+1, BACKUP_LIMIT-1);
   backup[BACKUP_LIMIT-1]= next_location;
   backup_link[BACKUP_LIMIT-1]= curlink;
   backup_page[BACKUP_LIMIT-1]= pagenum;
   }
else {
   backup[backup_count]= next_location;
   backup_link[backup_count]= curlink;
   backup_page[backup_count]= pagenum;
   backup_count++;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void in(void *x, USHORT len)
{
if (mylib_read_text (helpfile,(unsigned char *)x,len) != len) {
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  check_index (SHORT locus, SHORT screen)
{
struct index_rec index;
error_flag= FALSE;

DBG_HYPR1(dout<<"check_index searching locus = "<<locus<<" screen = "<<screen<<endl);

if (!mylib_seek (helpfile, 0L, MYLIB_SEEK_SET)) {  /* rewind */
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   return FALSE;
   }
do {
   in (&index,sizeof (struct index_rec));
   if (error_flag || index.locus > (byte)locus) break;
   if (index.screen == (byte)screen && index.locus == (byte)locus) {
      next_location= index.location;
      return TRUE;
      }
   } while (index.locus != 0);
return FALSE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool read_lpstring (char *s)
{
byte len;

in (&len,1);
if (error_flag) return FALSE;
if (len) in (s, len);
if (error_flag) return FALSE;
s[len]= '\0';
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  readpage_quietly (void)
{
byte count;
USHORT loop;
DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" read page quietly"<<endl);
                                             /* retrieve shade records */
in (&count,1);
if (error_flag) return FALSE;
if (mylib_seek (helpfile,count*2,MYLIB_SEEK_CUR)) {
   redbox (13,202,eh_cancel);
   error_flag= TRUE;
   return FALSE;
   }
in (&count,1);                                /* retrieve hypertext links */
if (error_flag) return FALSE;
if (mylib_seek (helpfile,count*sizeof(struct link_rec),MYLIB_SEEK_CUR)) {
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   return FALSE;
   }
in (&count,1);                                /* read title */
if (mylib_seek (helpfile,count,MYLIB_SEEK_CUR)) {
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   return FALSE;
   }
for (loop= 0; loop < height; loop++) {         /* read lines */
   in (&count,1);
   if (mylib_seek (helpfile,count,MYLIB_SEEK_CUR)) {
      redbox (13,201,eh_cancel);
      error_flag= TRUE;
      return FALSE;
      }
   }
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  read_page (SHORT pagenum)
{
 USHORT loop, firstcol;
char line[132];
byte shadecount;
vptr shade_v;
USHORT *shades;
static char s[]= "x/y";
DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" read page number "<<pagenum<<endl);
                                                       /* seek to page */
if (!mylib_seek (helpfile,page_locations[pagenum],MYLIB_SEEK_SET)) {
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   return FALSE;
   }
in (&shadecount,1);                            /* retrieve shade records */
if (error_flag) return FALSE;
if (shadecount > 0) {
   shades= (USHORT *)vmem_alloc (sheet_mem, shadecount*2, &shade_v);
   if (mylib_read_text (helpfile,(UCHAR *)shades,shadecount*2) != (USHORT)shadecount*2) {
      redbox (13,202,eh_cancel);
      error_flag= TRUE;
      vmem_free (shade_v);
      return FALSE;
      }
   }
in (&linkcount,1);                             /* retrieve hypertext links */
DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" read page linkcount = "<<linkcount<<endl);
if (error_flag) {
   if (shadecount > 0) vmem_free (shade_v);
   return FALSE;
   }
if (link_v) 
   vmem_free (link_v);                         /* clear out previous page */

if (linkcount > 0) {
   links= (struct link_rec *)vmem_alloc (sheet_mem,linkcount*sizeof(struct link_rec),&link_v);
   if (mylib_read_text (helpfile,(UCHAR *)links,linkcount*sizeof(struct link_rec)) != linkcount*sizeof(struct link_rec)) {
      redbox (13,201,eh_cancel);
      error_flag= TRUE;
      if (shadecount > 0) 
          vmem_free (shade_v);
      vmem_free (link_v);
      link_v= 0L;
      return FALSE;
      }
   }
read_lpstring (line);                   /* read title */
if (error_flag) {
   if (shadecount > 0) vmem_free (shade_v);
   return FALSE;
   }
                                        /* prepare the window */
hyhelpw->clearwin((settings.colors[HELP_COLORS][NORMAL_COLOR]),' ');
hyhelpw->border(window_core::Single,settings.colors[HELP_COLORS][BORDER_COLOR]);
hyhelpw->show();

if (page_count > 1) {
   s[0]= (char)(pagenum+'1');
   s[2]= (char)(page_count+'0');
   hyhelpw->title(TRight,s);      // page "1/4" 
   firstcol= width-3;            /* first column for PgUp or PgDn message */
   if (pagenum < page_count-1) {
//      put_string (w,height+1,firstcol,"PgDn");
      hyhelpw->put(height+1,firstcol,"PgDn");
//      hyhelpw->title(BRight,"PgDn");
      firstcol= width-8;
//      }
   if (pagenum > 0) 
//      put_string (w,height+1,firstcol,"PgUp");
//      hyhelpw->put(height+1,firstcol,"PgUp");
      hyhelpw->put(height+1,width-4,"/");
   }
   if (pagenum > 0) 
       hyhelpw->put (height+1,firstcol,"PgUp");
}
firstcol= (width+2-strlen (line))/2;
hyhelpw->title(TCenter,line);
//highlight (w,0,firstcol,settings.colors[HELP_COLORS][OTHER_COLOR],strlen (line));
hyhelpw->highlight(0,firstcol,settings.colors[HELP_COLORS][SEARCH_COLOR],strlen(line)+1);
      DBG_HYPR1(dout<<"displaying title ="<<line<<endl);
      DBG_HYPR1(dout<<hex<<"col data = "<<firstcol<<endl);

for (loop= 0; loop < height; loop++) {       /* now read lines */
   read_lpstring (line);
   if (error_flag) {
      if (shadecount > 0) vmem_free (shade_v);
      return FALSE;
      }
   hyhelpw->put(loop+1,1,line);
   DBG_HYPR1(dout<<" hypr line = "<<line<<endl);
   }
if (shadecount > 0) {                   /* display shaderecs */
   SHORT lastrow= 0;
   shades= (USHORT *)Mem::vmem_trans (shade_v);
   for (loop= 0; loop < shadecount; loop++) {
      USHORT x= shades[loop];
      USHORT len= x&31;            // 0x1F
      byte color= (byte)((x&32)?HIGHLIGHT_COLOR:SEARCH_COLOR);
      USHORT col= (x>>=6)&63;      // 0x3F
      USHORT row= (x>>6) + lastrow;
//      highlight (w,row+1, col+1, settings.colors[HELP_COLORS][color], len);
      hyhelpw->highlight(row+1,col+1,settings.colors[HELP_COLORS][color],len+1);
      lastrow= row;
      DBG_HYPR1(dout<<"displaying shadecounts "<<endl);
      DBG_HYPR1(dout<<hex<<"row data = "<<row<<endl);
      DBG_HYPR1(dout<<hex<<"row data = "<<row<<endl);
      DBG_HYPR1(dout<<hex<<"col data = "<<col<<endl);
//      DBG_HYPR1(dout<<hex<<"data = "<<data<<endl);
      }
   vmem_free (shade_v);
   }
if (linkcount > 0) {                             /* display linkrecs */
   links= (struct link_rec *)Mem::vmem_trans (link_v);
   for (loop= 0; loop < linkcount; loop++) {
//      highlight (w,(links[loop].data>>11)+1,((links[loop].data>>5)&63)+1,
//         settings.colors[HELP_COLORS][BLOCK_COLOR],links[loop].data&31);
      DBG_HYPR1(dout<<dec<<"displaying linkcounts "<<endl);
      DBG_HYPR1(dout<<hex<<"data = "<<links[loop].data<<endl);
      DBG_HYPR1(dout<<hex<<"row data >>11= "<<((links[loop].data)>>11)<<endl);
      DBG_HYPR1(dout<<hex<<"col data >>5 = "<<(((links[loop].data)>>5)&63)<<endl);
      hyhelpw->highlight((links[loop].data>>11)+1,((links[loop].data>>5)&63)+1,
             settings.colors[HELP_COLORS][BLOCK_COLOR], links[loop].data&31);
      }

   }
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  load_screen (void)
{
SHORT x,y;
USHORT echo_location;
long fileptr= next_location*4L;
int screen_height = Screen.get_screen_height();
int screen_width = Screen.get_screen_width();

DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" load screen"<<endl);
//set_win();
if (!mylib_random_read (helpfile,fileptr,&echo_location,2)) {
   redbox (13,201,eh_cancel);
   error_flag= TRUE;
   return;
   }
if (echo_location != next_location) {
   redbox (13,202,eh_cancel);
   error_flag= TRUE;
   return;
   }
if (error_flag) return;
in (&width,1);
if (error_flag) return;
in (&height,1);
if (error_flag) return;
in (&page_count,1);
if (error_flag) return;

/* calculate window's x coordinate */
x= help->x;
if (x == -1) x= (screen_width-(width+2))/2;
if (x+width+2 > screen_width) if ((x= screen_width-(width+2)) < 0) x= 0;

/* calculate window's y coordinate */
y= help->y;
if (y == -1) y= (screen_height-(height+2))/2;
if (y+height+2 > screen_height) if ((y= screen_height-(height+2)) < 0) y= 0;

/* if new window is same size and position as old window, don't create a new one */
if (hyhelpw) {
//   SHORT old_w,old_h,old_y,old_x;
//   window_get_position (w,&old_y,&old_x);
//   window_get_size (w,&old_h,&old_w);
//   if (old_x != x || old_y != y || old_h-2 != (SHORT)height || old_w-2 != (SHORT)width) {
//      destroy_window (w);                      // NEED to DELETE OLD!
     delete hyhelpw;
//      w= create_window (height+2,width+2,y,x);
      hyhelpw = new basewin(height+2,width+2,y,x);
//      }
   }
//else w= create_window (height+2,width+2,y,x);
else 
   hyhelpw = new basewin(height+2,width+2,y,x);

page_locations[0]= mylib_tell (helpfile);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  unload_screen (void)
{
if (link_v) {
   vmem_free (link_v);
   link_v= 0L;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT help_getkey (SHORT curlink)
{
if (linkcount == 0) {
    return getkey ();
}
else {
   SHORT row, col, len, key;
   
   links= (struct link_rec *)Mem::vmem_trans (link_v);
   row= (links[curlink].data>>11)+1;
   col= ((links[curlink].data>>5)&63)+1;
   len= links[curlink].data&31;

//   row= links[curlink].data.row + 1;
//   col= links[curlink].data.col + 1;
//   len= links[curlink].data.width;

//   highlight (w,row,col,settings.colors[HELP_COLORS][BLOCK_CURSOR_COLOR],len);
   hyhelpw->highlight(row,col,settings.colors[HELP_COLORS][BLOCK_CURSOR_COLOR],len);
   key= getkey ();
//   highlight (w,row,col,settings.colors[HELP_COLORS][BLOCK_COLOR],len);
   hyhelpw->highlight(row,col,settings.colors[HELP_COLORS][BLOCK_COLOR],len);
   return key;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  show_screen (void)
{
USHORT loop;
SHORT lastpage= 0;
USHORT key;

if (error_flag) 
   return;
DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" show screen"<<endl);
for (loop= 0; loop < pagenum; loop++) {
   if (!readpage_quietly ()) return;
   page_locations[++lastpage]= mylib_tell (helpfile);
   }
/* read and display first page */
read_page (pagenum);
for (;;) {
   key= help_getkey (curlink);
   switch (key) {
      case kESC:
         done= TRUE;
         return;
      case Help:  /* "Help on Help" */
         {
         unsigned long this_location, filepos;
         /* locus 255, screen 0 is the "help on help" screen */
         this_location= next_location;
         filepos= mylib_tell (helpfile);
         remember_backup ();
         if (check_index (255,0)) {
            DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<"called check_index(255,0)"<<endl);
            if (next_location != (USHORT)this_location) {
               pagenum= curlink= 0;
               return;
               }
            }
         else redbox (13,203,eh_continue); /* no help on help */
         /* if not found, retrieve last page and abort operation */
         mylib_seek (helpfile,filepos,MYLIB_SEEK_SET);
         backup_count--;
         next_location= backup[backup_count];
         curlink= backup_link[backup_count];
         pagenum= backup_page[backup_count];
         }
         break;
      case PageDown:
      case PageDowne:
         if (pagenum < (PAGE_LIMIT-1) && (pagenum < (byte)(page_count-1))) {
            if (++pagenum > (byte)lastpage) {
               /* remember location of next page for future references */
               page_locations[++lastpage]= mylib_tell (helpfile);
               }
            read_page (pagenum);
            curlink= 0;
            }
         break;
      case PageUp:
      case PageUpe:
         if (pagenum > 0) {
            pagenum--;
            read_page (pagenum);
            curlink= 0;
            }
         break;
      case kDown:
      case kDowne:
         if (linkcount > 0) {
            SHORT loop= curlink+1, thisline, thiscol;
            links= (struct link_rec *)Mem::vmem_trans (link_v);
            thisline= links[curlink].data >> 11;
            thiscol= (links[curlink].data >> 5) & 63;

//            thisline = links[curlink].data.row;
//            thiscol  = links[curlink].data.col;

            while (loop < (SHORT)linkcount) {
               if ((links[loop].data>>11) != (USHORT)thisline &&
                   ((links[loop].data>>5)&63) == (USHORT)thiscol) break; /* found one */

//               if ((links[loop].data.row != (USHORT)thisline) &&
//                   (links[loop].data.col == (USHORT)thiscol)) 
//                   break; /* found one */

               loop++;
               }
            if (loop < (SHORT)linkcount) curlink= (byte)loop;
            else if (++curlink == linkcount) curlink--;
            }
         break;
      case kRight:
      case kRighte:
         if (linkcount > 0 && (++curlink >= linkcount)) curlink= 0;
         break;
      case kUp:
      case kUpe:
         if (linkcount > 0 && curlink > 0) {
            SHORT loop= curlink-1, thisline, thiscol;
            links= (struct link_rec *)Mem::vmem_trans (link_v);
            thisline= links[curlink].data>>11;
            thiscol= (links[curlink].data>>5)&63;

//            thisline = links[curlink].data.row;
//            thiscol  = links[curlink].data.col;

            while (loop >= 0) {
               if ((links[loop].data>>11) != (USHORT)thisline &&
                  ((links[loop].data>>5)&63) == (USHORT)thiscol) break; /* found one */

//               if ((links[loop].data.row != (USHORT)thisline) &&
//                  (links[loop].data.col == (USHORT)thiscol)) 
//                  break; /* found one */

               loop--;
               }
            if (loop >= 0) curlink= (byte)loop;
#pragma warn -ccc                          // warning condition always false
            else if (--curlink < 0) curlink= 0;   
#pragma warn +ccc
            }
         break;
      case kLeft:
      case kLefte:
         if (linkcount > 0) {
            if (curlink == 0) curlink= (byte)(linkcount-1);
            else curlink--;
            }
         break;
      case kHOME:
      case kHOMEe:
         curlink= 0;
         break;
      case kEND:
      case kENDe:
         curlink= (byte)(linkcount-1);
         break;
      case Enter:  /* select hypertext link */
         if (linkcount == 0) break;
         remember_backup ();
         links= (struct link_rec *)Mem::vmem_trans (link_v);
         next_location= links[curlink].location;
         pagenum= curlink= 0;
         return;
      case kBS:
         if (backup_count) {
            backup_count--;
            next_location= backup[backup_count];
            curlink= backup_link[backup_count];
            pagenum= backup_page[backup_count];
            return;
            }
         break;
//      default:
//         if (keymove_window (w,-1,key)) break; /* move window, not used */
      }
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  show_help (void)
{
USHORT backup_array[BACKUP_LIMIT];
byte backup_link_array[BACKUP_LIMIT];
byte backup_page_array[BACKUP_LIMIT];

backup_count= 0;
backup= backup_array;
backup_link= backup_link_array;
backup_page= backup_page_array;

done= FALSE;
do {
   DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" show help"<<endl);
   load_screen();
   if (!error_flag) show_screen();
   if (!error_flag) unload_screen();
   } while (!done && !error_flag);
//if (w) 
//destroy_window (w);
delete hyhelpw;
hyhelpw = 0;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool (* OldFilter) (USHORT);
static USHORT help_key;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef NO_HELP
void init_help (USHORT key)
{
if (key) {
   help_key= key;
   if (key_preprocess != do_help) 
      {
      OldFilter= key_preprocess;
      key_preprocess= do_help;
      }
   }
else
   key_preprocess= OldFilter;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef NO_HELP
struct help_rec *set_help (struct help_rec *help_info)
/* change to new help data and return ptr to old help data */
{
struct help_rec *old= help;  /* remember currently active help */
help= help_info;
if(help != 0) {
   DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" set_help HELP locus = "<<(help->locus)<<" screen = "<<(help->screen)<<endl);
}
if(old != 0) {
   DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" set_help OLD HELP locus = "<<(old->locus)<<" screen = "<<(old->screen)<<endl);
}
return old;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//static bool do_help (USHORT key)
bool do_help (USHORT key)
{
static bool inhelp= FALSE;
bool ok = FALSE;
//USHORT return_code;
char help_path[FNAME_LEN];

if (inhelp || key != help_key) {
   if (OldFilter) return OldFilter (key); /* jump to old routine */
   else return TRUE;
   }
if (help->locus == 0) return FALSE; /* locus 0 means no help availiable */
inhelp= TRUE;

   ok = mylib_open (settings.help_path,&helpfile,MYLIB_READONLY);
   if (ok == FALSE){
      DosSearchPath(2,"PATH","KED.HLP",help_path,sizeof(help_path));
      ok = mylib_open ((char *)help_path, &helpfile, MYLIB_READONLY);
    }
   DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" do_help locus = "<<(help->locus)<<" screen = "<<(help->screen)<<endl);
   if (ok == TRUE) {
   if (check_index (help->locus, help->screen)) {
      DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" called check_index locus = "<<(help->locus)<<" screen = "<<(help->screen)<<endl);
      curlink= pagenum= 0;
//      link_v= w= 0L;
      link_v= 0L;
      show_help ();
      }
   else if (!error_flag)
      redbox (13,200,eh_cancel);
   mylib_close (helpfile);
   }
else 
   redbox_s (2,205,eh_continue,settings.help_path); // help file not found 

inhelp= FALSE;
return FALSE;
}

#undef my_alloc
#undef my_free
