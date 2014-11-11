/*****************************************************
file: ASKFILE.C     Copyright 1989 by Dlugosz Software
   load/save prompt boxes
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "dirlist.h" /* need dirname functions */
#include "edx.h"     /* need prompt_box() */
#include <string.h>  /* need strcpy() */

#include "basewin.h"
#include "test.h"

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool get_load_name (char *default_name, char *fname, char const *default_dir, char const *default_ext)
{
   DBG_LOAD(dout<<" get_load_name fname = °"<<fname<<"°"<<endl);
   DBG_LOAD(dout<<" get_load_name default_name = °"<<default_name<<"°"<<endl);
   DBG_EXT(dout<<" get_load_name extension = °"<<default_ext<<"°"<<endl);
do {
   if(*default_name != '\0') {
      DBG_LOAD(dout<<" get_load_name default_name = °"<<default_name<<"°"<<endl);
      DBG_LOAD(dout<<" get_load_name fname = "<<fname<<endl);
      if (!prompt_box("Load:", default_name, 79, NULL, 1|64, 0)) //last param was 1
          return FALSE;                                   // user pressed ESC
      strcpy (fname, default_name);
   }
   else
      strcpy (default_name, fname);     

   if (default_dir) 
       normalize_filename_2 (fname, default_dir, default_ext);
                                             /* take care of wildcard names */
   DBG_LOAD(dout<<" get_load_name line "<<__LINE__<<" fname = °"<<fname<<"°"<<endl);
   DBG_LOAD(dout<<" get_load_name line "<<__LINE__<<" default_name = °"<<default_name<<"°"<<endl);
   DBG_EXT(dout<<" get_load_name extension = °"<<default_ext<<"°"<<endl);
} while (filename_iswild(fname) && ! dirlist(" LOAD ",fname,
                                     SHOWALL+PATHWINDOW,4,3,Screen.get_screen_height()-1));

return TRUE;                            
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool get_save_name (char *fname, char const* default_dir, char const* default_ext)
{
char fname2[FNAME_LEN], fname3[FNAME_LEN];

strcpy (fname3, fname);
strcpy (fname2, fname);

if (!prompt_box("Save:", fname3, 79, NULL, 64|1, 2)) 
     return FALSE;                       /* changed his mind */
                                    /* now figure out what he really meant */
if (!*fname2) {                          /* does not have a name yet */
   strcpy (fname, fname3);
//   if (default_dir) normalize_filename (fname, default_dir, default_ext);
   if (default_dir) 
      normalize_filename_2 (fname, default_dir, default_ext);
   }
else {
   if (fname3[0] == '.') {            /* override path with default path */
      strcpy (fname2, default_dir);
      strcat (fname2, find_basename(fname));
      }
   if(strcmp(fname, fname3) != 0) {    // bug in delta_filename, 
      delta_filename (fname, fname2, fname3);
      }
   }
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */



