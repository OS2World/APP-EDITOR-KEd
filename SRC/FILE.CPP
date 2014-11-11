/*****************************************************
file: FILE.C        Copyright 1989 by Dlugosz Software
   save and load buffers
*****************************************************/
#include "usual.h"
#include "mylib.h"
#include "vmem.h"
#include "sheet.h"
#include <string.h>
#include "setting.h"        // need printer_setup_string
#include "redbox.h"

#include "test.h"

#define locus 2

void status_row (ULONG);        // ext prototype
static  char file_EOF_char= '\x1a';

/* **************************************** */
#ifdef FUCKUP
static bool line_ended_in_eoln;

static struct input_buffer {
   vptr buf_v;
   unsigned char *buf;
   mylib_file_t f;
   SHORT start, end, chars_left;
   bool eof;
   SHORT len;
   } ibuf;  /* used by load file stuff */

/* **************************************** */

//static bool  eoln (char c)
inline bool  eoln (char c)
{
return c == '\r' || c == '\n' || (file_write_eof && c == file_EOF_char);
}

/* **************************************** */

static unsigned char nextchar(void)
{
/* read the file if buffer is empty
   advance the end pointer in the buffer.  If the buffer is
   empty, copy the string to the beginning and read more in
   after it.  If no more could be read, set the EOF flag.
   It will actually scan ahead and only return interesting characters */

unsigned char c;
restart:
{      /* this block is optional.  It uses an assembly language function
          to improve performance.  It skips ahead to an interesting char */
//   SHORT count;

//#if FALSE
   if (ibuf.buf[ibuf.end] == '\r' && 
      (ibuf.buf[ibuf.end+1] != '\r' && 
       ibuf.buf[ibuf.end+1] != '\n'))
      { 
      ibuf.end++;
      ibuf.len++;
      ibuf.chars_left--;
//      return '\0';
      }
//   else
//#endif                // WARNING THIS WILL CRASH FILE LOAD
//   count= nextchar_fastscan(ibuf.buf+ibuf.end+1, ibuf.chars_left);
//   ibuf.chars_left -= count;
//   ibuf.len += count;
//   ibuf.end += count;
   }
if (ibuf.chars_left == 0) {                            /* nothing left */
   if (ibuf.eof) return '\0';
   memmove (ibuf.buf, ibuf.buf+ibuf.start, 1+ibuf.end-ibuf.start);
   ibuf.end -= ibuf.start;
   ibuf.start= 0;
   ibuf.chars_left= mylib_read_text (ibuf.f, 1+ibuf.buf+ibuf.end, (VMEM_PAGESIZE - ibuf.end)-1);
   if (ibuf.chars_left == 0) {
      ibuf.eof= TRUE;
      return '\0';
      }
   goto restart;  /* retry optimization routine */
   }
ibuf.chars_left--;
c= ibuf.buf[++ibuf.end];
if (file_write_eof && c == file_EOF_char) {  /* cut the file short here. */
   ibuf.eof= TRUE;
   ibuf.chars_left= 0;
   }
   else ibuf.len++;
                    /* optimize:  don't return uninteresting characters. */
   if (ibuf.len < 16000 && c >= 32 && c != file_EOF_char) goto restart;
return c;
}

/* **************************************** */

static unsigned char  really_nextchar(void)
{
/* advance the end pointer in the buffer.  If the buffer is
   empty, copy the string to the beginning and read more in
   after it.  If no more could be read, set the EOF flag */
unsigned char c;

if (ibuf.chars_left == 0) {  /* nothing left */
   if (ibuf.eof) return '\0';
   memmove (ibuf.buf, ibuf.buf+ibuf.start, 1+ibuf.end-ibuf.start);
   ibuf.end -= ibuf.start;
   ibuf.start= 0;
   ibuf.chars_left= mylib_read_text (ibuf.f, 1+ibuf.buf+ibuf.end, (VMEM_PAGESIZE - ibuf.end)-1);
   if (ibuf.chars_left == 0) {
      ibuf.eof= TRUE;
      return '\0';
      }
   }
ibuf.chars_left--;
c= ibuf.buf[++ibuf.end];
if (file_write_eof && c == file_EOF_char) {     /* cut the file short here. */
   ibuf.eof= TRUE;
   ibuf.chars_left= 0;
   }
else ibuf.len++;
return c;
}

/* **************************************** */

static unsigned char *eject_line(void)
{
/* return the line in the buffer, and start a new line. */
unsigned char *s;

if (!ibuf.eof) {
                                     /* skip over /n after a /r */
   if (ibuf.buf[ibuf.end] == '\r') {
      char c= really_nextchar();
      if (c != '\n') {               /* put it back */
         ibuf.end--;
         ibuf.chars_left++;
         }
      }
   }
else line_ended_in_eoln= FALSE;
s= ibuf.buf + ibuf.start;
ibuf.start= ibuf.end + 1;
ibuf.len= 0;
return s;
}

/* **************************************** */
#if FALSE
static void  verify_ibuf_ok(void)
{
assert (ibuf.buf == Mem::vmem_trans(ibuf.buf_v));
}
#endif

/* **************************************** */

//static bool  get_next_line (unsigned char** return_s,SHORT *return_length)
//{
/* read the next line from the file.  return a pointer to it
   in the buffer and its length.  Function returns TRUE.  If
   no line was read, function returns FALSE.  */

/* verify_ibuf_ok(); */
//while (ibuf.len < 16000 && !eoln(nextchar()) && !ibuf.eof) {}
//if (ibuf.len == 0 && ibuf.eof) 
//    return FALSE;
//*return_length= ibuf.eof ? ibuf.len : ibuf.len-1;  /* don't count eoln char */
//*return_s= eject_line();
//return TRUE;
//}

/* **************************************** */

static bool open_ibuf(char const* filename)
{
line_ended_in_eoln= TRUE;
if (mylib_open (filename, &ibuf.f, MYLIB_READONLY)) {
   ibuf.buf_v= vmem_newpage();
   ibuf.buf= (unsigned char *)Mem::vmem_trans (ibuf.buf_v);
   ibuf.eof= FALSE;
   ibuf.start= 0;
   ibuf.end= -1;
   ibuf.chars_left= 0;
   ibuf.len= 0;
   return TRUE;
   }
return FALSE;
}

/* **************************************** */

//static void  close_ibuf (void)
//{
//mylib_close (ibuf.f);
//vmem_droppage (ibuf.buf_v);
//}
#endif                             // end of conditional from line 23


/* **************************************** */
#ifdef FUCKUP
bool sheet_load (vptr sh, char const* filename, bool remember_name)
{
#pragma intrinsic memcpy
unsigned char *s;
SHORT length;
struct line_header *h;
vptr line;
struct sheet *cur;
USHORT row_subcount = 0;
bool EOT = FALSE;

cur = (struct sheet *)Mem::vmem_trans (sh);
if (remember_name) 
   strcpy (cur->filename, filename);
if (!open_ibuf (filename)) {
    return FALSE;
}
   /* the file read is appended to the existing sheet.  If the last
   line in the sheet is empty, it is overwritten by the first line read. */
if (cur->line_count) {
   /* only check if there is a line.  The sheet should always have at least
      one line, but I'll test to make sure.  I may decide to free all lines
      and then load for efficency, rather then putting in a blank line just
      to erase it again. */
   line= get_line_anysheet (cur, cur->line_count -1);
   h= (struct line_header *)Mem::vmem_trans (line);
   if (h->length == 0) {
      cur->line_count--;
      vmem_free (line);
      }
   }
                                                   /* read the new file */
cur->space= vmem_burst_start (cur->space);
//while (get_next_line (&s, &length)) {
while(! EOT) {
                                   // inline get_next_line
   while (ibuf.len < 16000 && !eoln(nextchar()) && !ibuf.eof) {}
   if (ibuf.len == 0 && ibuf.eof) {
      //return FALSE;
      EOT = TRUE;
   }
   length= ibuf.eof ? ibuf.len : ibuf.len-1;/*don't count eoln char*/
   s= eject_line();
     //  return TRUE;            // falls through
                                 // end get_next_line
//   DBG_FILE(dout<<"sheet_load line = °"<<s<<"°"<<endl);
   h= (struct line_header*)vmem_burst_alloc(sizeof(struct line_header)+length+5, &line);
   h->length= h->capacity= length;
   h->capacity = h->length+5;      // mod 3-15-93
   memset(h+1, 0, h->capacity);    // MOD 5-23-94
   memcpy (h+1, s, length);
                                  /* display row  if file is big */
   if (cur->line_count >= 100) {
      if (++row_subcount >= 10) {  /* display every 10 lines to save time */
         row_subcount= 0;
         status_row (cur->line_count);
      }
   }
   if(cur->line_count > TIER_SIZE * TIERCOUNT) {          // file too big
       redbox(2,101,eh_abort);               // report the error
       vmem_burst_end();
       mylib_close (ibuf.f);                 //close_ibuf();
       vmem_droppage (ibuf.buf_v);
       return TRUE;                          
   }
   put_line_anysheet (cur, cur->line_count++, line);   //put line into sheet
}                             // end while(get_line()
vmem_burst_end();
if (line_ended_in_eoln && !ibuf.eof) {     // end file with blank line
    put_line_anysheet (cur, cur->line_count++, zero_line());
}
mylib_close (ibuf.f);                 //close_ibuf();
vmem_droppage (ibuf.buf_v);

return TRUE;
#pragma intrinsic -memcpy
}
#endif
/* **************************************** */

SHORT sheet_save (vptr sheet_to_save, char const* filename)
{
                      // If filename = NULL, name stored in the sheet is used
vptr buf_v = vmem_newpage();                 /* allocate a buffer */
unsigned char *buf = (unsigned char *)Mem::vmem_trans (buf_v);
mylib_file_t f;
SHORT index = 0;
SHORT roomleft = VMEM_PAGESIZE;
struct sheet *cur = (struct sheet *)Mem::vmem_trans (sheet_to_save);
ULONG line= 0, linemax= cur->line_count;

if (!filename) 
   filename= cur->filename;
if (!mylib_create (filename, &f)) {
   return 1;                                /* create the output file */
}
      /* it seems wierd to put a printing function here, but I'm going
         to need to open the file and output text anyways....  */

if (stricmp((char *)settings.printer_setup_string, "")!= 0) {
    mylib_write_text (f, settings.printer_setup_string, 
                         sizeof(settings.printer_setup_string));
}

//buf_v= vmem_newpage();                       /* allocate a buffer */
//buf= (unsigned char *)Mem::vmem_trans (buf_v);
//index= 0, roomleft= VMEM_PAGESIZE;           /* buffer is empty */

for (;;) {                                   /* write each line */
   struct line_header *h= line_fetch (sheet_to_save, line);
   unsigned char *s= (unsigned char *)(h+1); /* string begins after header */
   SHORT length= h->length;
   while (length > 0) {
      if (length < roomleft) {               /* copy line to buffer */
         memcpy (buf+index, s, length);
         DBG_SHEET_SAVE(dout<<" sheet_save line = °"<<s<<"° "<<endl);
         index += length;
         roomleft -= length;
         break;              /* eliminate setting length= 0  and retesting */
         }
      else {                            // copy partial line to output buf 
         memcpy (buf+index, s, roomleft);
         DBG_SHEET_SAVE(dout<<" sheet_save line = °"<<s<<"° "<<endl);
         length -= roomleft;
         s += roomleft;
         if (!mylib_write_text (f, buf, VMEM_PAGESIZE)) 
              goto write_error;
         index= 0;
         roomleft= VMEM_PAGESIZE;
         }
      }                                      // end while length > 0
   if (++line == linemax) break;             /* the loop's exit */
                                          
   if (roomleft < 2) {                       /* flush */
      if (!mylib_write_text (f, buf, VMEM_PAGESIZE-roomleft)) 
           goto write_error;
      index= 0;
      roomleft= VMEM_PAGESIZE;
      }
   memcpy (buf+index, "\r\n", 2);            // terminate line with CRLF 
   index += 2;
   roomleft -= 2;
   }                                         // end write all lines
if (index) {                          /* flush the buffer */
   if (!mylib_write_text (f, buf, index)) 
       goto write_error;
   }
if (file_write_eof) {    /* option set to write EOF char at end of file */
   mylib_write_text (f, (UCHAR *)&file_EOF_char, 1);
   }

mylib_close (f);
vmem_droppage (buf_v);
cur->changed= FALSE;
return 0;

write_error:                     /* report the error */
 mylib_close (f);
 mylib_delete (filename);
 vmem_droppage (buf_v);
 return 2;
}

/* **************************************** */
