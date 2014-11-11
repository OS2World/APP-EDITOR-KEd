/*****************************************************
File: ADVFILE.C  Copyright 1989,90 by Dlugosz Software
   Advanced high-level disk functions

    High-level copy/rename/delete functions  
    All functions return 0 if error, 1 if ok 
*****************************************************/
#include "usual.h"
#include "mylib.h"
#include "redbox.h"
#include "dirlist.h"
#include <string.h>

#define locus 3

/* **************************************** */
//static bool prim_file_copy(const char *src,const char *dest, bool allow_abort)
static bool prim_file_copy(const char *src,const char *dest)
{
bool result= FALSE;

result = mylib_copy(src, dest);

#ifdef FUCKUP
ULONG filesize;
USHORT size;
vptr page= vmem_newpage ();
unsigned char *buf= (unsigned char*)Mem::vmem_trans (page);
mylib_file_t src_handle, dest_handle= -1;
struct dir_entry entry;

//dos_error= 0; /* clear any previous error */
if (!mylib_open (src,&src_handle,MYLIB_READONLY) || !mylib_create (dest,&dest_handle))
   goto exit_copy;
if (!file_findfirst (src,&entry,1+2+4))
   goto exit_copy;
filesize= entry.size;
while (0 != (size= mylib_read_text (src_handle,buf,VMEM_PAGESIZE))) {
   if (allow_abort && (key_ready () && getkey () == kESC))
      goto exit_copy;
   if (!mylib_write_text (dest_handle,buf,size)) {
      goto exit_copy;
      }
   filesize-= size;
   }
if (filesize == 0) result= TRUE;

exit_copy:
// dos_error_report();                         // not used for OS/2 ver
//if (src_handle != -1)    // org
if (src_handle != 0) 
   mylib_close (src_handle);
//if (dest_handle != -1) {
if (dest_handle != 0) {
   if (result) mylib_setdate (dest_handle,entry.date,entry.time);
   mylib_close (dest_handle);
   if (!result) mylib_delete (dest);  /* erase copy if error */
   }
vmem_droppage (page);
#endif

return result;
}

/* ********************************************************************
           Copy file from Src to Dest filenames 
           check that src and dest are different names
********************************************************************/
bool copy_file (char *src, char *dest)
{
if (strcmp (src,dest) == 0) {
   redbox_s (locus,201,eh_abort,src);
   return FALSE;
   }
//return prim_file_copy (src,dest,TRUE);
return prim_file_copy (src,dest);
}

/****************************** **************************************** */
                                      // Delete file or directory from src 
bool delete_file (char *src)
{
if (!mylib_delete (src))             /* attempt to delete as a file */
    return FALSE;
//   return mylib_removedir (src);     /* then try to delete as a directory */
else return TRUE;
}

/* ***************************************************************** */
/* Rename File from Src to Dest filename,
                          across drives or directories if necessary */
bool rename_file (char *src, char *dest)
{
if (mylib_rename (src,dest))
     return TRUE;                       /* try to rename file */
                                        /* else the long way */
//if (!prim_file_copy (src,dest,FALSE))
if (!prim_file_copy (src,dest))
     return FALSE;                     /* try to copy it to destination */
return delete_file (src);              /* and delete the original */
}

/* ***************************************************************** */
bool file_exists (char *name)
{
struct dir_entry entry;

return file_findfirst (name,&entry,1+2+4);
}

