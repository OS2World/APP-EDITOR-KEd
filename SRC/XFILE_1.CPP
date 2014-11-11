/*********************************************************************
  xfile.c -- implementation for fast line buffered files

  Currently XFILEs are for reading CR-LF or bare LF terminated lines
  from MS-DOS text files. The extracted line does NOT have a NEWLINE 
  at the end.

  The performance with long lines is good: it can
  handle lines the size of the buffer, though it may truncate up to one
  QUANTUM less one bytes "early": this depends on the location of the start
  of the line in the buffer when we begin scanning.  In practice, XBUFSIZE
  is probably larger than you'd set for a line buffer size anyway...

  TO DO:
  clean up xgetline!

    5-26-94
    Some files (compiler output!) have lines terminated with CR CR LF
    so I munged the xgetline code. We'll see if this holds......

  6-16-94
    I really don't like doing a read in the xopen func.
    This will NOT load binary file
    I'm not fond of the 2 sections in xgetline, the 2nd seems redundant

  10-3-94
    using CRTL functions lead to a funny bug on file save: access denied
    using XFILE for KEd 2.10
    tab2spc and spc2tab use tempstr[] buffer, currently 1024 bytes

  10-7-94
    sheet_save() and sheet_load() are here now. 
    a single static 16kb buffer is used for file save and load.
    a single static 1kb buffer is used for the tab2spc() and spc2tab()
    functions, this limits line length to 1023 chars. 
    THIS MAY BE A PROBLEM...
    tab2spc() and spc2tab appear to function correctly.
    Decision to make default "expand tabs" on file_load, and
    "compress tabs" = NO for file_save. (That's what I like ;-}  )

  9-1-95
    added BC rtl fgets code as a template for futures.
    I want to be able to load binary files (or files with embedded 0x00)
    but return line terminators of CR, CR LF, or LF lines.

  9-2-95
    Word Wrap support:
    Decision to not modify this section for word wrap, just read
    the file and return whole lines to Sheet_Load. Sheet functions will
    take the incoming line and break it into chunks for word wrap support.

  9-30-95
    Sheet_Load cleanup:
    Added several simple macros at the start of this file to automate
    changing from XFILE to CRTL functions. Helps Sheet_Load look a 
    little cleaner.

*********************************************************************/
#define USE_XFILE 1            // use xfile funcs
//#define USE_CRTL  1          // use c rtl funcs

#include "usual.h"
#include "mylib.h"
#include "vmem.h"
#include "sheet.h"
#include "redbox.h"
#include "status.h"
#include "setting.h"       // tab, write_eof, word_wrap settings
#include <stdlib.h>
#include <string.h>

#ifdef USE_CRTL
#include <stdio.h>
#endif

#include "test.h"

/************************ COMMON ******************************************/
static bool tab2spc(byte *str, USHORT in_len);   // expand tab to spaces
static bool spc2tab(byte *str, USHORT in_len);
#define CR 0x0D         // = '\r'
#define LF 0x0A         // = '\n'
#define TAB 0x09
#define SPACE 0x20
#define TAB_SPC_LEN 1024
static char tempstr[TAB_SPC_LEN];        // arbitrary length, used for tab2spc
static  char file_EOF_char= '\x1a';
//#define view_def settings.viewer_defaults;

//#define XBUFN 4            /* set default # of quanta in buffer, allow -D */
// org
#define XBUFN 8            /* set default # of quanta in buffer, allow -D */

#define QUANTUM 4096
#define XBUFSIZE (XBUFN * QUANTUM)   // 16kb buf 
//static byte buf[XBUFSIZE];           // used for file save
static byte *buf = 0;                // used for file load and save

/**********************************************************************/
#ifdef USE_XFILE
struct _xfile {
    ULONG fd;             // file descriptor
    int   bufSize;
    unsigned char *buf;
    unsigned char *nextChar;
    unsigned long total;
};
typedef struct _xfile XFILE;
static XFILE *f;
static XFILE *xopen(const char*);
static void   xclose(XFILE *);
static unsigned char  *xgetline(XFILE *);

#define k_close(f) xclose(f)
#define k_open  xopen
#endif

/************************************************************************/
#ifdef USE_CRTL
static FILE *f;
static FILE *myopen(const char*);
static bool myclose(FILE *f);
static bool myfgets(byte *str);

#define k_close(f) myclose(f)
#define k_open  myopen

#endif

/*******************************************************************
      tab2spc  expand tab characters in line to spaces,
      return new string.
      Input str must be null-terminated
**************************************************************************/
static bool tab2spc(byte *str, USHORT in_len)
{
  byte *p = tempstr;
  byte *s = str;
  int i = 0;
  bool result = FALSE;

DBG_FILE_TAB(dout<<"xfile tab2spc passed str    = °"<<str<<"°"<<endl);
                       //DBG_FILE_TAB(dout<<"xfile tab2spc begin str ptr = "<<&str<<endl);
                       //DBG_FILE_TAB(dout<<"xfile tab2spc begin s ptr   = "<<&s<<endl);   
                       //DBG_FILE_TAB(dout<<"xfile tab2spc begin p ptr   = "<<&p<<endl);

  while(in_len  && (i < (TAB_SPC_LEN-1))) {        
     if (*s == TAB) {               // found a tab
        result = TRUE;
        do {
//           DBG_FILE_TAB(dout<<"tab2spc expand tab i = "<<i<<" i%4 = "<<(i%4)<<endl);
           p[i++] = SPACE;
        } while((i % settings.viewer_defaults.tab_size) != 0);
//        DBG_FILE_TAB(dout<<"tab2spc expanded tab "<<endl);
        s++;                        // increment past the tab
        in_len--;
     }
     else {
        p[i++] = *s;
        s++;                        // get next char 
        in_len--;
     }
  }
   p[i] = 0x00;                       // terminate the tempstr
   DBG_FILE_TAB(dout<<"xfile tab2spc new str p[] = °"<<p<<"°"<<endl);
//   DBG_FILE_TAB(dout<<"xfile tab2spc end str ptr = "<<&str<<endl);
//   DBG_FILE_TAB(dout<<"xfile tab2spc end s   ptr = "<<&s<<endl);
//   DBG_FILE_TAB(dout<<"xfile tab2spc end p   ptr = "<<&p<<endl);
return result;
}
/****************************************************************
       compress spaces to tabs on sheet_save
       rework 10-31-94
***************************************************************/
static bool spc2tab(byte *str, USHORT in_len)
{
  byte *p = tempstr;
  byte *s = str;
  int in = 1;           // input string counter
  int out = 0;          // output string counter
  int spc_count = 0;
  bool result = FALSE;

DBG_FILE_TAB(dout<<"xfile spc2tab passed str    = °"<<str<<"°"<<endl);

   for(in; in <= in_len; in++) {
      if(*s == SPACE) {
         spc_count++;
         if(((in % settings.viewer_defaults.tab_size) == 0) && (spc_count > 1)) {
            DBG_FILE_TAB(dout<<"spc2tab out = "<<out<<"  spc_count = "<<spc_count<<" in = "<<in<<endl);
            out -= (spc_count-1);                 // trap if out < 0 ....
            p[out] = TAB;
            spc_count = 0;
            goto next;
         }
      }
      else {
        spc_count = 0;
      }
      p[out] = *s;
next:
      s++;
      out++;
   }
   p[out] = *s;
   p[++out] = 0x00;                       // terminate the tempstr
   DBG_FILE_TAB(dout<<"xfile spc2tab new str p[] = °"<<p<<"°"<<endl);
return result;
}


/*******************************************************************
     load a file in the sheet given by the "vptr sh" parameter.
     returns TRUE for success, and FALSE only for a file open error.
**************************************************************************/
bool sheet_load (vptr sh, char const* filename, bool remember_name)
{
#pragma intrinsic memcpy
DBG_LOAD(dout<<"sheet_load filename = °"<<filename<<"°"<<endl);
DBG_LOAD(dout<<"sheet_load XBUFSIZE = "<<XBUFSIZE<<endl);

#ifdef USE_XFILE
  unsigned char *s;
#else
  unsigned char s[132];        // CRTL
#endif

SHORT length;
struct line_header *h;
vptr line;
struct sheet *cur;
USHORT row_subcount = 0;

cur = (struct sheet *)Mem::vmem_trans (sh);
if (remember_name) 
   strcpy (cur->filename, filename);

buf = new byte[XBUFSIZE];
memset(buf, 00, XBUFSIZE);

if (!k_open(filename)) {
    return FALSE;
}

                              // file read is appended to existing sheet.
if (cur->line_count) {
   line= get_line_anysheet (cur, cur->line_count -1); // check if line in sheet
   h= (struct line_header *)Mem::vmem_trans (line);
   if (h->length == 0) {
      cur->line_count--;
      vmem_free (line);               // if empty, wipe it
   }
}

status_redraw();                      // file open ok, so update status
cur->space= vmem_burst_start (cur->space);

DBG_FILE(dout<<"sheet_load after xopen, before xgetline loop "<<endl);

#ifdef USE_XFILE
  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
  while((s = xgetline(f)) != 0) {                    // read the new file 
#else
  while(myfgets(s)) {                                // CRTL
#endif
   DBG_FILE2(dout<<"sheet_load line  "<<(cur->line_count)<<"  = °"<<s<<"°"<<endl);
   length = strlen((char *)s);
   if(settings.flags & 4) {          // expand tabs?
         tab2spc(s, length);
         length = strlen((char *)tempstr);
         DBG_FILE_TAB(dout<<"sheet_load tab2spc = °"<<tempstr<<"°"<<endl);
   }
   h= (line_header*)vmem_burst_alloc(sizeof(struct line_header)+length+5, &line);
   h->length   = length;
   h->capacity = h->length+5;      // mod 3-15-93
//   memset(h+1, 0, h->capacity);    // zero the sheet line buffer MOD 5-23-94
   memcpy (h+1, s, length);        // copy the line we read into the sheet line
   s[length+1] = 0x00;             // terminate str

   if (cur->line_count >= 100) {       // display row  if file is big 
      if (++row_subcount >= 10) {      // display every 10 rows to save time 
         row_subcount= 0;
         status_row (cur->line_count);
      }
   }
   if(cur->line_count > MAXLINES) {                     // file too big
   DBG_FILE2(dout<<" File too big !! "<<" lines = "<<(cur->line_count)<<endl);
       redbox(2,101,eh_abort);                          // report the error
       goto end_load;
   }
   put_line_anysheet (cur, cur->line_count++, line);   //put line into sheet
}                             // end while(get_line()

  put_line_anysheet (cur, cur->line_count, zero_line()); //end with blank line

end_load:
vmem_burst_end();
DBG_FILE2(dout<<" end file_load, closing file "<<endl);
k_close(f);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

delete [] buf;
buf = 0;
return TRUE;
#pragma intrinsic -memcpy
}

/***************************** **************************************** 
     sheet_save  
                 // If filename = NULL, name stored in the sheet is used
*********************************************************************/
SHORT sheet_save (vptr sheet_to_save, char const* filename)
{
//#pragma intrinsic memcpy
DBG_LOAD(dout<<"sheet_SAVE XBUFSIZE = "<<XBUFSIZE<<endl);
mylib_file_t f;
SHORT index = 0;
SHORT roomleft = XBUFSIZE;
struct sheet *cur = (struct sheet *)Mem::vmem_trans (sheet_to_save);
ULONG line= 0;
ULONG linemax = cur->line_count;
bool close_ok = FALSE;
USHORT length = 0;

buf = new byte[XBUFSIZE];
if(buf != 0) {
   DBG_LOAD(dout<<"sheet_SAVE calling memset XBUFSIZE = "<<XBUFSIZE<<endl);
   memset(buf, 00, XBUFSIZE);
}

  DBG_MEM(memout<<" sheet_save file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

if (!filename) 
   filename= cur->filename;
if (!mylib_create (filename, &f)) {
   return 1;                                /* create the output file */
}
      /* it seems wierd to put a printing function here, but I'm going
         to need to open the file and output text anyways....  */

//if (stricmp((char *)settings.printer_setup_string, "")!= 0) {
//    mylib_write_text (f, settings.printer_setup_string, sizeof(settings.printer_setup_string));
//}

for (;;) {                                       // write each line 
   struct line_header *h= line_fetch (sheet_to_save, line);
   unsigned char *s= (unsigned char *)(h+1);    // string begins after header 
   length = h->length;
   if(settings.flags & 8) {                     // compress spaces to tabs ?
      spc2tab(s, length);
      length = strlen(tempstr);
      s = tempstr;
      DBG_FILE_TAB(dout<<"sheet_save spc2tab = °"<<tempstr<<"°"<<endl);
   }
   DBG_MEM(memout<<"sheet_save line = "<<line<<"  file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
   DBG_SHEET_SAVE(dout<<"sheet_save line = "<<line<<"length = "<<length<<" index = "<<index<<endl);
   DBG_SHEET_SAVE(dout<<" sheet_save str = °"<<s<<"° "<<endl);
   while (length > 0) {
      if (length < roomleft) {                  // copy line to buffer 
         DBG_MEM(memout<<"sheet_save line = "<<line<<"  file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
         DBG_SHEET_SAVE(dout<<"sheet_save line = "<<line<<"length = "<<length<<" index = "<<index<<endl);
         DBG_SHEET_SAVE(dout<<" sheet_save line = °"<<s<<"° "<<endl);
         memcpy (buf+index, s, length);
         index += length;
         roomleft -= length;
         break;                         // whole line fit into buf
      }
      else {                            // copy partial line to buf + retry
         memcpy (buf+index, s, roomleft);
         DBG_SHEET_SAVE(dout<<" sheet_save line = °"<<s<<"° "<<endl);
         length -= roomleft;
         s += roomleft;
         if (!mylib_write_text (f, buf, XBUFSIZE)) 
              goto write_error;
         index= 0;
         roomleft= XBUFSIZE;
      }
   }                                      // end while length > 0
   if (++line == linemax) 
      break;                              /* the loop's exit */
                                          
   if (roomleft < 2) {                       /* flush */
      if (!mylib_write_text (f, buf, XBUFSIZE-roomleft)) 
           goto write_error;
      index= 0;
      roomleft= XBUFSIZE;
   }
   memcpy (buf+index, "\r\n", 2);            // terminate line with CRLF 
   index += 2;
   roomleft -= 2;
   }                                         // end write all lines

if (index) {                          /* flush the buffer */
   if (!mylib_write_text (f, buf, index)) 
       goto write_error;
   }
if (settings.write_eof) {    // option set to write Ctrl-Z at EOF
   mylib_write_text (f, (UCHAR *)&file_EOF_char, 1);
   }

  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

delete [] buf;
close_ok = mylib_close (f);
cur->changed= FALSE;
if(close_ok == TRUE)
   return 0;

write_error:                     /* report the error */
 delete [] buf;
 mylib_close (f);
 mylib_delete (filename);
 return 2;
//#pragma intrinsic -memcpy
}


#ifdef USE_CRTL
/***********************************************************************/
FILE *myopen(const char *name)
{
  if(name == NULL)                                // no name!
     return 0;
//  if (!mylib_open(name, f, MYLIB_READONLY)) {
  if ((f = fopen(name, "rb")) == 0) {
        return 0;
  }
  setvbuf(f, buf, _IOFBF, XBUFSIZE);
  return f;
}
#endif

#ifdef USE_CRTL
/***********************************************************************/
bool myclose(FILE *f)
{
  bool rc;
  setvbuf(f, 0, _IOFBF, XBUFSIZE);
  rc = fclose(f);
  return rc;
}
#endif

#ifdef USE_CRTL
/***********************************************************************/
bool myfgets(byte *str)
{
#pragma intrinsic strlen
static bool remainder = FALSE;
int length;

  if(!remainder) {
     if((fgets(str,132,f)) != 0) {
        length = strlen(str);
        length--;                       // discard trailing '\0'
        if(str[length] == LF) {
           str[length] = 0x00;
           if(str[length-1] == CR) 
              str[length-1] = 0x00; 
           if(str[length-2] == CR) {
              remainder = TRUE;
              str[length-2] = 0x00; 
              return TRUE;
           }
        }               
     }
     else {
        return FALSE;    // fgets is finished
     }
  }
  else {
     remainder = FALSE;
     str[0] = 0x00;
     return TRUE;
  }
  return TRUE;
#pragma intrinsic -strlen
}
#endif

#ifdef FUCKUP
/*------------------------------------------------------------------------
 * filename - fgets.c
 *
 * function(s)
 *        fgets - gets a string from a stream

 *      C/C++ Run Time Library - Version 1.5 Borland
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <_io.h>
#include <_stdio.h>

/*---------------------------------------------------------------------*

Name            fgets - gets a string from a stream

Usage           char *fgets(char *string, int n, FILE *stream);

Prototype in    stdio.h

Description     reads characters from stream into the string string:
                The function stops reading when it either reads n-1
                characters or reads a newline character (whichever
                comes first).  fgets retains the newline character.
                The last character read into string is followed by a
                null character.

Return value    success : pointer to string
                failure : NULL

*---------------------------------------------------------------------*/

char * _RTLENTRY _EXPFUNC fgets (char *s, int n, FILE *fp)
{
    int     c, Chunk;
    char    *P, *eolp;

    P = s;

//    _lock_stream(fp);

//    if (fp->bsize != 0)             /* file is buffered */
//    {
        n--;                        /* leave room for null terminator */
        while( n >= 0 )
        {
            /* If there is anything in the buffer, copy it directly
             * to the user's buffer.
             */
            if (fp->level > 0)         /* something in the buffer */
            {
                if (n > (unsigned)fp->level)
                    Chunk = fp->level;
                else
                    Chunk = n;

                /* If there's a newline in the buffer, treat that
                 * the end of the buffer.
                 */
                if ((eolp = memchr(fp->curp, '\n', Chunk)) != NULL)
                    Chunk = eolp - (char *)fp->curp + 1;

                /* Copy bytes from the file buffer to the user's buffer,
                 * then update pointers and counts.
                 */
                memcpy(P, fp->curp, Chunk);
                fp->curp += Chunk;
                fp->level -= Chunk;
                P += Chunk;
                n -= Chunk;

                /* If the user's buffer is full, or a newline was seen,
                 * stop reading bytes now.
                 */
                if (eolp != NULL || n == 0)
                {
                    c = '\n';
                    break;
                }

            }
            else
                {
                /* The buffer is empty.  Use getc() to get one character
                 * and fill the buffer.
                 */
                if ((c = (int)getc(fp)) == EOF)
                    {
                    fp->flags |= _F_EOF;
                    break;
                    }
                *P++ = (char)c;
                --n;
                if (c == '\n')
                    break;
                }
        }
    if (EOF == c && P == s)
        P = NULL;
    else
    {
        *P = '\0';
        P = ferror (fp) ? NULL : s;
    }

//    _unlock_stream(fp);
    return (P);
}
#endif


#ifdef USE_XFILE
/***********************************************************************
  xopen -- allocate and open an XFILE

  currently using a  READ-ONLY TEXT FILES call
  returns pointer to XFILE of opened file or null pointer on error
  ? should it leave a better error description somewhere ?
**************************************************************************/
XFILE *xopen(const char *name)
{
  int n;

  if(name == NULL)                                // no name!
     return 0;
          
  f = (XFILE *)malloc(sizeof(XFILE) + XBUFSIZE + 1);
  if (f == 0)                                       // malloc failed
     return 0;

  f->buf = (unsigned char *)f + sizeof(XFILE);   //set buf ptr to start of buf

  DBG_LOAD(dout<<"xopen filename = °"<<name<<"°"<<endl);
  if (!mylib_open(name, &f->fd, MYLIB_READONLY)) {
        free(f);
        return 0;
  }
  if ((n = mylib_read_text(f->fd, f->buf, XBUFSIZE)) == 0) {  // read 1st buf
        mylib_close(f->fd);
        return 0;
  }
  f->total = XBUFSIZE;              // number of chars in buf
  DBG_FILE(dout<<" xopen read  "<<n<<" bytes"<<endl);
//  DBG_FILE(dout<<" xopen last char in buf was °"<<hex<<(f->buf[n])<<dec<<"°"<<endl);
  f->buf[n] = 0;                    // set last char in buf to 0
//  DBG_FILE(dout<<" xopen last char in buf now °"<<hex<<(f->buf[n])<<dec<<"°"<<endl);
  f->nextChar = f->buf;             // set to start of buf
  return f;
}
#endif

#ifdef USE_XFILE
/************************************************************************
**  xclose -- close and deallocate an XFILE
************************************************************************/
void xclose(XFILE *f)
{
   mylib_close(f->fd);
   free(f);              // free the buf
   DBG_FILE(dout<<" file close OK, free(buf) ok "<<endl);
}
#endif

#ifdef USE_XFILE
/************************************************************************
**  xgetline -- get the next text line into memory
**  returns a pointer to the line (a NUL-terminated string) or a null pointer

    1st time:
             f->nextChar and s point to start of buf
             if CRCRLF, CRLF, or LF is found, return the line
                 f->nextChar points to next char after CR or LF
    2nd time:
            f->nextChar and s point somewhere in buf


***********************************************************************/
unsigned char *xgetline(XFILE *f)
{                      
   unsigned char *p;                    // temp ptr
   unsigned char *s = f->nextChar;      // "s" is start of string we read
   int n;
   static bool partial_line = FALSE;
   static bool final_line = FALSE;

//   DBG_FILE(dout<<"xgetline entry p      = 0x"<<hex<<(ULONG)s<<dec<<endl);
//   DBG_FILE(dout<<"xgetline entry f->buf = 0x"<<hex<<(ULONG)f->buf<<dec<<endl);

//   DBG_FILE(dout<<" xfile entry  s = °"<<s<<"°"<<endl);

   if(final_line == TRUE) {
      DBG_FILE1(dout<<" xgetline  final_line == TRUE "<<endl);
      partial_line = FALSE;
      final_line = FALSE;
      f->nextChar = f->buf;           // set buf back to defaults
      f->buf[0] = 0;
      return 0;                       // we're done
   }
   for (p = s; *p != 0; ++p) {          // walk through the string
       if (*p == LF) {                 
//          DBG_FILE1(dout<<" xgetline section 1A *p == LF"<<endl);
          if ((s < p) && (p[-1] == CR)) {
             if(p[-2] == CR) {          // 0x line space
                p[-2] = 0;             // some lines have CR CRLF
                f->nextChar = p -1;
                DBG_FILE1(dout<<" xgetline section 1A s = "<<s<<endl);
                return s;              // return this piece
             }
             p[-1] = 0;                // replace prev CR with 0
          }
          else                   
             *p = 0;                   // replace LF or CR with 0
          f->nextChar = p + 1;         // save position for next call
          DBG_FILE1(dout<<" xgetline section 1B s = "<<s<<endl);
          return s;                    // found a whole line, return it
       }
//   DBG_FILE1(dout<<"xgetline line "<<__LINE__<<"  p = 0x"<<hex<<(ULONG)p<<dec<<endl);
   }
   DBG_FILE1(dout<<" xfile 1st section left partial line"<<endl);
               // EOL not found in buffer, p usually points to 0 at end of buf
   if (p == f->buf) {   // if p points to start of buf, buf is empty, so EOF 
       DBG_FILE1(dout<<" xfile 1st section  EOF"<<endl);
       return 0;
   }

   DBG_FILE1(dout<<"xgetline line "<<__LINE__<<"  s = 0x"<<hex<<(ULONG)s<<dec<<endl);                               
   if (s != f->buf) {           //move partial line to beginning of buffer
         DBG_FILE1(dout<<" xfile 1st section moving partial line to begin"<<endl);
         for (p = f->buf; (*p = *s) != 0; ++p, ++s)
               ;
         s = f->buf;
         DBG_FILE1(dout<<" xfile 1st section partial line = °"<<s<<"°"<<endl);
         partial_line = TRUE;
   }
   n = XBUFSIZE - (p - f->buf);
   if (n < QUANTUM) {                  // insufficent room, break line 
      DBG_FILE1(dout<<" xfile 1st section insufficient room"<<endl);
      f->nextChar = p;
      return s;
   }
   n = (n / QUANTUM) * QUANTUM;        // quantize: count to read 
   n = mylib_read_text(f->fd, p, n);
   DBG_FILE1(dout<<" xfile read  "<<n<<" bytes"<<endl);
                                      // read error is same return as EOF.
   if (n <= 0) {                      // read error or end of file 
      DBG_FILE1(dout<<" xfile  partial line = "<<partial_line<<" final_line = "<<final_line<<endl);
//      if((partial_line == TRUE) && (final_line == FALSE)) {
      if(s == f->buf) {
         DBG_FILE1(dout<<" xfile n <= 0  "<<endl);
         partial_line = FALSE;
         final_line = TRUE;
         return s;
      }
      f->nextChar = f->buf;           // set buf back to defaults
      f->buf[0] = 0;
      return 0;                       // we're done
   }

   DBG_FILE1(dout<<"xgetline 2nd section last char in buf was °"<<(p[n])<<"°"<<endl);
   f->total += XBUFSIZE;             // read was successful
   p[n] = 0;                         // set last char in buf to 0
   DBG_FILE1(dout<<"xgetline 2nd section last char in buf now °"<<(p[n])<<"°"<<endl);

   for ( ; *p != 0; ++p) {
     if (*p == LF) {
        if ((s < p) && (p[-1] == CR)) {
           if(p[-2] == CR) {         // some lines have CR CRLF
              p[-2] = 0;             // zero the CR
              f->nextChar = p -1;
              return s;              // return this piece
           }
           p[-1] = 0;
        }
        else  
           *p = 0;
        ++p;
        break;
     }
  }
  f->nextChar = p;
//  return p == s ? 0 : s;
  if(p == s) {
     DBG_FILE1(dout<<" xfile 2nd section s= 0 "<<endl);
     return 0;
  }
  else {
    DBG_FILE1(dout<<" xfile 2nd section s = "<<s<<endl);
    return s;
  }

}
#endif










