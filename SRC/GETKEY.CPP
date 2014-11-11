/*****************************************************
File: GETKEY.C      Copyright 1989 by Dlugosz Software
   Functions to read a key, with support for
   background processing and macros.  

   v 3.0  9-11-94
          moved all thread and semaphore functions to read_que() and
          write_que(). eliminated global key_rec mykey_rec and numkeys.

          found that under certain circumstances (what ??) that read_que
          would exit ReqSem(hkbdsem) with NO valid keys. Modified
          read_que to reenter the wait if the key.value = 0.

*****************************************************/
// #define INCL_SUB
#define INCL_KBD
#define INCL_DOSPROCESS
//#define INCL_DOSDEVICES        // DosDevIoctl function for NUMLOCK set
#define INCL_DOSSEMAPHORES
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "getkey.h"
#include "setting.h"
#include "keycodes.h"  // need CTRLKEY def
#include <string.h>
#include "initkeys.h"

#include "test.h"

static struct key_rec key_rec_t;
struct key_rec& mykey_rec = key_rec_t;

void inchar(void *);                // keyboard read thread
static struct key_rec& read_que(void);
static void   write_que(struct key_rec& krec);
static bool check_time(ULONG keytime);

#define KBUFSIZE 1024
static key_rec  keyqueue[KBUFSIZE];     // holds key strokes until processed
static USHORT rpos = 0;         // read position in key queue
static USHORT wpos = 0;         // write position in key queue
static USHORT num_keys = 0;
bool   inchar_ready = FALSE;
bool   OldKey = FALSE;
char   memkey = 0x00;
//bool   ORG_NUMLOCK = FALSE;

extern HMTX hkbdsem;
extern HMTX hkquesem;
extern HMTX hshell_ok_sem;
extern bool recording;          // macro is being recorded
extern bool play_macro;         // macro is being played

/*******************************************************************
         KBDKEYINFO fsState 
#define KbdShift(kbci) ((kbci.fsState & LEFTSHIFT)||(kbci.fsState & RIGHTSHIFT))
#define KbdCtrl(kbci)  ((kbci.fsState & LEFTCTRL) ||(kbci.fsState & RIGHTCTRL))
#define KbdAlt(kbci)   ((kbci.fsState & LEFTALT)  ||(kbci.fsState & RIGHTALT))
#define KbdCapsL(kbci) (kbci.fsState & CAPS)
#define KbdNumL(kbci)  (kbci.fsState & NUMLOCK)
*********************************************************************/

extern char *macro;
bool (* key_preprocess)(USHORT)= NULL;     // set to load HELP by hyprhelp.c
USHORT KeyState = 0x0000;

#ifdef FUCKUP
char read_port(void)
{
  char fookey;
  _EDX = 0x00000060;
  RPORTB();
  fookey = _CL;
//  DBG_INCHAR1(dout<<" read_port memkey = "<<hex<<fookey<<dec<<endl);
  return fookey;
}
#endif

static void write_que(struct key_rec& krec)
/******************************************************************
     write a keyboard char entry into keyqueue 
       called only by INCHAR thread
*****************************************************************/
{
 rc = DosRequestMutexSem(hkquesem, 500);  // wait .5 sec for Sem
 if(rc != 0) {
    DosBeep(1000,200);
    DBG_Q(dout<<"WRITE ReqMutex kqueusem  rc = "<<hex<<rc<<dec<<endl);
 }

 if(wpos == KBUFSIZE) {
    wpos = 0;
 }

 keyqueue[wpos].value = krec.value;
 keyqueue[wpos].state = krec.state;
 keyqueue[wpos].ktime = krec.ktime;
 keyqueue[wpos].owner = krec.owner;
 num_keys++;
 wpos++;

 DBG_Q(dout<<"WRITE wpos = "<<(wpos-1)<<" numkeys = "<<num_keys);
// DBG_Q(dout<<"  owner = "<<keyqueue[wpos-1].owner<<"   hex key = "<<hex<< keyqueue[wpos-1].value<<dec);
 DBG_Q(dout<<"                 "<<"key = 0x"<<hex<< keyqueue[wpos-1].value<<dec);
 DBG_Q(dout<<"  key = "<< LOUCHAR(keyqueue[wpos-1].value)<<endl);

 if(num_keys > KBUFSIZE /2) {        // queue too full
    rc = DosReleaseMutexSem(hkquesem);       //let read_que empty the queue
    DBG_Q(dout<<"WRITE ERROR QUEUE_TOO_FULL "<<endl);
    DosSleep(700);
 }
 else {
    rc = DosReleaseMutexSem(hkquesem);       // and let read_que continue
 }
}

static struct key_rec& read_que(void)
/************************************************************
    read a char from key queue, and zero that entry in the queue array
    called by getkey() from thread MAIN
    10-13-94
    if a context switch occurs AFTER inchar() releases hkbdsem, then
    read_que will get garbage. After I read a key from the queue, 
    I zero the value, and then if I read a zero, I assume an context
    switch occured and just go back to the read loop  CONRAD!!
*******************************************************************/
{
 static struct key_rec read_key_rec;
 rc = DosRequestMutexSem(hkquesem, 500);  // wait .5 sec for Sem
 if(rc != 0) {
    DosBeep(200,200);
    DBG_Q(dout<<"READ   ReqMutex kqueusem  rc = "<<hex<<rc<<dec<<endl);
 }

 if(rpos == KBUFSIZE)  
      rpos = 0;           // wrap around to beginning

 if(num_keys == 0) {
retry:
    rc = DosReleaseMutexSem(hkquesem);        // let write_que continue
    DBG_Q(dout<<'\n'<<"READ  waiting num_keys = "<<num_keys<<endl);
    rc = DosRequestMutexSem(hkbdsem, -1L);  // and wait forever for key
    rc = DosReleaseMutexSem(hkbdsem);         // let inchar go

    rc = DosRequestMutexSem(hkquesem, 500);  // wait .5 sec for Sem was .2
    if(rc != 0) {
       DosBeep(200,200);
       DBG_Q(dout<<"READ   ReqMutex kqueusem  rc = "<<hex<<rc<<dec<<endl);
    }
 }                                         // end num_keys == 0
 read_key_rec.value = keyqueue[rpos].value;
 keyqueue[rpos].value = 0x0000;
 read_key_rec.state = keyqueue[rpos].state;
 read_key_rec.ktime = keyqueue[rpos].ktime;
// read_key_rec.owner = keyqueue[rpos].owner;
 KeyState = (read_key_rec.state & 0x0020) ? 0x0020 :  0x0000; 
                     // I'm not sure this will work. What if a 0x00 is read
                     // from the queue? I don't want to wait for kbdsem then.
 if(read_key_rec.value == 0x0000) {          // 0x00 not valid, so retry
    DBG_Q(dout<<"READ  retry  "<<endl);
    goto retry;
 }

 num_keys = (num_keys == 0) ? 0 : num_keys - 1;
 rpos++;

  if(check_time(read_key_rec.ktime)) {
     DBG_INCHAR1(dout<<" check_time FALSE says memkey = "<<hex<<memkey<<dec<<endl);
     read_key_rec.owner = FALSE;    // clipboard paste
  }
  else {
     DBG_INCHAR1(dout<<" check_time  TRUE says memkey = °"<<hex<<memkey<<dec<<"°"<<endl);
     read_key_rec.owner = TRUE;     // key was depressed
  }

 DBG_Q(dout<<"READ  rpos = "<<(rpos-1)<< " numkeys = "<<num_keys);
// DBG_Q(dout<<"  owner = "<<read_key_rec.owner<<"   hex key = "<<hex<< read_key_rec.value<<dec);
 DBG_Q(dout<<" hdw kbd = "<<(read_key_rec.owner ? "TRUE " : "FALSE ")<<" key = 0x"<<hex<< read_key_rec.value<<dec);
// DBG_Q(dout<<"  time = "<<read_key_rec.ktime<<"   hex key = "<<hex<< read_key_rec.value<<dec);
 DBG_Q(dout<<"  key = "<< LOUCHAR(read_key_rec.value)<<endl);

 rc = DosReleaseMutexSem(hkquesem);        // let write_que continue
 return read_key_rec;
}
/**********************************************************************/
bool check_time(ULONG keytime)
{
  static ULONG oldtime = 0;

  if(oldtime < keytime) {
    oldtime = keytime;         // store the new key time
    return FALSE;              // regular key stroke, not clipboard
  }
  else {
    oldtime = keytime;         // update time
    return TRUE;               // regular key
  }
}        
/**********************************************************************/

void inchar(void *)        /* this function runs as a separate  */
{                          /* thread started in edit.c main()   */
   static struct key_rec in_key_rec;
   KBDKEYINFO kbci;
   KBDINFO kbdinfo;
   kbdinfo.cb = sizeof(kbdinfo);
   USHORT  xch = 0x00;                         /* extended key char   */
//   char    memkey = 0x00;

   rc = KbdFlushBuffer(0);
   rc = DosRequestMutexSem(hkbdsem, 200);  
   inchar_ready = TRUE;                        // let main() continue
   rc = DosReleaseMutexSem(hkbdsem);        

   for (;;) {
      rc = DosRequestMutexSem(hkbdsem, 200);  // getkey should release Sem fast
      if(rc != 0) {
         DosBeep(2000,100);
         DBG_Q(dout<<" INCHAR ReqMutex kbdsem rc = "<<hex<<rc<<dec<<endl);
      }
      rc = KbdCharIn(&kbci, IO_WAIT, 0);
#ifdef FUCKUP
      if(check_time(kbci.time)) {
         DBG_INCHAR1(dout<<" RPORTB FALSE says memkey = "<<hex<<memkey<<dec<<endl);
         in_key_rec.owner = FALSE;    // clipboard paste
      }
      else {
         DBG_INCHAR1(dout<<" RPORTB  TRUE says memkey = °"<<hex<<memkey<<dec<<"°"<<endl);
        in_key_rec.owner = TRUE;     // key was depressed
      }
#endif
//       WARNING, MODIFYING CRUCIAL CODE 8-23-95  
//
//      if(kbci.chChar == 0xE0){      /* ext key */
//         kbci.chChar = 0x00;
//      }
      //    comment line above if you need to distinguish 101 keyboard keys
      //    from "normal" keys. The keys between the normal keyboard and the
      //    numeric keypad return a scan code of 0xE0. I don't find it useful
      //    to identify those keys separately for the editor.

      xch = (kbci.chChar | (kbci.chScan << 8));
      in_key_rec.value = xch;
      in_key_rec.state = kbci.fsState;
      in_key_rec.ktime = kbci.time;

      DBG_INCHAR2(dout<<"inchar key state = "<<hex<<(in_key_rec.state));
      DBG_INCHAR2(dout<<" combined = "<<(in_key_rec.value)<<dec);
      DBG_INCHAR2(dout<<" key hi = "<<HIUCHAR(in_key_rec.value));
      DBG_INCHAR2(dout<<" key lo = "<<LOUCHAR(in_key_rec.value)<<endl);
                                                                    
      if(xch != 0x00){                                 // we have a key
         write_que(in_key_rec);                        // put it in queue
      }
      rc = DosReleaseMutexSem(hkbdsem);        // let getkey go anyway

      if((xch == 0x5500) || (xch == 0x5600)){    // prepare for OS shell
         rc = DosReleaseMutexSem(hshell_ok_sem);
         DosSleep(100);                 // let shell go
         while((rc = DosRequestMutexSem(hshell_ok_sem, 100)) != 0){
            DosSleep(100);
            DBG_INCHAR1(dout<<"inchar waiting "<<xch<<endl);
            rc = KbdCharIn(&kbci, IO_NOWAIT, 0);  // this work??? 10=8=93
         }
      }   // end if shell 
  }      // end for(;;)
}


struct key_rec& getexkey(void)
{
/*********************************************************************
   NOTE: "goto reject" in this function originally "threw away" the keystroke
          I make sure it's gone by setting it to 0x00

          May 15, 1994
          Change this to getexkey, which returns pointer
          to key_rec struct and create a shell getkey() function which
          just returns a USHORT 

          May 17, 1994
          changed pointers to references
*******************************************************************/
reject:
 mykey_rec.value = 0x0000;
 mykey_rec.state = 0x0000;

if (play_macro) {                 
    DBG_MACRO(dout<<" in getexkey and calling read_from_macro"<<endl);
    mykey_rec = read_from_macro();
    return mykey_rec;
}
   mykey_rec = read_que();                  // block here
   DBG_KEY1(dout<<" getexkey state = "<<hex<<mykey_rec.state);
   DBG_KEY1(dout<<" getexkey value = "<<mykey_rec.value<<dec<<endl);

if (key_preprocess != '\0')
    if( !(*key_preprocess)(mykey_rec.value)){         // call that func
        goto reject;
    }

if ((recording) && (mykey_rec.value > 0x0000) ) {   // kybd macro code
    DBG_MACRO(dout<<" in getexkey, adding "<<hex<<mykey_rec.value<<dec<<" to buf"<<endl);
    put_key_in_macro_buf(mykey_rec);
}
                             
return mykey_rec;
}


/************************************************************************/
USHORT getkey()
{
struct key_rec simp_key_rec;

reject:                            /* start over if rejected */
USHORT  mykey = 0x0000;       // for "goto reject" lines, throw away mykey

  simp_key_rec = getexkey();
  mykey = simp_key_rec.value;
   DBG_KEY1(dout<<" getkey value = 0x"<<hex<<simp_key_rec.value<<dec<<endl);
  if (key_preprocess != '\0') {
    if( !(*key_preprocess)(mykey)){  // help system leaves a key in buffer
        goto reject;
    }
  }
return mykey;
}

/********************************************************************
  look up key in extended key table and call the function 
  associated with that key.  

  if Shift and "Grey" key is depressed, use another table of functions.

  returns:  USHORT QUIT or 0   
***********************************************************************/
USHORT do_command(struct key_rec& krec)
{
  USHORT i;
  SHORT retval = 0x0000;
  USHORT key = krec.value;
  USHORT kstate = krec.state;
  UCHAR hibyte = HIUCHAR(key);
  UCHAR lobyte = LOUCHAR(key);
  fptr  do_func  = 0;
  sfptr sdo_func = 0;

//      DBG_KEY1(dout<<'\n'<<"do_cmd key = "<<hex<< key);
//      DBG_KEY1(dout<<"    key hi = "<<HIUCHAR(key));
//      DBG_KEY1(dout<<"    key lo = "<<LOUCHAR(key)<<dec<<endl);

//      DBG_KEY(dout<<"    index = "<<dec<< i<<endl);
//      DBG_KEY(dout<<"intern key_struct num "<<i<< " = "<<
//          KeyNameVal[i].name<<" val = "<<hex<<KeyNameVal[i].num<<
//          "  keystate = "<<kstate<<dec<<endl);

         /* only map Ctrl-Key to +115 if hibyte < 73 because Ctrl-Arrow keys
            do not need to be remapped. See KeyDef.cpp.... */
  if((CTL_KEY(kstate)) && (hibyte < 73)) {                 // control + key
     i = 116 + lobyte;                     // was 116
  }
  else
     i = hibyte - 0x10;

  if(SHIFT_KEY(kstate) && (lobyte == 0xE0)) {
     i = hibyte - 0x48;           
     sdo_func = ShiftKeyNameVal[i].func;   // no range check...
     retval = sdo_func(key);
     return retval;
  }


  DBG_KEY2(dout<<"intern key num "<<i<<" val = "<<hex<<KeyNameVal[i].num<<
        "  keystate = "<<kstate<<dec<<" name =  "<<KeyNameVal[i].funcname<<endl);

//  DBG_KEY2(dout<<"intern key num "<<i<<" val = "<<hex<<KeyNameVal[i].num<<
//           dec<<" name =  "<<KeyNameVal[i].funcname<<endl);

  if((i >= 0) && (i <= 147)) {
     do_func = KeyNameVal[i].func;
  }
  if(do_func != NULL) {              // better be sure <G>
    retval = do_func();
    retval = CMD_HANDLED;
  }
  if(key == 0x2D00) {   // AltX
    really_exit = TRUE;
    retval = do_func();
    DBG_KEY2(dout<<"  key = "<<hex<<key<<dec<<" and Quitting"<<endl);
    if(retval == TRUE)
       return CMD_QUIT;
    else {
       really_exit = FALSE;
       return retval;
    }
  }
  else {
//    DBG_KEY1(dout<<" key = "<<hex<<key<<dec<< "  retval = "<<retval<<endl);
    DBG_KEY2(dout<<"do_cmd result ="<<retval<<endl);
    return retval;
  }
}


/********************************************************************/
bool key_ready()
{
//  if (OldKey) 
//     return TRUE;
//  else 
//     return TRUE;
   return FALSE;
}

