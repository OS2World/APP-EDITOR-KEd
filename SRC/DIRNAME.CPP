/*****************************************************
File: DIRNAME.C     Copyright 1989 by Dlugosz Software
   functions for managing filenames
*****************************************************/
#include "usual.h"
#include "dirlist.h"
#include "misc.h"                /* need upcase() */
#include <ctype.h>
#include <string.h>
#include <direct.h>   /* getdcwd */
#include <stdlib.h>   /* _MAX_PATH constant  */
#include <stdio.h>    /* perror func  */

#include "test.h"

char cwd[FNAME_LEN];
char progname[FNAME_LEN];
char exe_dir[FNAME_LEN];

char *find_basename (char const *filename)
/*********************************************************
      return pointer to last filename component in 
      drive:\subdir\subdir\file.exe
****************************************************/
{
char const *s= filename + strlen(filename) -1;

while (*s != '\\') 
       s--;                    /* filename MUST contain a \ character!!!! */
                                  /* s points to rightmost seperator */
return (char *)(s+1);
}


char *find_basesubdir (char *filename)
/************************************************************
     return pointer to leading subdir string in 
     drive:\subdir\subdir\file.exe
     modifies passed string!
******************************************************/
{
int len = strlen(filename);
char *s= filename + len -1;  // point to end of string
char *p= filename;                        // point to beg of string
int counter = 0;

//fprintf(log, "find_sub org str = %s\n", filename);

while (*s != '\\') {
       counter++;
       s--;                    /* filename MUST contain a \ character!!!! */
}
p[(len - counter)] = '\0';                  // terminate at last '\'

//fprintf(log, "find_sub returned str = %s\n", p);                                 
//fprintf(log, "find_sub str length = %d\n", counter);

return (char *)(p);
}

/* **************************************** */

char *find_extension (char *filename)
{
char *s= filename + strlen(filename) -1; // point *s at last char in name
while (*s != '.') 
       s--;                    /* filename MUST contain a . character!!!! */
                                /* s points to rightmost dot */
return s+1;                    // return ptr to 1st char of ext
}

/* **************************************** */

void replace_basename (char *filename, char const* newname)
{
                      /* replace the basename of the filename with newname */
char *s= find_basename (filename);

//if (s-filename + strlen (newname) > 79)  /* will newname fit in filename? */
if (s-filename + strlen (newname) > FNAME_LEN)  /* will newname fit in filename? */
   return;                               /* if not, abort replacement */
strcpy (s, newname);
}

/* **************************************** */

bool filename_iswild (char const* filename)
{
while (*filename) {
   if (*filename == '?' || *filename == '*') {
      DBG_LOAD(dout<<" filename_iswild returning TRUE"<<endl);
      return TRUE;
   }
   filename++;
   }
DBG_LOAD(dout<<" filename_iswild returning FALSE"<<endl);
return FALSE;
}

/* **************************************** */

bool dot_dir (char const* s)
{
return (*s == '.' && (s[1] == '\\' || s[1] == '/' || s[1] == '\0'));
}

/* **************************************** */

bool dotdot_dir (char const* s)
{
return ((*s == '.' && s[1] == '.') && 
        (s[2] == '\\' || s[2] == '/' || s[2] == '\0'));
}

/* **************************************** */
#ifdef FUCK_UP
void normalize_filename (char *filename, char const* default_path,
                         char const* default_extension)
{
char basename[14];
USHORT  chars_left;

char buffer[FNAME_LEN], *s= buffer, *in= filename;
if (*filename == '\0') return;
                                                 /* check for drive letter */
if (filename[1] == ':') {                        /* drive letter is present */
   *s++= (char)upcase(*in++);
   *s++= *in++;                                  /* copy letter and : */
   default_path += 2;                            /* skip it in default */
   }
else {                                    /* copy letter and : from default */
   *s++= *default_path++;
   *s++= *default_path++;
   }
                                     /* see if this is a relative path name */
if (*in != '\\' && *in != '/') {
   if (default_path[-2] == s[-2]) {
      strcpy (s, default_path);
      s += strlen (s);
      }
   else {
      *s++ = '\\';                        /* look up default on actual drive */
      get_directory (s[-3],s);
      while (*s) s++;                     /* s points at end of string */
      }
   }
else {
   *s++ = '\\';
   in++;                                  /* copy the \ */
   }
                                          /* copy one segment at a time */
  chars_left= 79-strlen (buffer);
while (*in && chars_left) {
   if (dot_dir(in)) in+=2;
   else if (dotdot_dir(in)) {
      in += 3;
      if (s[-2] != ':') {
         do {
            s--;
            chars_left++;
            } while (s[-1] != '\\');
         }
      }
   else {                                    /* a normal directory */
      while (chars_left) {
         if (*in == '/' || *in == '\\') {
            *s++ = '\\';
            in++;
            chars_left--;
            break;
            }
         else if (*in == '\0') break;
         else if (*in == ' ') in++;
         else {
            *s++ = (char)upcase(*in++);
            chars_left--;
            }
         }
      }
   }
if (s[-1] == '\\') {                          /* check base name */
      strcpy (basename,"*.");
      strcat (basename,default_extension);
   }
else {
   char *scan= s-1;                  /* make sure name has an extension */
   while (*scan != '\\' && *scan != '.') scan--;
   if (*scan != '.') {
      basename[0]= '.';
      strcpy (basename+1,default_extension);
      }
   else 
      basename[0]= '\0';
   }
if (strlen (basename) < chars_left) 
    strcpy (s,basename);
else 
    *s= '\0';

strcpy (filename, buffer);
}
#endif
/********************************************************************/
void normalize_filename_2 (char *filename, char const* default_path,
                         char const* default_extension)
{
char basename[FNAME_LEN];
USHORT  chars_left;

char buffer[FNAME_LEN], 
            *s= buffer, 
            *in= filename;
if (*filename == '\0') 
     return;

DBG_LOAD(dout<<" normalize_filename entry filename     = °"<<filename<<"°"<<endl);
DBG_LOAD(dout<<" normalize_filename entry default_path = °"<<default_path<<"°"<<endl);
DBG_LOAD(dout<<" normalize_filename entry default_ext  = °"<<default_extension<<"°"<<endl);

                                                 /* check for drive letter */
if (filename[1] == ':') {                        /* drive letter is present */
   *s++= (char)upcase(*in++);
   *s++= *in++;                                  /* copy letter and : */
   default_path += 2;                            /* skip it in default */
   }
else {                                  /* copy letter and : from default */
   *s++= *default_path++;
   *s++= *default_path++;
   }
                                     /* see if this is a relative path name */
if (*in != '\\' && *in != '/') {
   if (default_path[-2] == s[-2]) {
      strcpy (s, default_path);
      s += strlen (s);
      }
   else {
      *s++ = '\\';                      /* look up default on actual drive */
      get_directory (s[-3],s);
      while (*s) 
         s++;                           /* s points at end of string */
      }
 }
else {
   *s++ = '\\';
   in++;                                /* copy the \ */
 }
                                          /* copy one segment at a time */
chars_left= FNAME_LEN - strlen (buffer);
while (*in && chars_left) {
   if (dot_dir(in)) 
      in+=2;
   else if (dotdot_dir(in)) {
      in += 3;
      if (s[-2] != ':') {
         do {
            s--;
            chars_left++;
            } while (s[-1] != '\\');
      }
   }
   else {                                    /* a normal directory */
      while (chars_left) {
         if (*in == '/' || *in == '\\') {
            *s++ = '\\';
            in++;
            chars_left--;
            break;
            }
         else if (*in == '\0') break;
   //      else if (*in == ' ') in++;
         else {
//            *s++ = (char)upcase(*in++);   // mod 9-19-93 HPFS was upper only
            *s++ = (*in++);
            chars_left--;
            }
      }                     // end  while (chars_left) {
   }                        // end  /* a normal directory */
}
if (s[-1] == '\\') {                          /* check base name */
    DBG_LOAD(dout<<"normalize_filename line "<<__LINE__<<" basename     = °"<<basename<<"°"<<endl);
    strcpy (basename,"*.");
//      strcat (basename,default_extension);
    DBG_LOAD(dout<<"normalize_filename line "<<__LINE__<<" basename     = °"<<basename<<"°"<<endl);
   }
else {
   char *scan= s-1;                 /* make sure name has an extension */
   while (*scan != '\\' && *scan != '.') {     // was //
//   while (*scan != '\\') {                   // was not //
      scan--;
   }
   if (*scan != '.') {       // never found a trailing '.'      // was //
      basename[0]= '.';                                         // was //
      DBG_LOAD(dout<<"normalize_filename line "<<__LINE__<<" basename     = °"<<basename<<"°"<<endl);
      strcpy (basename+1,default_extension);   // mod 2-7-93    // was //
      DBG_LOAD(dout<<"normalize_filename line "<<__LINE__<<" basename     = °"<<basename<<"°"<<endl);
      }                                                         // was //
   else                                                         // was //
      basename[0]= '\0';
 }
if (strlen (basename) < chars_left) {
   strcpy (s,basename);
   DBG_LOAD(dout<<"normalize_filename line "<<__LINE__<<" s  = °"<<s<<"°"<<endl);
}
else {
   *s= '\0';
}

strcpy (filename, buffer);
DBG_LOAD(dout<<" normalize_filename returning filename = "<<filename<<endl);
//#pragma warn -par         // we don't use "default_ext" param 
}
//#pragma warn +par 
/* **************************************** */

void add_subdir (char *filename, char const* newnode)
{
char temp[35];                        /* the filename must be normalized. */
memset(temp, 0x00, 35);

DBG_DIR(dout<<" add_subdir newnode  = °"<<newnode<<"°"<<endl);
DBG_DIR(dout<<" add_subdir filename = °"<<filename<<"°"<<endl);

if (dot_dir(newnode)) 
   return;                            /* nothing to do */
if (strlen (filename)+strlen (newnode)+1 > FNAME_LEN)
   return;               /* can't append anything without causing overflow */
filename += strlen (filename);        /* start at the right */
do {
   filename--;
   } while (*filename != '\\');
//strncpy (temp, filename,14);          /* copy '\' and basename to safety */
strncpy (temp, filename, 34);           //copy '\' and basename to safety 

if (dotdot_dir (newnode)) {           /* back up another node */
   if (filename[-1] != ':') do {
      filename--;
      } while (*filename != '\\');
   }
else {
   strcpy (filename+1, newnode);      /* place newnode after \ */
   filename += strlen (filename);
   }
strcpy (filename, temp);              /* put base back on end */
DBG_DIR(dout<<" add_subdir new filename = °"<<filename<<"°"<<endl);
}

/* **************************************** */

bool get_directory (SHORT drive_letter, char *s)
{
/* Get current directory of drive specified by drive_letter ('A','B', etc.).
   Directory path will not have drive code or leading backslash.
   If drive_letter is 0, then the current directory is read. */
static char buf[_MAX_PATH];
SHORT drivenum;

drivenum = drive_letter - 'A' +1;

 if(_getdcwd(drivenum, buf, _MAX_PATH) == NULL)
    perror("Error getting current directory");

//retry:
//if (critical_error) {
//   if (critical_handler (TRUE)) goto retry;
   /* can't read directory properly, return root */
//   s[0]= '\0';
//   return FALSE;
//   }
strcpy (s,buf);
                  /* get_curdir() does not append a \ to
                   the end of the string, so we append one if not root */
if (s[0]){                   /* only append if directory exists */
    if ((strlen(s) >= 3) && (s[strlen(s)-1] != '\\')) { 
      strcat (s,"\\");  
    }
 strcpy (cwd, s);
}
return TRUE;
}

/* **************************************** */

bool get_current_directory (char* s)
{
char drive;
SHORT drivenum;

    drivenum = _getdrive();
    drive = (char)(drivenum + 'A' -1);

if (!get_directory (drive, s)) {
    return FALSE;
}

return TRUE;
}

/* **************************************** */

void delta_filename (char *buf, char const* original, char const* delta)
{
/******************************************************************** 
   this creates a new name based on the original name and a delta.  This
   is similar to the way destination files to a COPY can be named.  If
   the delta contains a drive or path, the file is placed there.  If the
   delta contains a basename, it is renamed.  If the basename contains
   wildcards, they are filled in from the corresponding positions in
   the original name.
**************************************************************************/
char *s, *source, *compare, *dest, temp[FNAME_LEN], original_base[FNAME_LEN];

/* start by normalizing delta based on original.  This takes care of the
   drive and path, and cleans up delta, too.  It is assumed that the
   original is alredy normalized.  */
strcpy (temp, original);

s= find_basename (temp);
strcpy (original_base, s);
*s= '\0';
                                  /* temp now holds a drive and path only */
strcpy (buf, delta);
//normalize_filename (buf, temp, "*");
normalize_filename_2 (buf, temp, "$$");
                               /* now expand any wildcards in the base name */
s= find_basename (buf);
source= s;
compare= original_base;
dest= temp;
while (*source) {
   if (*source == '.') {        /* skip compare ahead to the extension */
      *dest++= '.';
      while (*compare != '.' && *compare != '\0') 
             compare++;
      }
   else if (*source == '?') {
      if (*compare && *compare != '.') 
           *dest++= *compare;
      }
   else if (*source == '*') {
      while (*compare != '.' && *compare != '\0') 
             *dest++ = *compare++;
                               /* if the '*' was not last, skip the rest. */
      while (source[1] != '.' && source[1] != '\0') 
            source++;
      }
   else *dest++ = *source;                      /* a regular character */
                                               /* now increment */
   if (*compare == '.') {
      /* only advance if source is also at the dot, that is, if compare
         gets there first, it waits for source to catch up. */
      if (*source == '.') 
           compare++;
      }
   else if (*compare != '\0') 
             compare++;
   source++;
   }
*dest= '\0';
                              /* now put the basename back where it belongs */
strcpy (s, temp);
}

/* **************************************** */
#ifndef NO_COMMANDO
bool filename_samedir (char const* name1, char const* name2)
{
char const *base1= find_basename (name1);
char const *base2= find_basename (name2);
SHORT len= base1-name1;

if (len == base2-name2) {
   while (len--) if (*name1++ != *name2++) return FALSE;
   return TRUE;
   }
return FALSE;
}
#endif
/* **************************************** */

