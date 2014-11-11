/*****************************************************
file: VIEW_FIL.C  Copyright 1989 by Dlugosz Software
   view functions for saving and loading files
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "dirlist.h"
#include "status.h"
#include "mylib.h"
#include "askfile.h"
#include "setting.h"   /* needed for load/save prompt boxes */
#include "edx.h"       /* need prompt_box() */
#include "redbox.h"
#include <string.h>    /* need strcpy() */
#include "advfile.h"

#include "test.h"

#include "basewin.h"

#define locus 2

bool really_exit = FALSE;      // extern global, for ked_exit

/*****************************************************************
    returns TRUE if no unsaved changes, ok to quit
            FALSE if unsaved changes, do not quit
*******************************************************************/
bool view_dont_throw_away (vptr view_v)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
struct sheet *cur= (struct sheet *)Mem::vmem_trans (v->sheet);
bool retcode = FALSE;

if (cur->flags & 2)               // OK to discard this sheet
   return TRUE;                 
if (cur->changed)  {
   enum err_choices e;
   e= redbox_s (locus, 100, eh_yes|eh_no|eh_cancel, cur->filename);
   switch(e) {
      case eh_yes:
//                  if(cur->filename[0] == '\0') {
                     retcode = view_save_file(cwd,"");
                     return retcode;
//                  }
//                  else {
//                     retcode = view_save_file(cwd, "");
//                  }
//                  if(retcode == TRUE)
//                     return TRUE;
//                  else
//                     return FALSE;
      case eh_cancel:                 // fageddaboutit
                  return FALSE;
      case eh_no:                     // no, don't save
                  return TRUE;
   }
   }
return TRUE;
}

/* **************************************** */

bool view_assure_all_saved (void)
{
vptr view_v= top_view, next;
struct view_rec *v;
SHORT loop= viewer_count;

while (loop--) {
   v= (struct view_rec *)Mem::vmem_trans (view_v);
   next= v->under;
   if (!view_dont_throw_away (view_v)) 
      return FALSE;
   view_v= next;
   }
return TRUE;
}

/* **************************************** */
#ifdef FUCKUP
bool view_assure_all_saved2 (bool dontcare)
{
vptr view_v= top_view, next;
struct view_rec *v;
SHORT loop= viewer_count;

if(!dontcare) {
while (loop--) {
   v= (struct view_rec *)Mem::vmem_trans (view_v);
   next= v->under;
   if (!view_dont_throw_away (view_v)) 
      return FALSE;
   view_v= next;
   }
}
return TRUE;
}
#endif

//#ifdef FUCKUP                     // not for v 2.15
void view_save_all(void)
{
vptr view_v= top_view, next;
struct view_rec *v;
SHORT loop= viewer_count;
struct sheet *cur = 0;
SHORT retcode = 0;           // 0 ok, 1 cannot create, 2 error writing

while (loop--) {
   v= (struct view_rec *)Mem::vmem_trans (view_v);
   cur= (struct sheet *)Mem::vmem_trans (v->sheet);
   next= v->under;
   if (cur->changed) {
      if(!cur->filename) {
         redbox_s (locus, 203, eh_cancel, cur->filename);          
      }
      retcode = sheet_save(v->sheet, "");
      if(retcode) {
         switch(retcode) {
               case 1:
                      redbox_s (locus, 200, eh_cancel, cur->filename);
                      break;
               case 2:
                      redbox_s (locus, 201, eh_cancel, cur->filename);
                      break;
         }
      }
   }
   view_v= next;
 }
}
//#endif

/* *********************************************************************
The view_load_file() function uses the Dirname system to process
the file name prior to loading.  The Dirname system is divided
into 3 source files.  The first, DIRNAME.C, manipulates file
names.  It includes functions to normalize a filename and check
if a name is wild (see DIRLIST.H).

The normalize_filename() function takes a name, a default
directory, and a default extension.  The name is converted to all
caps, and made into an absolute path name using the drive and
path from the default path if needed.  The default extension is
used if none is in the original.  The final result replaces the
original in the first parameter.

You can see how the view_load_file() function uses this.  The
parameters are passed to normalize the filename before loading. 
If the result is wild, another function is called.  You can
examine view_load_file() in VIEW_FIL.C and see how the Dirname
system calls are used.
**************************************************************************/

bool view_load_file (char const* name, char const* default_dir, char const* default_ext)
{
char fname[FNAME_LEN], default_name[FNAME_LEN], 
     my_default_ext[FNAME_LEN], my_org_filename[FNAME_LEN];
bool first_time = TRUE;
struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct view_rec *v = (struct view_rec *)Mem::vmem_trans (active_view);

//struct sheet *cur = my_active_sheet;         // mod 8-27-93

memset(fname,0x00, FNAME_LEN);
memset(default_name,0x00, FNAME_LEN);
memset(my_default_ext,0x00, FNAME_LEN);
memset(my_org_filename,0x00, FNAME_LEN);

   DBG_LOAD(dout<<" view_load_file entry name        = °"<<name<<"°"<<endl);
   DBG_LOAD(dout<<" view_load_file entry default_dir = °"<<default_name<<"°"<<endl);
   DBG_EXT(dout<<" view_load_file  entry default_ext = °"<<default_ext<<"°"<<endl);

if(default_ext) 
   strcpy(my_default_ext, default_ext);

if(name)
   strcpy(my_org_filename, name);

if (!view_dont_throw_away (active_view)) 
   return FALSE;
if (name) {                         // if name, no prompt box
   strcpy (fname, name);
   default_name[0]= '\0';
   }
else  {                                          // name == NULL
//   strcpy (default_name, cur->load_filename);
   strcpy (default_name, "*.*");
}
   DBG_LOAD(dout<<" view_load_file fname = "<<fname<<endl);
   DBG_LOAD(dout<<" view_load_file my_org_name = °"<<my_org_filename<<"°"<<endl);
   DBG_LOAD(dout<<" view_load_file name = "<<name<<endl);
   DBG_LOAD(dout<<" view_load_file default_name = °"<<default_name<<"°"<<endl);
   DBG_EXT(dout<<" view_load_file extension = °"<<default_ext<<"°"<<endl);

try_load_again:
   DBG_LOAD(dout<<" view_load_file my_org_name = °"<<my_org_filename<<"°"<<endl);
if (get_load_name (default_name, fname, default_dir, my_default_ext)) {
   DBG_LOAD(dout<<" view_load_file got fname = "<<fname<<endl);

                                   /* store modified default back in sheet */
   strcpy (cur->load_filename, default_name ? default_name : fname);
   if(*default_name != '\0')
       strcpy(my_org_filename, default_name); // do I really want this ??
                                                 /* proceed with load */
   sheet_clear (active_sheet);                   /* out with the old */
   if (sheet_load (active_sheet, fname, TRUE)) { /* in with the new */
      DBG_LOAD(dout<<" view_load_file sheet_load TRUE for fname = "<<fname<<endl);
      #ifdef WORD_WRAP
      if(v->flags & 32) {                           /* word_wrap mode */      
         reform_doc_down(0);                 //word wrap whole file
      }
      #endif
      status_redraw();
      return TRUE;
   }
   else {                     // unable to load file (sheet_load = FALSE)
       DBG_LOAD(dout<<" view_load_file sheet_load FALSE for fname = "<<fname<<endl);
       if((my_default_ext != "") && (first_time == TRUE)) {
          DBG_LOAD(dout<<" view_load_file def_ext failed, try again def_ext = 0 "<<endl);
          memset(my_default_ext,0x00, FNAME_LEN);
          memset(default_name,0x00, FNAME_LEN);
          memset(fname,0x00, FNAME_LEN);
          strcpy (fname, my_org_filename);
          DBG_LOAD(dout<<" view_load_file NOW fname = "<<fname<<endl);
          first_time = FALSE;
          goto try_load_again;
       }
       if(crit_error == 0){
         cur->line_count= 1;         /* sheet has 0 lines.  put one in */
         put_line_anysheet (cur, 0, zero_line());
         status_redraw();
         return TRUE;
       }
   }
}
return FALSE;
}

/* **************************************** */

static bool  save_and_report (char *name)
{                                   /* sheet save with error checking */
SHORT err;
   DBG_FILE(dout<<"save_and_report  name = "<<name<<endl);
  err = sheet_save (active_sheet, name);
if (err != 0) {
  DBG_FILE(dout<<"save_and_report err = "<<err<<endl);
   switch (err) {
      case 1: redbox_s (locus, 200, eh_cancel, name);  break;
      case 2: redbox_s (locus, 201, eh_cancel, name);  break;
      }
   return FALSE;
   }
return TRUE;
}

/*********************************************************************
   this adds functionality over sheet_save.
   It saves as a different name, then renames after save is OK.
   global setting flags:  1-autobackup  2-safe save  
   PSEUDO CODE: 
        if safe_save ON
           save "foo.doc" as "foo.S$"
           rename "foo.s$" to "foo.doc"
           if autobackup ON
               copy "foo.doc" to "foo.bak"
        else
           if autobackup ON
               copy "foo.doc" to "foo.bak"
           else
               save "foo.doc"
*******************************************************************/
static bool  safe_save (char *fname)
{
char backupname[FNAME_LEN];
DBG_FILE(dout<<"safe_save line "<<__LINE__<<" passed fname = °"<<fname<<"°"<<endl);
if (settings.flags & 2) {        /* safe save */
   char myname[FNAME_LEN];
   delta_filename (myname, fname, settings.tempfile_path);
   DBG_FILE(dout<<"safe_save line "<<__LINE__<<" myname = "<<myname<<" fname = "<<fname<<endl);
   if (!save_and_report(myname)) {
       return FALSE;
   }  
   else {
      DBG_FILE(dout<<"safe_save line "<<__LINE__<<" myname = "<<myname<<" fname = "<<fname<<endl);
      rename_file (myname, fname);   
   }
   if (settings.flags & 1) {         // autobackup = TRUE
      delta_filename (backupname, fname, settings.backup_path);
      DBG_FILE(dout<<"safe_save line "<<__LINE__<<" fname = "<<fname<<" backupname = "<<backupname<<endl);
//      if (!rename_file (myname, backupname)) {
      if (!copy_file (fname, backupname)) {         // was myname
         redbox_s(locus,204,eh_continue,backupname); // report err
      }
//   rename_file (backupname, fname);
   }
   else {                       // no autobackup
//     mylib_delete (fname);
   DBG_FILE(dout<<"safe_save line "<<__LINE__<<" myname = "<<myname<<" fname = "<<fname<<endl);
//   rename_file (myname, fname);
   }
 }
else {                       // safe_save OFF
   if (settings.flags & 1) {                      // autobackup TRUE
      delta_filename (backupname, fname, settings.backup_path);
      DBG_FILE(dout<<"safe_save line "<<__LINE__<<" fname = "<<fname<<" backupname = "<<backupname<<endl);
//      if (!rename_file (fname, backupname)) {           /* report this */
      if (!save_and_report(fname)) {            // save original
         DBG_FILE(dout<<"safe_save fail "<<__LINE__<<" fname = "<<fname<<endl);
         if (redbox_s(locus,204,eh_continue|eh_cancel,fname) == eh_cancel)
            return FALSE;
      }
//   }
   if (!save_and_report(backupname)) {    // save original as "backupname" 
      DBG_FILE(dout<<"safe_save line "<<__LINE__<<" fname = "<<fname<<" backupname = "<<backupname<<endl);
      if (settings.flags & 1) {
         copy_file (backupname, fname);
         return FALSE;
      }
   }
   }
   else {                      // safe_save OFF, autobackup OFF
      if (!save_and_report(fname)) {            // save original
         DBG_FILE(dout<<"safe_save fail "<<__LINE__<<" fname = "<<fname<<endl);
         if (redbox_s(locus,204,eh_continue|eh_cancel,fname) == eh_cancel)
            return FALSE;
      }
   }
 }
return TRUE;
}

/* **************************************** */
bool view_resave_file(void)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur = my_active_sheet;     // mod 8-27-93 
if (cur->filename[0] == '\0') {
   if(view_save_file(cwd, ""))
     return TRUE;
   else
//   redbox (locus, 203, eh_cancel);
   return FALSE;
}
else
return safe_save (cur->filename);
}

/*****************************************************************   
   I "fixed that, so I need a new function for "save_as"
******************************************************************/
//bool view_save_file_as(char const* default_dir)
//{
//char fname[FNAME_LEN];
//memset(fname, 0x00, FNAME_LEN);


//}
/*****************************************************************
   original would prompt for filename if used as "save_as"
   I "fixed that, so I need a new function for "save_as"
******************************************************************/
bool view_save_file (char const* default_dir, char const* default_ext)
{
char fname[FNAME_LEN];
memset(fname, 0x00, FNAME_LEN);

struct sheet *cur = (struct sheet *)Mem::vmem_trans (active_sheet);
if(cur->filename[0] == '\0') {
   if (! get_save_name (fname, default_dir, default_ext)) {
       DBG_FILE(dout<<"view_save_file line "<<__LINE__<<" fname = "<<fname<<endl);
       return 1;
   }                          
   strcpy (cur->filename, fname);       /* save the name back in the sheet */
}                                      
else {
   strcpy (fname, cur->filename);
   if (! get_save_name (fname, default_dir, default_ext)) {
       DBG_FILE(dout<<"view_save_file line "<<__LINE__<<" fname = "<<fname<<endl);
       return 1;
   }                          
   strcpy (cur->filename, fname);       /* save the name back in the sheet */
}

DBG_FILE(dout<<"view_save_file line "<<__LINE__<<" fname = °"<<fname<<"°  cur->filename = °"<<(cur->filename)<<"°"<<endl);
status_name();
return safe_save (fname);
}

/* **************************************** */
