/**************************************************************************

          ERR_HAND.CPP
           
                  handles a "graceful" exit if critical things fail

          v 1.0  8-6-93

          v.1.1 11-21-93 added GPF fault handler
          v 2.0 8-31-94  added code from Dave Mooney, IBM toronto
************************************************************************/
#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#define INCL_DOSSIGNALS
#define INCL_DOSEXCEPTIONS
#include <os2.h>

#include "usual.h"
#include "err_hand.h"
#include "mylib.h"
#include "getkey.h"
#include "misc.h"      // num2str
#include <stdio.h>
#include <string.h>    // strcat

#include "basewin.h"
#include "test.h"

static basewin trapw(10,Screen.get_screen_width()- 20, 10,10);

void err_exit(const char *msg, USHORT number)
{
 if(msg) {
    char errstr[80];
    char numstr[10], *s;
    strcpy(errstr, msg);
    mylib_write_to_screen("\nA critical error occurred!");
    mylib_write_to_screen(" Please report this to Kudzu support");
    if(number != 0) {
       s = num2str_fast(numstr, number, 4);
       strcat(errstr, "   ");
       strcat(errstr, s);
    }
    mylib_write_to_screen(errstr);
    mylib_write_to_screen("\n");
//    DosExit(EXIT_PROCESS, number);
 }

}


extern ULONG APIENTRY MyExceptionHandler(PEXCEPTIONREPORTRECORD pReportRecord,
                                     PEXCEPTIONREGISTRATIONRECORD pRegRecord,
                              PCONTEXTRECORD pContextRecord, PVOID pReserved)
{
#pragma warn -par         // we don't use pReserved
//   ULONG retcode;
   char outbuf[55];
   DBG_TRAPS(dout<< "in exception handler, opening window"<<endl);
   trapw.clearwin(YELLOW,_RED, ' ');
   trapw.border(window_core::Double, attr(YELLOW,_RED));
   trapw.put(8,2,"Press Any Key to exit ");

   DBG_TRAPS(dout<< "in exception handler, about to jump"<<endl);

   if(EH_EXIT_UNWIND & pReportRecord->fHandlerFlags) {
       DBG_TRAPS(dout<< "in exception handler, EXIT_UNWIND detected"<<endl);
       return XCPT_CONTINUE_SEARCH;
   }

   if(EH_UNWINDING & pReportRecord->fHandlerFlags) {
       DBG_TRAPS(dout<< "in exception handler, EXIT_UNWINDING detected"<<endl);
       return XCPT_CONTINUE_SEARCH;
   }

   if(EH_NESTED_CALL & pReportRecord->fHandlerFlags) {
       DBG_TRAPS(dout<< "in exception handler, NESTED_CALL detected"<<endl);
       return XCPT_CONTINUE_SEARCH;
   }

   if(pReportRecord->ExceptionNum == XCPT_UNWIND                   ||
      pReportRecord->ExceptionNum == XCPT_PROCESS_TERMINATE        ||
      pReportRecord->ExceptionNum == XCPT_GUARD_PAGE_VIOLATION     ||
      pReportRecord->ExceptionNum == XCPT_UNABLE_TO_GROW_STACK     ||
      pReportRecord->ExceptionNum == XCPT_BREAKPOINT               ||
      pReportRecord->ExceptionNum == XCPT_SINGLE_STEP) {
         DBG_TRAPS(dout<< "in exception handler, MISC_TRAPS detected"<<endl);        
         return XCPT_CONTINUE_SEARCH;
   }

   if(pReportRecord->ExceptionNum == XCPT_ACCESS_VIOLATION) {
      DBG_TRAPS(dout<< "memory access violation"<<endl);
      trapw.put(2,2,"memory access violation! ");
      sprintf(outbuf, "trap number 0X%X ",pReportRecord->ExceptionNum);
      trapw.put(3,2,outbuf);
      sprintf(outbuf, "   EIP = %8.8X", pContextRecord->ctx_RegEip);
      trapw.put(4,2,outbuf);
      trapw.show();
      getkey();
      trapw.hide();
      longjmp(((PMYEXCEPTIONREGISTRATIONRECORD)pRegRecord)->env, -1);
   }

   if(pReportRecord->ExceptionNum == XCPT_SIGNAL)  {
      switch(pReportRecord->ExceptionInfo[0]) {
         case XCPT_SIGNAL_BREAK:
         case XCPT_SIGNAL_INTR:
              trapw.put(2,2,"Control-Break  Detected ");
              DBG_TRAPS(dout<< "in exception handler, received Ctrl-C"<<endl);
              break;
         case XCPT_SIGNAL_KILLPROC:
              trapw.put(2,2,"DosKillProcess Detected ");              
              DBG_TRAPS(dout<< "in exception handler, received Kill_Process"<<endl);
              break;
      }
      trapw.show();
      getkey();
      trapw.hide();
      longjmp(((PMYEXCEPTIONREGISTRATIONRECORD)pRegRecord)->env, -1);
   }

   DBG_TRAPS(dout<< "in exception handler, unhandled trap = ");
   DBG_TRAPS(dout<<hex<< (pReportRecord->ExceptionNum )<<dec<<endl);
//   return retcode;
   return (pReportRecord->ExceptionNum);
}
#pragma warn +par       


 
