/**********************************************************************
        TEST.H    holds defines for various levels of debug output
                  in all cases, the output is written to a file
                  actual code is in TEST.CPP

        rev 1.0   May 5, 1994
                  I probably need several levels of DBG_BUG switches
                  based on severity. Critical, Warning, and Minor
        rev 2.0   October 22, 1994
                  added BETA identifier to selectively run beta debug
                  log. 
                  NOT! can't get it to work.....
*********************************************************************/
#ifndef TEST_H
#define TEST_H

#include <fstream.h>
#include <iomanip.h>

#ifdef TESTING                     // TESTING defined in usual.h
extern ofstream dout;
extern ofstream dout2;             // getkey read_que() output
extern ofstream memout;            // stack and heap checking
#endif                             // end TESTING

//#ifdef TESTING
//   #ifdef KED_BETA
//     #define 
//   #endif
//#endif

#ifdef MEM_DBG                 // mem tests in tow_os2
//#include <stdio.h>              // need printf()
//#include <stdlib.h>             // need itoa()
  #ifdef __OS2__
     #include <malloc.h>
  #else
     #include <alloc.h>
  #endif
//  extern void LOG(const char *msg, unsigned val);
//  extern void close_log(void);
#endif
//  extern void checkmem(const char *msg);


#ifdef MEM_DBG                     // heapchk functions
#define DBG_MEM(expr) expr
ostream& checkmem2(ostream& os);
#else                               
#define DBG_MEM(expr)
#endif

#ifdef VID_DBG                     // low level video functions
#define DBG_VID(expr) expr
#else                               
#define DBG_VID(expr)
#endif


#ifdef VMEM_DBG                     // vmem functions
#define DBG_VMEM(expr) expr
#else                               
#define DBG_VMEM(expr)
#endif

#ifdef WIN_DBG                     // TOW win functions  old def
#define DBG_WIN(expr) expr
#else                               
#define DBG_WIN(expr)
#endif

#ifdef WIN_DBG1                     // TOW win functions MAJOR bug
#define DBG_WIN1(expr) expr
#else                               
#define DBG_WIN1(expr)
#endif

#ifdef WIN_DBG2                     // TOW win functions  MINOR bugs
#define DBG_WIN2(expr) expr
#else                               
#define DBG_WIN2(expr)
#endif

#ifdef WIN_DBG3                     // TOW win functions WARNING bugs
#define DBG_WIN3(expr) expr
#else                               
#define DBG_WIN3(expr)
#endif

#ifdef WIN_SIZE1                     // window sizing stuff
#define DBG_SIZE1(expr) expr
#else                               
#define DBG_SIZE1(expr)
#endif

#ifdef HYPR_DBG1                  // HyperHelp functions
#define DBG_HYPR1(expr) expr
#else                               
#define DBG_HYPR1(expr)
#endif

#ifdef SHEET_DBG                  // KEd sheet functions
#define DBG_SHEET(expr) expr
#else                               
#define DBG_SHEET(expr)
#endif

#ifdef SHEET_CPY_DBG                  // KEd sheet functions
#define DBG_SHEET_CPY(expr) expr
#else                               
#define DBG_SHEET_CPY(expr)
#endif

#ifdef SHEET_SAVE_DBG                  // KEd sheet functions
#define DBG_SHEET_SAVE(expr) expr
#else                               
#define DBG_SHEET_SAVE(expr)
#endif

#ifdef BLOCK_DBG1                  // KEd block functions
#define DBG_BLOCK1(expr) expr
#else                               
#define DBG_BLOCK1(expr)
#endif

#ifdef MS_BLOCK_DBG1                  // Microsoft block functions
#define DBG_MS_BLOCK1(expr) expr
#else                               
#define DBG_MS_BLOCK1(expr)
#endif

#ifdef VIEW_DBG1                  // KEd view functions
#define DBG_VIEW1(expr) expr
#else                               
#define DBG_VIEW1(expr)
#endif

#ifdef FILE_DBG                   // file  debug old def
#define DBG_FILE(expr) expr
#else                               
#define DBG_FILE(expr)
#endif

#ifdef FILE_DBG1                   // file  in/out low-level buffer stuff
#define DBG_FILE1(expr) expr
#else                               
#define DBG_FILE1(expr)
#endif

#ifdef FILE_DBG2                   // file  in/out mid-level 
#define DBG_FILE2(expr) expr
#else                               
#define DBG_FILE2(expr)
#endif

#ifdef EXT_DBG                   // file  default extensions
#define DBG_EXT(expr) expr
#else                               
#define DBG_EXT(expr)
#endif

#ifdef FILE_TAB_DBG                   // file  (load, save, misc) functions
#define DBG_FILE_TAB(expr) expr
#else                               
#define DBG_FILE_TAB(expr)
#endif

#ifdef DIR_DBG                   // file directory funcs
#define DBG_DIR(expr) expr
#else                               
#define DBG_DIR(expr)
#endif

#ifdef STATUS_DBG                   // status functions
#define DBG_STATUS(expr) expr
#else                               
#define DBG_STATUS(expr)
#endif


#ifdef INI_DBG                   // INI functions ie: load settings and help
#define DBG_INI(expr) expr
#else                               
#define DBG_INI(expr)
#endif

#ifdef KEY_DBG                   // keyboard  functions
#define DBG_KEY(expr) expr
#else                               
#define DBG_KEY(expr)
#endif

#ifdef KEY_DBG1                   // keyboard  functions
#define DBG_KEY1(expr) expr
#else                               
#define DBG_KEY1(expr)
#endif

#ifdef KEY_DBG2                   // keyboard  functions
#define DBG_KEY2(expr) expr
#else                               
#define DBG_KEY2(expr)
#endif

#ifdef NUMLOCK_DBG                   // Numlock set/unset  functions
#define DBG_NUML(expr) expr
#else                               
#define DBG_NUML(expr)
#endif

#ifdef LOAD_DBG                   // file and sheet load functions
#define DBG_LOAD(expr) expr
#else                               
#define DBG_LOAD(expr)
#endif

#ifdef INCHAR_DBG1                   // inchar() keyboard  functions
#define DBG_INCHAR1(expr) expr
#else                               
#define DBG_INCHAR1(expr)
#endif

#ifdef INCHAR_DBG2                   // inchar() keyboard  functions
#define DBG_INCHAR2(expr) expr
#else                               
#define DBG_INCHAR2(expr)
#endif

#ifdef INCHAR_Q_DBG                   // inchar() keyboard  functions
#define DBG_Q(expr) expr
#else                               
#define DBG_Q(expr)
#endif

#ifdef SHELL_DBG                   // shell functions
#define DBG_SHELL(expr) expr
#else                               
#define DBG_SHELL(expr)
#endif

#ifdef SEARCH_DBG                   // search functions (view_sch.c, search.c)
#define DBG_SEARCH(expr) expr
#else                               
#define DBG_SEARCH(expr)
#endif

#ifdef MACRO_DBG                   // key macros
#define DBG_MACRO(expr) expr
#else                               
#define DBG_MACRO(expr)
#endif

#ifdef ERROR_DBG                   // file I/O critical errs
#define DBG_ERR(expr) expr
#else                               
#define DBG_ERR(expr)
#endif


#ifdef PROMPT_DBG                   // prompt boxes
#define DBG_PROMPT(expr) expr
#else                               
#define DBG_PROMPT(expr)
#endif

#ifdef TRAP_DBG                   // misc  functions, leave in for beta test
#define DBG_TRAPS(expr) expr
#else                               
#define DBG_TRAPS(expr)
#endif

#ifdef BUG_DBG                   // misc  functions, leave in for beta test
#define DBG_BUGS(expr) expr
#else                               
#define DBG_BUGS(expr)
#endif

#endif     // end TEST_H

