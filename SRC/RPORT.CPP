//
//        C wrapper for asm read port func.
//
//        used to read status byte for keyboard control
//
#include "usual.h"

extern "C" {
void  __pascal __far16 WPORTW(void);
void  __pascal __far16 WPORTB(void);
void  __pascal __far16 RPORTW(void);
void  __pascal __far16 RPORTB(void);
}

byte read_port(void)
{
  _EDX = 0x00000060;
  RPORTB();
  return _CL;
}

