/*****************************************************
File: TOOL.C       Copyright 1992 by Kudzu Software
      implements file load for compile log file

      rev 1.0    12-27-92
*****************************************************/
#define INCL_DOS
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "vmem.h"
#include "redbox.h"
#include "misc.h"
#include "sheet.h"
#include "view.h"
#include "dirlist.h"    // need cwd
#include "mymenu.h"      // need closewindow()
#include <stdlib.h>      // need atol
#include <stdio.h>       /* need puts() */
#include <string.h>

#define redbox_locus 30

#include "test.h"

struct _comperr{
      char *str;       // line containing error: file: line: message
      char *filename;  // filename of "bad" file
      ULONG linenum;   // line number of error
} compile_error;

/* I should probably not keep line pointer in the struct..... */

/* the line looks like this   */
/* Warning tool.c 68: 'cur' is assigned a value that is never used in function */
/* Error tool.c 72: Pointer to structure required on left side of -> or ->* */

/* **************************************** */

char *get_log_line (void)
{
/*   get the line that the cursor is on, return a pointer to chars  */

  char *str;
struct line_header *line_hdr;
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
struct mark_rec *m= get_mark (v->cursor);

line_hdr= (struct line_header *)Mem::vmem_trans (get_line (m->row));
str= (char *)(line_hdr+1);

 compile_error.str = str;
return str;

}
/* **************************************** */

static bool parse_line(void)
{
/*    grab the filename and line number from the input line   */
  char *tempstr;
  LONG linenum;
                        /* look for space delimited substrings   */
  tempstr = strtok(compile_error.str, " ");  // warning or error 

  tempstr = strtok(NULL, " ");              // get filename
  if(tempstr == NULL)
    return FALSE;
  compile_error.filename = tempstr;         // store it
  tempstr = strtok(NULL, " ");              // get linenum
  if(tempstr == NULL)
    return FALSE;
  linenum = atol(tempstr);                  // cvt char * to long
  compile_error.linenum = linenum;

return TRUE;
}

/**********************************************************************/
bool load_prob_file(void)
{
/*  load the file that generated the error, scroll Up 10 lines,
    and  position on the error line */

   bool result;

   get_log_line();
 if( parse_line() == TRUE) {

   new_sheet();
   new_standard_view (1);
   result = view_load_file (compile_error.filename, cwd, NULL);
   if(result == TRUE) {
      view_gotoline(compile_error.linenum-1);   // goto line in err
      view_flush(active_view);
      view_scroll(UP, 10);                       // make view scroll
    }
   else {
    closewindow();
   }
 }
   return result;
}



#ifdef FUCKUP
#ifdef TESTING
extern FILE *mylog;

void writelog(char *format, ...)
{
 va_list args;
 va_start(args, format);             /* Initialize variable argument list */
 vfprintf(mylog, format, args);       /* Write message to output file */
 va_end(args);
 fflush(mylog);
}
#endif
#endif




