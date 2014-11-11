/***************************************************************
      misc routines for TOW, mostly os/2 stuff
**************************************************************/

#include "usual.h"
             //#include "tow.h"
#include "basewin.h"
             //#include "tgetkey.h"
#include <string.h>
#include "test.h"

#ifdef TEST_APP
#include <stdio.h>
#include <stdlib.h>
#include <time.h>     // needed for clock()
#endif

#ifdef FUCKUP
#ifdef MEM_TEST
#include <stdlib.h>

FILE *log_file;
void LOG(const char *msg, unsigned val)
{
  static bool first_time = TRUE;

  if(first_time == TRUE) {
     log_file = fopen("core_err.log", "wt");  //create + write text
     if(log_file == NULL) {
       printf("ERR opening LOG FILE");
       exit(-3);
      }
   first_time = FALSE;
   }
   fprintf(log_file, "\n %s %x", msg, val);
   fflush(log_file);
}

void close_log(void)
{
  fclose(log_file);
} 
#else
  void LOG(const char *msg, unsigned val)
  {
  #pragma warn -par         // we don't use any  params
  }
  #pragma warn +par  
#endif
#endif

#ifdef TEST_APP
/************************************************************************/

void tdelay(int tenths)         // delay execution by .1 seconds
{
  clock_t tstart, tend;
  double diff_tenths;               
  bool done = FALSE;

  tstart = clock();
  while(!done) {
     tend = clock();                    // int div is a little inaccurate
     diff_tenths = (double) ((tend - tstart) / (CLK_TCK / 10));
//     if((double) ((tend - tstart) / (CLK_TCK / 10)) >= tenths) return;
     if(diff_tenths >= tenths) {
       done = TRUE;
     }
  }
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef KED
void simple_error_box (char const* message)
{
static char PressKey[]= "Press any Key";
int len= strlen(message);
if (len < sizeof PressKey) len= sizeof PressKey;
len += 4;
int xpos= (window_core::display_size().x - len)/2;
basewin t (5,xpos, 5,len);
t.clear (attr(WHITE,_BLACK), ' ');
t.show();
// >> border
t.put (1,2, message);
t.put (3,2, PressKey);
//key::get();  //pause
}

#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef KED
int string_to_long (const char* s, long& result)
{
/* parse 's' into a long, put result in 'result' parameter.
   on success, return -1 and 'result' contains answer.
   on failure, 'result' is unchanged and the position of the
   offending character is returned.

   format: <sign><digits>
   */
int pos= 0;
bool negative= FALSE;
// check for leading sign
if (s[0] == '-') {
   negative= TRUE;
   pos++;
   }
else if (s[0] == '+' || s[0] == ' ')
  pos++;
long n= 0L;
while (s[pos]) {
   if (s[pos] > '9' || s[pos] < '0') return pos; //error
   n *= 10;  //no test for overflow
   n += s[pos] - '0';
   pos++;
   }
result= n;
if (negative) n *= -1;
return -1;  //success
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

int string_to_int (const char* s, int& result)
{
long temp;
int retval= string_to_long (s, temp);
if (retval == -1) result= temp;
return retval;
}
#endif
/************************************************************************/
