/*************************************************************** 
       USUAL.H   global definitions

********************************************************************/
#ifndef USUAL_H
#define USUAL_H

#define CHAR    char            /* ch  */
#define SHORT   short           /* s   */
#define LONG    long            /* l   */
typedef unsigned char  UCHAR;   /* uch */
typedef unsigned short USHORT;  /* us  */
typedef unsigned long  ULONG;   /* ul  */
#define LOUCHAR(w)      ((UCHAR)(w))
#define HIUCHAR(w)      ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l)     ((USHORT)((ULONG)l))
#define HIUSHORT(l)     ((USHORT)(((ULONG)(l) >> 16) & 0xffff))
/* Combine l & h to form a 32 bit quantity. */
#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | (((ULONG)((USHORT)(h))) << 16)))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))


#define KED  1                  // use to eliminate unused base lib stuff

//#define PACKUP                // NO packup for now   6-91 
//#define NO_HELP 1              // eliminate hyperhelp 8-24-92
//#define NO_ASCIIBOX 1          // no ascii 8-24-92
//#define NO_SETTING 1             // try external settings prog 6/17/94
#define WORD_WRAP              // KEd v 2.17 August 95
//#define REGEXP 1               // no reg exp now 7/94
// define USE_FILELIST           // no filelist
#define NO_COMMANDO 1        /* comment this out to include COMMANDO  */
#define MEMCPY_C   1
#define ERR_HAND 1
#define MOVE_WIN 1

typedef unsigned char byte;
typedef short bool;
#define TRUE 1
#define FALSE 0
typedef unsigned long vptr;
const PARAGRAPH = 16;              /* paragraph of RAM = 16 bytes */

    /* Max length of filenames, including the terminating null character */
#define FNAME_LEN 255            // mod 8-11-94
//extern USHORT MAX_NAME_LEN;
enum udlr {UP, DOWN, LEFT, RIGHT};
extern bool really_exit;          // global, final exit if TRUE

//void fillwords (void *dest, USHORT value, USHORT count); // misc.cpp
extern void my_assert (short, char *, char *,short);
#define assert(expr) my_assert(expr,#expr,__FILE__,__LINE__)

extern unsigned rc;     // OS/2 API return codes

/************************  TESTING  ***************************************/
/*       SEVERITY is defined as:
                                1 major bug
                                2 minor bug
                                3 warning bug
         In general, Beta tests should only involve "Severity 2" bugs,
         and shipping version should have only "Severity 3" bugs

         The coorelation to BUG FLAGS is that most of the time during
         development, "3" is on, and only if a major bug surfaces will
         I turn on the "1" or "2" FLAGS.  As a side effect, if a routine
         is generally correct, you should use "BUG FLAG 1" for that, and
         if a routine has problems, use "BUG FLAG 3" for that.
***************************************************************************/
//#define TEST_APP   1            // timing func in tmisc.cpp

//#define MEM_TEST   1           // checkmem() in tow_os2.cpp, and LOG in tmisc
                               // writes to "mem_err"

#define TESTING                // MASTER DEBUG write "logfile"s for debug

//#define KED_BETA 1             // beta test, turn on/off logfiles
//

#ifdef TESTING                  // most flags on for now
//  #define MEM_DBG     1
//  #define VID_DBG     1
/// #define VMEM_DBG    1
//  #define WIN_DBG     1         // old def, need to change source
//  #define WIN_DBG1    1
#define WIN_DBG2    1
//  #define WIN_DBG3    1
//  #define WIN_SIZE1   1
//  #define HYPR_DBG1     1       // hyperhelp
  #define SHEET_DBG   1
//  #define SHEET_CPY_DBG   1
  #define SHEET_SAVE_DBG   1
  #define BLOCK_DBG1   1
  #define MS_BLOCK_DBG1  1
  #define VIEW_DBG1     1
  #define KEY_DBG     1           // init_keys and ed_cmd funcs
//  #define KEY_DBG1     1           // init_keys and ed_cmd funcs
  #define KEY_DBG2     1           // init_keys and ed_cmd funcs
//  #define NUMLOCK_DBG     1           // set/unset numlock
  #define LOAD_DBG    1           // file load funcs "view_fil.c and file.c"
  #define EXT_DBG       1          // default file extensions
//  #define INCHAR_DBG1  1         // getkey.c inchar()
  #define INCHAR_DBG2  1         // getkey.c inchar()
//  #define INCHAR_Q_DBG  1
//  #define FILE_DBG    1         // file in/out old def
//  #define FILE_DBG1    1         // file in/out low-level buffers
  #define FILE_DBG2    1           // file in/out mid-level funcs
//  #define FILE_TAB_DBG    1         // tab expand/compress file funcs
  #define DIR_DBG       1         // dirlist and sort funcs
//  #define STATUS_DBG  1           // status line funcs
  #define INI_DBG     1         // file setting and help funcs
//  #define SHELL_DBG     1
//  #define SEARCH_DBG   1        // view_search funcs
//  #define MACRO_DBG   1
// #define ERROR_DBG     1          // file I/O critical errors
// #define PROMPT_DBG    1          // prompt boxes
  #define TRAP_DBG    1          // debug exception handling
  #define BUG_DBG     1
#endif
/***********************************************************************/
#endif              // USUAL_H





