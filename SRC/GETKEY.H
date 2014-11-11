#ifndef GETKEY_H
#define GETKEY_H

struct key_rec {
      USHORT value;
      USHORT state;
      ULONG  ktime;
      bool   owner;    // TRUE = key from kbd, FALSE = key from clipboard
};

extern struct key_rec& mykey_rec;

void dummy(void);  /* does nothing */
extern void key_macro (const char *), (*KeyBackground)(void);
//extern USHORT OldKey;
USHORT getkey(void);
//struct key_rec *getexkey(void);      // get ext key info (shift state)
struct key_rec& getexkey(void);      // get ext key info (shift state)

extern bool inchar_ready;
extern bool (* key_preprocess)(USHORT key);
extern char KeyComment, KeyEscape;
extern USHORT KeyStatus; /* 1-macro loaded  2-macro playing  4-recording */
                         /* clear bit 2 to pause macro, set to resume */
                         /* clear bit 4 to pause recording */
bool key_ready(void);    /* see if a key is ready */

//extern void key_capture (char* buf, SHORT buflen, SHORT stopkey, 
//                            void (* callback) (SHORT status, SHORT len));

extern bool   ORG_NUMLOCK;
extern USHORT KeyState;
                  //     1 = ?
                  //     0x02 = NUMLOCK ON
                  //     4 = CONTROL (pressed w/ another key)
bool keybd_setup(void);
bool numlock_set(bool on_off);
void numlock_toggle(void);
void numlock_toggle2(SHORT mode);
                                          // KEYMAC.CPP
//void put_key_in_macro_buf(USHORT mykey);            // 
void put_key_in_macro_buf(key_rec mykeyrec);            // 
//USHORT  read_from_macro (void);
key_rec  read_from_macro (void);

//         KBDKEYINFO fsState 
#define LEFTSHIFT               0x0002
#define LEFTCTRL                0x0100
#define LEFTALT                 0x0200
#define CAPS                    0x0040
#define NUMLOCK                 0x0020
#define SCROLLLOCK              0x0010
#define INSERT                  0x0080
#define RIGHTALT                0x0800
#define RIGHTCTRL               0x0400
#define RIGHTSHIFT              0x0001
#define ECHO                    0x0001
#define RAW                     0x0004
#define COOKED                  0x0008

#define KbdShift(kbci)      ((kbci.fsState & LEFTSHIFT) || (kbci.fsState & RIGHTSHIFT))
#define KbdCtrl(kbci)       ((kbci.fsState & LEFTCTRL)  || (kbci.fsState & RIGHTCTRL))
#define KbdAlt(kbci)        ((kbci.fsState & LEFTALT)   || (kbci.fsState & RIGHTALT))
#define KbdCapsL(kbci)      (kbci.fsState & CAPS)
#define KbdNumL(kbci)       (kbci.fsState & NUMLOCK)

#define CTL_KEY(State)       ((State & LEFTCTRL) || (State & RIGHTCTRL))
#define ALT_KEY(State)       ((State & LEFTALT)  || (State & RIGHTALT))
#define SHIFT_KEY(State)    ((State & LEFTSHIFT)  || (State & RIGHTSHIFT))

#endif
