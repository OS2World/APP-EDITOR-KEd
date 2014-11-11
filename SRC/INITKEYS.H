/************************************************************************
           INITKEYS header, holds definitions for default key mapping
             and sets up user - defined keyboard remapping 

    rev 2.0  March 16, 1994
    I'll need some functions to return and take parameters, so I'll
     mimc the EMACS convention and always return an SHORT and take
     a ULONG param. This way I can pack 2 ints into the ULONG,
     and most functions return an SHORT or bool

***********************************************************************/

#ifndef INITKEYS_H
#define INITKEYS_H

bool load_keys(void);

#define CMD_QUIT 0xffff
#define CMD_HANDLED 0xfffe

#define MAX_KEYS 150            // should do it

//typedef void (* fptr)();            // function pointer

typedef SHORT (* fptr)(void);    // function pointer returns SHORT
typedef SHORT (* sfptr)(USHORT);// function pointer takes USHORT returns SHORT
const FUNC_NAME_LEN = 20;
const KEY_NAME_LEN  = 10;

struct kname_val {
    char      name[KEY_NAME_LEN];      // char "Esc"
    fptr      func;                    // function pointer
    char      funcname[FUNC_NAME_LEN]; // func name "printmenu" (will change)
    USHORT    num;                     // ushort 0x0027
    };

struct shift_kname_val {
    char      name[KEY_NAME_LEN];      // char "Esc"
    sfptr     func;                    // function pointer
    char      funcname[FUNC_NAME_LEN]; // func name "printmenu" (will change)
    USHORT    num;                     // ushort 0x0027
    };

//const int MAX_KEYS = ( /sizeof(kname_val))  // look how emacs does this

extern struct kname_val KeyNameVal[MAX_KEYS];
extern struct shift_kname_val ShiftKeyNameVal[11];

//USHORT do_command(USHORT key);
USHORT do_command(struct key_rec& krec);

#endif
