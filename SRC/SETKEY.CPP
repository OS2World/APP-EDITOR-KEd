/*********************************************************************
   SETKEY.CPP             KEd 2.15   Nov 27, 1994

   simple keybd_setup() and   numlock_set()  functions

********************************************************************/
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES        // DosDevIoctl function for NUMLOCK set
#define INCL_NOPMAPI
#include <os2.h>

#include "usual.h"
#include "getkey.h"
#include "setting.h"

#include "test.h"

bool   ORG_NUMLOCK = FALSE;


/************************************************************************/
bool keybd_setup(void)            // setup special keyboard settings
{
  bool ok = FALSE;
  DBG_INI(dout<<" keybrd_setup org settings.numlock_on = "<<settings.numlock_on<<endl);
  if(settings.numlock_on == FALSE) {
      DBG_INI(dout<<" keybrd_setup setting NUMLOCK OFF "<<endl);
      ok = numlock_set(FALSE);
  }
  else {
      DBG_INI(dout<<" keybrd_setup setting NUMLOCK ON "<<endl);
      ok = numlock_set(TRUE);
  }
  return ok;    
}


/************************************************************************/
bool numlock_set(bool on_off)
{
struct  keyparams {
        USHORT ShiftState;
        BYTE Nls;
        } Data;

HFILE  KbdHandle;
ULONG act;
ULONG plio, dlio;

ULONG  KEYBOARD_CATEGORY   =   0x0004;
ULONG  KBD_GETSHIFTSTATE   =   0x0073;
ULONG  KBD_SETSHIFTSTATE   =   0x0053;
static first_time = TRUE;

   rc = DosOpen((PBYTE)"kbd$", &KbdHandle, &act, 0L, 0, FILE_OPEN,
        OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_NOINHERIT,
        0L);
   if(rc) {
       DBG_NUML(dout<<"set_numlock Kbd$ open fail rc = "<<rc<<endl);
       return FALSE;
   }
    plio = 0L;
    dlio = sizeof(Data);
    rc = DosDevIOCtl(KbdHandle, KEYBOARD_CATEGORY, KBD_GETSHIFTSTATE,
         NULL, plio, &plio,                       /* Input data  */
         (PVOID) &Data, dlio, &dlio);             /* Output data */
    if(rc) {
       DBG_NUML(dout<<"set_numlock get state fail rc = "<<rc<<endl);
       return FALSE;
    }
    if(first_time) {
       ORG_NUMLOCK = Data.ShiftState & NUMLOCK;    // save org state
       DBG_NUML(dout<<"set_numlock org Kbd ShiftState = 0x"<<hex<<(Data.ShiftState)<<dec<<endl);
       DBG_NUML(dout<<"set_numlock first_time ORG_NUMLOCK = 0x"<<hex<<ORG_NUMLOCK<<dec<<endl);
       first_time = FALSE;
    }

//    DBG_NUML(dout<<"set_numlock setting NUMLOCK  = "<<on_off ? "TRUE " : "FALSE "<<endl);
    DBG_NUML(dout<<"set_numlock setting NUMLOCK  = "<<on_off <<endl);
//    DBG_NUML(dout<<"set_numlock org Kbd ShiftState = 0x"<<hex<<(Data.ShiftState)<<dec<<endl);

    on_off ? (Data.ShiftState |= NUMLOCK) : (Data.ShiftState &= ~NUMLOCK);
    Data.Nls        = 0x00;

    plio = sizeof(Data);
    dlio = 0L;
    rc = DosDevIOCtl(KbdHandle, KEYBOARD_CATEGORY, KBD_SETSHIFTSTATE,
                    (PVOID) &Data, plio, &plio,
                    NULL, dlio, &dlio);
    if(rc) {
       DBG_NUML(dout<<"set_numlock set state fail rc = "<<rc<<endl);
      return FALSE;
    }
    else
      on_off ? (KeyState = 0x0020) : (KeyState = 0x0000); 

    DBG_NUML(dout<<"set_numlock  Kbd ShiftState NOW  = 0x"<<hex<<(Data.ShiftState)<<dec<<endl);
    rc = DosClose(KbdHandle);
    if(rc) {
       DBG_NUML(dout<<"set_numlock kbd close fail rc = "<<rc<<endl);
       return FALSE;
    }
return TRUE;
}
