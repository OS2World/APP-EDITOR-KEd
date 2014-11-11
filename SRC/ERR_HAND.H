/*     ERR_HANDLER.H          */

#ifndef ERR_HAND_H
#define ERR_HAND_H

//#define INCL_DOS
//#define INCL_DOSPROCESS
//#define INCL_NOPMAPI
             //#define INCL_DOSSIGNALS
              //#define INCL_DOSEXCEPTIONS

//#include <os2.h>

#include <setjmp.h>

typedef struct {
   struct _EXCEPTIONREGISTRATIONRECORD* volatile prev_structure;
   _ERR* volatile ExceptionHandler;
   jmp_buf env;
} MYEXCEPTIONREGISTRATIONRECORD, *PMYEXCEPTIONREGISTRATIONRECORD;

extern ULONG APIENTRY MyExceptionHandler(PEXCEPTIONREPORTRECORD pReportRecord,
                                     PEXCEPTIONREGISTRATIONRECORD pRegRecord,
                              PCONTEXTRECORD pContextRecord, PVOID pReserved);

extern unsigned long cdecl set_int24(void);
void err_exit(const char *msg, unsigned short number);

#endif



