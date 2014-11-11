/*****************************************************
File: MYLIB.C    Copyright 1989,90 by Dlugosz Software
   useful functions that may be ported or optomized
   for each compiler.

      for File class, need to test these uses:

   open file, no name
   save file, no name  (prompt for name)
   open file, name
   save file, name
   save file, new name (save as)
   save file, create backup 
   rename file

   open file, no extension
   open file, no extension w/dot
   open file, default extension (s)

*****************************************************/
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "redbox.h"
#include "mylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>        /* need strcpy  */
#include <memory.h>        /* memcpy   */
#include "err_hand.h"      // set_int24()

#include "test.h"
                                               /* look at OS/2 error codes */
//static bool examine_error(ULONG error_code, const char *name); 

// char file_EOF_char= '\x1a';
//static bool file_write_eof= FALSE;

SHORT crit_error= 0;
//static HDIR hdir = 1;                   // handle to use current dir (SYSTEM) 
//static FILEFINDBUF3 findbuf;             
//static ULONG      srchcount = 1;      
static HFILE       fhandle;
static FILESTATUS  fstatus;
static USHORT MAX_NAME_LEN = 0;

#ifdef FILE_CLASS
//////////////////////////////////////////////////////////////////////////
TheFile::TheFile()
{
  _File        = 0;
   _name[0]    = '\0';
  _is_loaded   = FALSE;
  critical_err = 0;
//  _size        = 0;
  pos          = 0;
  _binary_mode = FALSE;
//  _internal_buf[0] = '\0';
//  _level       = 0;
}


/*------------------------------------------------------------------
     Construct and open a named file
     probably want to use mylib_open() routine here.
*-----------------------------------------------------------------*/
TheFile::TheFile(const char* filename)
{
//   _is_loaded = mylib_open(filename, &_File,MYLIB_READONLY|MYLIB_WRITEONLY);

   critical_err = 0;
   pos          = 0;
  _binary_mode  = FALSE;
//  _internal_buf[0] = '\0';
//  _level        = 0;

   _is_loaded = Open(filename,READONLY|WRITEONLY);
   strcpy(_name, filename);

}


//----------------------------------------------------------------
bool TheFile::Open(const char *filename, SHORT access)
{
ULONG  action   = 0x01L;
ULONG  openmode = OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR;

//if (access == (MYLIB_READONLY|MYLIB_WRITEONLY))
//     openmode = OPEN_ACCESS_READWRITE | open_default;
//else if (access == MYLIB_READONLY)
//    openmode = OPEN_ACCESS_READONLY | open_default;
//else if (access == MYLIB_WRITEONLY)
//    openmode = OPEN_ACCESS_WRITEONLY | open_default;

       DBG_FILE(dout<<"TheFile::Open access   = 0x"<<hex<<access<<dec<<endl);
  openmode |= access;
       DBG_FILE(dout<<"TheFile::Open openmode = 0x"<<hex<<openmode<<dec<<endl);

retry:
   rc = DosOpen((PSZ)filename,
                &fhandle,
                &action,
                0L,         // size
                0L,         // attrib
                FILE_OPEN,  // flags =  fail if file not exist
                openmode,
                0L);
    if(rc == 0) {
       _File = fhandle;
       _is_loaded = TRUE;
       DBG_FILE(dout<<"Open file Success rc = "<<rc<<" ked file handle = "<<_File<<" os2 file handle = "<<fhandle<<endl);
//       _internal_buf = new unsigned char(_BUF_SIZE);
               // no error checking on "new"......
//       _level        = 0;       // buf internal ptr
    }
    else {                //if(rc != 0){
      DBG_FILE(dout<<"Open file ERR rc = "<<rc<<" ked file handle = "<<_File<<" os2 file handle = "<<fhandle<<endl);
      if(examine_error(rc, filename) == TRUE) 
         goto retry;
      else {
         _File = 0;
         return  FALSE;
      }
    }

return TRUE;      //return (*file != -1);
}

//------------------------------------------------------------------
bool TheFile::Create (const char *filename)
{
    ULONG ulAction;
retry:
                                          // shareable read/ no write
   rc = DosOpen((PSZ)filename, &fhandle, &ulAction,
                0L,                        // initial size
                FILE_NORMAL,               // attribute
                FILE_OPEN | FILE_CREATE,   // open existing file or create
                OPEN_FLAGS_WRITE_THROUGH | // flush writes to disk
                OPEN_ACCESS_READWRITE |
                OPEN_SHARE_DENYWRITE,
                0L);
   if(rc == 0) {
      _File = fhandle;
       DBG_FILE(dout<<"mylib_create rc = "<<rc<<" ked file handle = "<<_File<<" os2 file handle = "<<fhandle<<" name = °"<<filename<<"°"<<endl);
   }
   else {
      if(examine_error(rc, filename) == TRUE) {
        goto retry;
      }
      else {
         _File = 0;
         return  FALSE;
      }
   }
return TRUE;              //return (*file != -1);
}


//-------------------------------------------------------------------
bool TheFile::Delete (const char *filename)
{
retry:
  rc = DosDelete((PSZ)filename);
  DBG_FILE(dout<<"mylib_delete rc = "<<rc<<" filename = "<<filename<<endl);
   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
        return  FALSE;
   }
return TRUE;
}

//-----------------------------------------------------------------
ULONG TheFile::Size()
{

retry:
  rc = DosQFileInfo(_File, 0x0001, &fstatus, sizeof(fstatus));

   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
     else
        return 0;
return (fstatus.cbFile);
}

//----------------------------------------------------------------------
ULONG TheFile::Tell()   // return curr position in file used only by HYPRHELP
{
retry:
  rc = DosChgFilePtr(_File, 0L, FILE_CURRENT, &pos);

   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
        return 0;
   }
return pos;
}

//--------------------------------------------------------------------
bool TheFile::Seek (ULONG filepos,SHORT whence)
{
   ULONG newpos;
retry:
   rc = DosChgFilePtr(_File, filepos, whence, &newpos);
   if(rc != 0) {                                     // (rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
      return FALSE;
    }
return TRUE;
}

//-------------------------------------------------------------------
USHORT TheFile::Read(UCHAR* buf,ULONG size)
{
ULONG sizeread;

retry:
   rc = DosRead(_File, buf, size, &sizeread);
   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
        return  0;
   }
return (USHORT)sizeread;
}

//-------------------------------------------------------------------
bool TheFile::Write(UCHAR* buf,ULONG size)
{
  ULONG sizewritten;

retry:
    rc = DosWrite(_File, buf, size, &sizewritten);

   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
        return  FALSE;
   }
return sizewritten == size;
}

//-----------------------------------------------------------------
bool TheFile::Close ()
{
retry:
   rc = DosResetBuffer(_File);         // PROBABLY DON'T NEED THIS.....
   DBG_FILE(dout<<"mylib_close rc = "<<rc<<" file handle = "<<_File<<endl);

   rc = DosClose(_File);
   DBG_FILE(dout<<"mylib_close rc = "<<rc<<" file handle = "<<_File<<endl);
   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
      else
        return  FALSE;
   }
   delete _internal_buf;  // buffered file mod 3/96
return TRUE;
}

/*-------------------------------------------------------------------
     ReadLine   
         receive passed String ptr and maxlen for that str.
         read a line from an open file, return a null-terminated
         string. Strip any CR, LF or any combination, if Binary
         flag not set in Class.

         manages read-ahead buffer. (complex part)

---------------------------------------------------------------------*/
UCHAR* TheFile::ReadLine(UCHAR* str, int maxlen)
{
ULONG sizeread;
UCHAR *P, *eolp;
int   Chunk;

#ifdef FUCKUP

   P = str;

  if(_level != 0) {      // some data not processed in buf
     if(maxlen > _level)
        Chunk = _level;
     else
        Chunk = maxlen;

     if(eolp = memchr(

retry:
   sizeread = Read(&_internal_buf, _BUF_SIZE);

#endif

return sizeread;
}


//--------------------------------------------------------------------
/* called only by HELP.C and Hyprhelp.c            */
bool TheFile::RandomRead(ULONG filepos,void* buf,USHORT size)
{
    ULONG   sizeread;
    ULONG   newpos;

retry:
     rc = DosChgFilePtr(_File, filepos, FILE_BEGIN, &newpos);
       if (rc != 0) goto err;
     rc = DosRead(_File, buf, (ULONG)size, &sizeread);
       if (rc != 0) goto err;

     return TRUE;

err:
   if(rc != 0) {
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;
   }
#pragma warn -rvl
}
#pragma warn +rvl


/*************************************************************
   this is called only by ADVFILE.CPP rename_file(), and the 
   correct behavior in this case is to just report the failure
   back to upper layers.
   future extensions (ie: file mgr) may require other behavior
********************************************************************/
bool TheFile::Rename (CHAR *old, CHAR *my_new)
{
  rc = DosMove((PSZ)old, (PSZ)my_new);
  DBG_FILE(dout<<"mylib_rename rc = "<<rc<<" old = °"<<old<<"°  new = °"<<my_new<<"°"<<endl);
  if(rc != 0)
     return  FALSE;

return TRUE;
}

/********************************************************************/
bool TheFile::Copy (const char *old, const char *my_new)
{
retry:
  rc = DosCopy((PSZ)old, (PSZ)my_new, DCPY_EXISTING);
  DBG_FILE(dout<<"mylib_copy rc = "<<rc<<" old = °"<<old<<"° new = °"<<my_new<<"°"<<endl);

   if(rc != 0) {
      if(examine_error(rc, my_new) == TRUE)
        goto retry;
   else
      return  FALSE;
   }

return TRUE;
}

#endif   // FILE_CLASS
//
///////////////////////////////////////////////////////////////////////////

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     file functions                       */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool filesys_init(void)
{                     // query OS2 for FAT or HPFS info, if HPFS set 
                      // FNAME_LEN accordingly, and use HPFS filename
                      // parsing routine
 ULONG   StartIndex;    /* Ordinal of 1st variable to return */
//  23.QSV_MAX_COMP_LENGTH:  
//     Maximum length, in bytes, of one component in a path name. 

 ULONG   LastIndex = StartIndex = 23; /* Ordinal of last variable to return */
 USHORT   DataBuf[2];        /* System information (returned) */
 ULONG   DataBufLen = 5;    /* Data buffer size */

 rc = DosQuerySysInfo(StartIndex, LastIndex, DataBuf,DataBufLen);
 if(rc == 0) {
    MAX_NAME_LEN = DataBuf[0];    // always returns 255
    return TRUE;
 }
 else
   return FALSE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_create (char const *filename, mylib_file_t *file)
{
    ULONG ulAction;
retry:
                                          // shareable read/ no write
   rc = DosOpen((PSZ)filename, &fhandle, &ulAction,
                0L,                         // initial size
                FILE_NORMAL,               // attribute
                FILE_OPEN | FILE_CREATE,  // open existing file or create
                OPEN_FLAGS_WRITE_THROUGH |  // flush writes to disk
                OPEN_ACCESS_READWRITE |
                OPEN_SHARE_DENYWRITE,
                0L);
   if(rc == 0) {
      *file = fhandle;
       DBG_FILE(dout<<"mylib_create rc = "<<rc<<" ked file handle = "<<*file<<" os2 file handle = "<<fhandle<<" name = °"<<filename<<"°"<<endl);
   }
   if(rc != 0){
 //     printf(" error creating file %u\t%s\n",rc,filename);
  //    DosSleep(1000L);
      if(examine_error(rc, filename) == TRUE)
        goto retry;
   else {
      *file = 0;
      return  FALSE;
   }
   }
return TRUE;              //return (*file != -1);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_open (char const *filename, mylib_file_t *file,SHORT access)
{
ULONG action = 0x01L;
ULONG  attrib = 0L;
ULONG  size = 0L;
ULONG  flags = FILE_OPEN;
ULONG  open_default = OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR;
ULONG  openmode = open_default;
//#define EABUF 0L;

if (access == (MYLIB_READONLY|MYLIB_WRITEONLY))
     openmode = OPEN_ACCESS_READWRITE | open_default;
else if (access == MYLIB_READONLY)
    openmode = OPEN_ACCESS_READONLY | open_default;
else if (access == MYLIB_WRITEONLY)
    openmode = OPEN_ACCESS_WRITEONLY | open_default;

retry:
   rc = DosOpen((PSZ)filename,
                &fhandle,
                &action,
                size,
                attrib,
                flags,                         // fail if file not exist
                openmode,
                0L);
    if(rc == 0) {
       *file = fhandle;
       DBG_FILE(dout<<"mylib_open rc = "<<rc<<" ked file handle = "<<*file<<" os2 file handle = "<<fhandle<<endl);
    }
   if(rc != 0){
    //  printf(" error opening file %u\t%s\n",rc,filename);
    //  DosSleep(1000L);
      if(examine_error(rc, filename) == TRUE)
        goto retry;
   else {
      *file = 0;
      return  FALSE;
   }
   }

return TRUE;      //return (*file != -1);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_close (mylib_file_t file)
{
retry:
   rc = DosResetBuffer(file);
   DBG_FILE(dout<<"mylib_close rc = "<<rc<<" file handle = "<<file<<endl);
   if(rc != 0)
      examine_error(rc, "");

   rc = DosClose(file);
   DBG_FILE(dout<<"mylib_close rc = "<<rc<<" file handle = "<<file<<endl);
   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;

return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_seek (mylib_file_t file,ULONG filepos,SHORT whence)
{
   ULONG newpos;
   bool ret_val;
retry:
   rc = DosChgFilePtr(file, filepos, whence, &newpos);
   if (rc == 0)
        ret_val = TRUE;

   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      ret_val =  FALSE;
return ret_val;
}


#ifdef FUCK_UP
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_random_write(mylib_file_t file,ULONG filepos,void* buf,USHORT size)
{
    ULONG       byteswritten;
    ULONG        newpos;
    static ULONG filesize;
                /* this function only called by VMEM, and filepos is
                   always CURRENT_POS = 1, so it doesn't look like
                   DosChgFilePos is needed
                */
retry:
     if (filepos >= filesize) {
       rc = DosNewSize(file, filepos + size);     // don't increase size
       filesize = filepos + size;                 // if write pos not at end
       if (rc != 0) goto err;
      }
       rc=DosChgFilePtr(file, filepos, FILE_BEGIN, &newpos);
          if (rc != 0) goto err;
       rc = DosWrite(file, buf, (ULONG)size, &byteswritten);
          if (rc != 0) goto err;
         return TRUE;
err:
   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;
#pragma warn -rvl
}
#pragma warn +rvl
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/* called only by HELP.C and Hyprhelp.c            */
bool mylib_random_read(mylib_file_t file,ULONG filepos,void* buf,USHORT size)
{
    ULONG   sizeread;
    ULONG   newpos;

retry:
     rc = DosChgFilePtr(file, filepos, FILE_BEGIN, &newpos);
       if (rc != 0) goto err;
     rc = DosRead(file, buf, (ULONG)size, &sizeread);
       if (rc != 0) goto err;

     return TRUE;

err:
   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;
#pragma warn -rvl
}
#pragma warn +rvl


#ifdef FUCKUP
USHORT mylib_read_lines (mylib_file_t file,UCHAR* buf,ULONG size)
{
/*        returns 1 line of chars from file,
          terminates on 0D 0A or any combination
          DosRead is set for 1 char, I figure OS/2 will always
          buffer the file in cache, so for small files speed should
          not be a problem
*/
ULONG sizeread;
bool eol = FALSE;
unsigned char mychar;

retry:
  while(!eol) {
    rc = DosRead(file, mychar, 1, &sizeread);
    if(rc == 0) {
       if(mychar == 0x0D) {
          DosRead(file, mychar, 1, &sizeread);  // see if 0A next
          if(mychar == 0x0A) {
             // DosChgFilePtr()
          }
       }
    else {                       // rc != 0
//    if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
  }
 //  else
 //     return  FALSE;

return sizeread;
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

USHORT mylib_read_text (mylib_file_t file,UCHAR* buf,ULONG size)
{
ULONG sizeread;

retry:
    rc = DosRead(file, buf, size, &sizeread);
   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  0;

return (USHORT)sizeread;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_write_text (mylib_file_t file, unsigned char *buf,ULONG size)
{
  ULONG sizewritten;

retry:
    rc = DosWrite(file, buf, size, &sizewritten);

   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;

return sizewritten == size;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

ULONG mylib_filesize (mylib_file_t file)
{

retry:
  rc = DosQFileInfo(file, 0x0001, &fstatus, sizeof(fstatus));

   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;

return (fstatus.cbFile);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_delete (char const *filename)
{
retry:
  rc = DosDelete((PSZ)filename);
  DBG_FILE(dout<<"mylib_delete rc = "<<rc<<" filename = "<<filename<<endl);
   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;

return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

ULONG mylib_tell (mylib_file_t file)
{
ULONG pos;
                       /*     returns current position in file      */
retry:
  rc = DosChgFilePtr(file, 0L, FILE_CURRENT, &pos);

   if(rc != 0)
      if(examine_error(rc, "") == TRUE)
        goto retry;
   else
      return  FALSE;

return pos;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef FUCKUP
bool mylib_makedir (CHAR *dir)
{
retry:
   rc = DosMkDir((PSZ)dir, 0L);

   if(rc != 0)
      if(examine_error(rc, dir) == TRUE)
        goto retry;
   else
      return  FALSE;

return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool mylib_removedir (CHAR *dir)
{
retry:
  rc = DosRmDir((PSZ)dir);

   if(rc != 0)
      if(examine_error(rc, dir) == TRUE)
        goto retry;
   else
      return  FALSE;

return TRUE;
}
#endif
/*************************************************************
   this is called only by ADVFILE.CPP rename_file(), and the 
   correct behavior in this case is to just report the failure
   back to upper layers.
   future extensions (ie: file mgr) may require other behavior
********************************************************************/
bool mylib_rename (CHAR *old, CHAR *my_new)
{
//retry:
  rc = DosMove((PSZ)old, (PSZ)my_new);
  DBG_FILE(dout<<"mylib_rename rc = "<<rc<<" old = °"<<old<<"°  new = °"<<my_new<<"°"<<endl);
   if(rc != 0)
//      if(examine_error(rc, my_new) == TRUE)
//        goto retry;
//   else
      return  FALSE;

return TRUE;
}

/********************************************************************/
bool mylib_copy (const char *old, const char *my_new)
{
ULONG mode = DCPY_EXISTING;
retry:
  rc = DosCopy((PSZ)old, (PSZ)my_new, mode);
  DBG_FILE(dout<<"mylib_copy rc = "<<rc<<" old = °"<<old<<"° new = °"<<my_new<<"°"<<endl);

   if(rc != 0)
      if(examine_error(rc, my_new) == TRUE)
        goto retry;
   else
      return  FALSE;

return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef FUCKUP
bool mylib_setdate (mylib_file_t file,USHORT date,USHORT time)
{

   FILESTATUS filestat;
   memcpy(&filestat.ftimeLastWrite, &(time), 2);             // ftime
   memcpy(&filestat.fdateLastWrite, &(date), 2);             // ftime

   rc = DosSetFileInfo(file, 0x0001, (PBYTE) &filestat, sizeof(filestat));

   if(rc == 0)
     return FALSE;
   else
     return TRUE;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void mylib_write_to_screen (char *string)
                                       /* write string to standard output */
{
  char tmpstr[80];
  strcpy(tmpstr, string);
  strcat(tmpstr, "\n\r");
  mylib_write_text (1, (UCHAR *)tmpstr, strlen(tmpstr));   /* stdout */
}

#ifdef FUCKUP
/**********************************************************************/
//                             find number of files in named dir
USHORT find_num_files(byte const* name)
{
   srchcount = 1;
   hdir = 1;    // reset this, it gets messed up on error
   USHORT file_count = 0;
   USHORT dirflags = 55;    // should get all.... 1+2+4+16+32

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
  DBG_DIR(dout<<" mylib find_num_files file_count = "<<file_count<<endl);
return file_count;
}
#endif

#ifdef FUCKUP
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
//   DBG_DIR(dout<<" mylib findnext rc = "<<rc<<endl);
   if(rc == 18) 
      return TRUE;           // err no_more_files
   if(rc != 0) {
       DosBufReset(0xFFFF);           // reset internal file buffers....
       return 1;                      // err
    }
  }
  return 1;
}

#endif

#ifndef NO_COMMANDO
ULONG get_freespace(char drive)
{
  FSALLOCATE fsalloc;
  ULONG      availbytes;

  rc = DosQFSInfo(drive - ('A'-1), 1, (PBYTE) &fsalloc, sizeof(fsalloc));

  if(rc != 0){
     if(examine_error(rc, "") == TRUE)
       return  0;
   }
               /* everything OK, return disk free spec  */
  availbytes = (fsalloc.cUnitAvail * fsalloc.cSectorUnit * fsalloc.cbSector);
  return availbytes;

}
//#ifndef NO_COMMANDO
//;--------------------------------------------
//; Set File Attributes: first parameter is
//;  pointer to complete filename. 2nd parameter
//;  is unsigned int describing new attributes
//
SHORT set_file_attributes(char *file,USHORT attrib)
{
   FILESTATUS3 filestat;
   HFILE hfile;
   filestat.attrFile = attrib;

   if(!mylib_open(file, &hfile, OPEN_ACCESS_READWRITE))
      goto error;

   rc = DosSetFileInfo(hfile, 0x0001, (PBYTE) &filestat, sizeof(filestat));

error:
   if(rc != 0){
     if(examine_error(rc, file) == TRUE)
       return  0;
   }
}
#endif

APIRET cdecl set_int24(void)
{
                                      // disable 7-4-92
//  DosError(HARDERROR_DISABLE);      // works for MS  headers
  rc = DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);   // BC  headers
  return rc;
}

/*************************************************************************/
/*
          Error handling and classification for OS/2
          needed because DOS C can classify and handle
          crtical errors via _dos_setvect() functions
*/

bool examine_error(ULONG error_code, const char *name) 
{
/*
    look at file I/O errors, if not critical, return FALSE,
    if critical, give user "redbox" and allow retry or abort
    possibly develop custom "redbox" function
*/
  enum err_choices myresult;
  ULONG errclass;
  ULONG action;
  ULONG locus;
  bool result;

  DosErrClass(error_code, &errclass, &action, &locus);

 DBG_ERR(dout<<" examine_error passed error_code = "<<error_code<<endl);
 DBG_ERR(dout<<" examine_error errclass = "<<errclass<<endl);
 DBG_ERR(dout<<" examine_error locus = "<<locus<<endl);

  switch (errclass) {
    case 9:                       // illegal char in name
          crit_error = 1;
    case 8:                       // file not found
          result = FALSE;
          break;
    case 7:                         // hardware error ( device not ready )
    case 5:
         crit_error = 1;
   //      myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code, eh_abort|eh_retry, __FILE__);
         myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code, eh_abort|eh_retry, name );
         if(myresult == eh_abort) {
    //        DosBufReset(0xFFFF);      // reset internal file buffers....
            crit_error = 0;
            result = FALSE;
         }
       break;                    // need dialog box for new values ??
    default:
//         myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code,eh_abort | (TRUE ? eh_retry : 0),__FILE__);
         myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code,eh_abort | (TRUE ? eh_retry : 0), name);
         if(myresult == eh_abort) {
    //        DosBufReset(0xFFFF);      // reset internal file buffers....
            crit_error = 0;
            result = FALSE;
         }
      break;
  }
if(crit_error > 0) {
   //      myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code, eh_abort|eh_retry, __FILE__);
         myresult = os2_redbox_s((SHORT)locus,errclass,(SHORT)error_code, eh_abort|eh_retry, name);
         if(myresult == eh_abort) {
   //       DosBufReset(0xFFFF);      // reset internal file buffers....
          result = FALSE;
       }
}
 DBG_ERR(dout<<" examine_error returning "<<result<<endl);
 return result;
}
