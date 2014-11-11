/*****************************************************
File: SETTING.C  copyright 1989,90 by Dlugosz Software
   setting info for the editor
*****************************************************/
#include "usual.h"
#include "vmem.h"   /* needed by others */
#include "window.h" /* needed by view.h */
#include "sheet.h"  /* needed by view.h */
#include "view.h"   /* it defines types needed by viewer */
#include "mylib.h"  /* used to save/load settings */
#include "dirlist.h" /* need cwd variable and file attribute bits */
#include "setting.h"
#include "search.h" /* for search flags */
#include <string.h>   /* memcpy() used by packup and unpack */
#include <stdio.h>

#include "test.h"

static void debug_current_settings(void);

//char setting_filename[FNAME_LEN];
char setting_filename[] = "KED.INI";
//
//                         norm  high  block blk  win
//                         text  text  clr   crsr border

#define VIEW1            { 0x1e, 0x70, 0x30, 0x0f, 0x17, 0x5f }
#define VIEW2            { 0x6e, 0x70, 0x30, 0x0f, 0x6e, 0x5f }
#define VIEW3            { 0x70, 0x1f, 0x07, 0x30, 0x78, 0x5f }
#define VIEW4            { 0x30, 0x1f, 0x0b, 0x70, 0x30, 0x9e }
#define DIRLIST_PALETTE  { 0x6f, 0x0e, 0x6f, 0x1e, 0x6f, 0x00 }
//#define MENU_PALETTE     { 15+16, 15+48, 0x70, 0x07, 15+16 } // mod 9-14
#define MENU_PALETTE     { 0x0f, 15+48, 0x70, 0x07, 15+16,0x00  } 
//#define MENU_PALETTE     { 15+16, 0x17, 0x17, 0x07, 15+16 } // mod mono 8-29
#define REDBOX_PALETTE   { 0x4f, 0x70, 0x4e, 0x4f, 0x4e, 0x00 }
#define STATUS_PALETTE   { 0x4e, 0x07, 0x70, 0x07, 0x70, 0x00 }
#define DIALOG_PALETTE   { 0x70, 0x07, 0x7f, 0x71, 0x78, 0x00 }
#define HELP_PALETTE     { 0x70, 0x0f, 0x71, 0x1f, 0x78, 0x74 }
#define ASCIIBOX_PALETTE { 0x70, 0x1f, 0x70, 0x07, 0x07, 0x00 }
#define CALCULATOR_PALETTE {0x0f,0x30, 0x0a, 0x07, 0x02, 0x00 }
#define STANDARD_BW      { 0x1f, 0x0A, 0x70, 0x07, 0x10, 0x00 }
                          // 0F    70                07

#define STANDARD_SIZE_WINDOW {24,80,0,0}
typedef byte chart[COLORSET_COUNT][6];            // byte = unsigned char

static chart mono_colors= {
   STANDARD_BW, STANDARD_BW, STANDARD_BW, {0x70,0x07,0x07,0x70} ,
   STANDARD_BW, {0x07,0x70,0x01,0x70,0x0f,0x0f}, STANDARD_BW, STANDARD_BW,
   STANDARD_BW, STANDARD_BW, STANDARD_BW,  STANDARD_BW };

static chart color_colors= {
      DIRLIST_PALETTE, MENU_PALETTE, REDBOX_PALETTE, STATUS_PALETTE,
      DIALOG_PALETTE, HELP_PALETTE, ASCIIBOX_PALETTE, CALCULATOR_PALETTE,
      VIEW1, VIEW2, VIEW3, VIEW4 };

struct setting_info settings= {
      {{2,0},{1,0},{30,7},{10,8},{5,2},{1,1},{0,0},{10,5}, // menu locations 
      {2,2},                   // 8 file    WAS 1,2
      {10,2},                   // 9 block  
      {14,2},                  // 10 view
      {18,2},                  // 11 search  
      {42,2},                  // 12 options   WAS 36
      {34,2},                  // 13 about the ed  
      {26,2},                  // 14 print         
      {38,2}},                 // 15 unassigned
     {                         // colors 
      DIRLIST_PALETTE, MENU_PALETTE, REDBOX_PALETTE, STATUS_PALETTE,
      DIALOG_PALETTE, HELP_PALETTE, ASCIIBOX_PALETTE, CALCULATOR_PALETTE,
      VIEW1, VIEW2, VIEW3, VIEW4
      },
    {                              /* viewer_defaults */
      1,                           // reserved topline (normally border)
      1,                           // reserved bottomline (border)
      1+2, // 8 for tabs           /* flags, insert on, indent on, tabs on */
                // 32 for word wrap  /* indent on TEMP   10-17-93
      0,0,                         /* scroll margins */
      1,                           /* view_page_mode = new style pageup/dn */
      view_rec::bk_stream,         /* block mode */
      9,                           /* color index */
      4,                           /* tab size */
      100,                        /* default Right Margin for word wrap */
      ">>",                        /* default Quote Prefix for msg paste */
      },                // viewer_defaults end
   "*.BAK",                        /* backup path */
   "*.S$",                         /* temporary file path */
   "",                             /* help path */
   2+4,                     // flags 1 = autobackup OFF, 2 = save_save ON,
                            // 4 = expand tabs on file load 
                            // 8 = compress tabs on file save
   FALSE,                          /* write EOF-OFF */
   FALSE,                          /* numlock normally off */
   {                               /* zoom defaults */
//      {STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
      {STANDARD_SIZE_WINDOW,{15,60,10,10},STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW,
       STANDARD_SIZE_WINDOW,STANDARD_SIZE_WINDOW}
      },
                                      /* search/replace defaults */
      {QUERY_REPLACE | SEARCH_IGNORE_CASE,    /* flags */
       1,                             /* search direction      */
        },                            /* (1 == forward, -1 == backwards) */
     "C",                             /* file extension */
                                      /* dirlist defaults */
      {0,                             /* sort entries by name */
       1,                             /* sort direction            */
       FA_ARCHIVED|FA_RD_ONLY         /*(1 == ascending, -1 == descending) */
      },
   "",                                // printer_setup_string
   "PRN",                             /* printer */
                                      /* shell command */
   {"  cmd.exe /k", 1|4 },
   "   ",
   };

static char settings_header[]= "KEd_OS2_VERSION_2.17\n\x1a";

/******************************************************************
     load a built-in colorset, typically used to set MONO colors
**************************************************************************/
void setting_defaultcolors (SHORT n)
{
  static chart *p[]= { &mono_colors, &color_colors };
  memcpy (settings.colors, p[n], sizeof (settings.colors));
}

/*************************************************************************/
bool setting_load (char *name)
{                                            /* load setting file as named. */
    char buf[sizeof (settings_header)];
    USHORT bytes_read = 0;
    bool ok= FALSE;
    mylib_file_t f;
    DBG_INI(dout<<"           BEFORE   SETTINGS  LOADED           "<<endl);
    debug_current_settings();

    DBG_INI(dout<<" setting.c setting path =  "<<name<<endl);

    if (! mylib_open(name, &f, MYLIB_READONLY)) {
        DBG_INI(dout<<" setting.c open fail =  "<<name<<endl);
        return FALSE;
    }
    bytes_read = (mylib_read_text(f,(byte *)buf,(ULONG)strlen(settings_header)+1));
    if(bytes_read == 0)
        goto exit;
    if (strcmp (buf,settings_header) != 0){
        DBG_INI(dout<<" setting.c settings not match "<<endl);
        goto exit;                                 /* headers do not match */
    }
    bytes_read = (mylib_read_text (f,(byte *)&settings,sizeof settings));
    DBG_INI(dout<<"settings load bytes_read      = "<<bytes_read<<endl);
    DBG_INI(dout<<"settings load write_eof       = "<<settings.write_eof<<endl);
    DBG_INI(dout<<"settings load numlock_on      = "<<settings.numlock_on<<endl);
    DBG_INI(dout<<"settings load help_path       = "<<settings.help_path<<endl);
    DBG_INI(dout<<"settings load shell_command.s = "<<settings.shell_command.s<<endl);
    exit:
    mylib_close (f);

    if (strlen(name)>4 && name[1]==':' && name[2]=='\\')          /* OK */
       strcpy (setting_filename, name);
    else {                                    /* name should be normalized */
       strcpy (setting_filename, cwd);
       strcat (setting_filename, name);
       }

    DBG_INI(dout<<" setting.c settings load from cur dir bytes_read = "<<bytes_read<<endl);
    DBG_INI(dout<<"           SETTINGS  LOADED           "<<endl);
    debug_current_settings();
    ok = (bytes_read > 0) ? TRUE : FALSE;
return ok;
}


/*************************************************************************/
bool setting_save (char *name)
{                                 /* save setting file as named. */
  bool ok;
  mylib_file_t f;
  DBG_INI(dout<<"       SETTINGS struct SAVING "<<endl);
  debug_current_settings();

  if (! mylib_create (name, &f)) {
      DBG_INI(dout<<"settings save failed "<<endl);
      return FALSE;
  }
  ok = (mylib_write_text(f,(byte *)settings_header,(ULONG)strlen(settings_header)+1) != 0);
  if (ok) {
    DBG_INI(dout<<"settings save SUCCESS "<<endl);
    DBG_INI(dout<<"settings save write_eof       = "<<settings.write_eof<<endl);
    DBG_INI(dout<<"settings save numlock_on      = "<<settings.numlock_on<<endl);
    DBG_INI(dout<<"settings save shell_command.s = °"<<settings.shell_command.s<<"°"<<endl);
    DBG_INI(dout<<"settings save writing  "<<(sizeof settings)<<" bytes"<<endl);
    ok= (mylib_write_text (f, (byte *)&settings, sizeof settings) != 0);
  }
  mylib_close (f);
return ok;
}

void debug_current_settings(void)
{
#ifdef INI_DBG
    DBG_INI(dout<<"settings struct in memory "<<endl);
    DBG_INI(dout<<"view_defaults "<<endl);
    for(int a = 0; a < LOCATION_COUNT; a++) {
       DBG_INI(dout<<"location "<<a<<"  x = "<<settings.location[a].x<<" y = "<<settings.location[a].y<<endl);
    }
                        //   struct { SHORT x,y; } location[LOCATION_COUNT];
//    for(int b = 0; b < COLORSET_COUNT; b++) {
//       DBG_INI(dout<<"color_set "<<b<<" colors = ");
//       for(int c = 0; c < 6; c++) {
//          DBG_INI(dout<<settings.colors[b][c]);
//          DBG_INI(dout<<"  ");
//       }
//       DBG_INI(dout<<endl);
//    }

//   byte colors[COLORSET_COUNT][6];  /* 12 sets of 6 colors.  */
//   struct {                        /* viewer descript, used by new_view() */

      DBG_INI(dout<<" more view_defaults "<<endl);
      DBG_INI(dout<<" toplines = "<<settings.viewer_defaults.toplines<<" bottomlines = "<<settings.viewer_defaults.bottomlines<<endl);
      DBG_INI(dout<<" flags = 0x"<<hex<<settings.viewer_defaults.flags<<dec<<endl);
      DBG_INI(dout<<" vert_scroll_margin = "<<settings.viewer_defaults.vert_scroll_margin<<endl);
      DBG_INI(dout<<" horz_scroll_margin = "<<settings.viewer_defaults.horz_scroll_margin<<endl);
      DBG_INI(dout<<" view_page_mode = "<<settings.viewer_defaults.view_page_mode<<endl);        
      DBG_INI(dout<<" block_mode = "<<settings.viewer_defaults.block_mode<<endl);
      DBG_INI(dout<<" color set = "<<settings.viewer_defaults.color<<endl);
      DBG_INI(dout<<" tab_size = "<<settings.viewer_defaults.tab_size<<endl);
      DBG_INI(dout<<" RightMargin = "<<settings.viewer_defaults.RightMargin<<endl);
      DBG_INI(dout<<" QuotePrefix = °"<<settings.viewer_defaults.QuotePrefix<<"°"<<endl);

      DBG_INI(dout<<"backup_path = °"<< settings.backup_path<<"°"<<endl);
      DBG_INI(dout<<"tempfile_path = °"<< settings.tempfile_path<<"°"<<endl);
      DBG_INI(dout<<"help_path = °"<< settings.help_path<<"°"<<endl);
      DBG_INI(dout<<"flags = 0x"<<hex<< settings.flags<<dec<<endl);
      DBG_INI(dout<<" write_eof = "<<settings.write_eof<<endl);
      DBG_INI(dout<<" numlock_on = "<<settings.numlock_on<<endl);
      DBG_INI(dout<<"  extension = °"<<settings.default_ext<<"°"<<endl);
//      for(int d = 0; d < 26; d++) {
//         DBG_INI(dout<<"zoom location "<<d<<"  ");
//         DBG_INI(dout<<settings.zoom_defaults.position[d].h<<"  ");
//         DBG_INI(dout<<settings.zoom_defaults.position[d].w<<"  ");
//         DBG_INI(dout<<settings.zoom_defaults.position[d].x<<"  ");
//         DBG_INI(dout<<settings.zoom_defaults.position[d].y<<"  ");
//         DBG_INI(dout<<endl);
//      }

#endif
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*   resume file support                    */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

#ifdef PACKUP                           /* NO packup for now 6-91  */
SHORT setting_packup (byte  * buf)
{
memcpy (buf, setting_filename, sizeof setting_filename);
memcpy (buf + sizeof setting_filename, &settings, sizeof settings);
return sizeof setting_filename + sizeof settings;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

SHORT setting_unpack (byte  * buf)
{
memcpy (setting_filename, buf, sizeof setting_filename);
memcpy (&settings, buf + sizeof setting_filename, sizeof settings);
return sizeof setting_filename + sizeof settings;
}

#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */


