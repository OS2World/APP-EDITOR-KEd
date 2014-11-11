
PKUNZIP (tm) FAST!  Extract Utility  Ver 1.01-OS/2 Prot Mode  7-21-89
Copyright 1989 PKWARE Inc.  All Rights Reserved.  PKUNZIP/h for help.

Searching ZIP: AUG27SRC.ZIP
  Exploding: KEYCODES.H  <to console>

/*       KEYCODES.H                              */

/*   abbreviations are fairly obvious            */
/*
      k    = key
      kc   = Ctrl   with another key
      ks   = Shift  with another key
      kAlt = Alt    with another key
      k***e=        on 101 enhanced keyboard, the separate 6 key block
                    that has "home end pgup, pgdn and separate cursor
                    key block beneath
*/
#ifndef KEYCODES_H
#define KEYCODES_H

#define  Left      0x4b00        /* "special" keys */
#define  Right     0x4d00 
#define  Up        0x4800 
#define  kDOWN      0x5000 
#define  kcLEFT     0x7300 
#define  kcRIGHT    0x7400
#define  kBS        8 
#define  kcBS       127 
#define  kCR        13 
#define  kINS       0x5200 
#define  kNUMLOCK   0x52E0       /* wild ass value - MAY CAUSE PROBS */
#define  kDEL       0x5300 
#define  kHOME      0x4700 
#define  kEND       0x4f00 
#define  kPGUP      0x4900 
#define  kPGDN      0x5100 
#define  kcPGUP     0x8400 
#define  kcPGDN     0x7600 
#define  kTAB       9 
#define  kBACKTAB   271 
#define  kESC       0x001B 

#define  Lefte      0x4bE0        /*  keys on enhanced kybd only*/
#define  Righte     0x4dE0 
#define  Upe        0x48E0 
#define  kDOWNe      0x50E0 
#define  kPGUPe      0x49E0 
#define  kPGDNe      0x51E0 
#define  kcPGUPe     0x84E0 
#define  kcPGDNe     0x76E0 
#define  kHOMEe      0x47E0 
#define  kENDe       0x4fE0 

#define  kF         0x3A00       /* some modules have this.... */
#define  kF1        0x3B00 
#define  kF2        0x3C00 
#define  kF3        0x3D00 
#define  kF4        0x3E00 
#define  kF5        0x3F00 
#define  kF6        0x4000 
#define  kF7        0x4100 
#define  kF8        0x4200 
#define  kF9        0x4300 
#define  kF10       0x4400 

#define  ksF        0x0009      /* shifted keys              */
#define  ksF1       0x5400 
#define  ksF2       0x5500 
#define  ksF3       0x5600 
#define  ksF4       0x5700 
#define  ksF5       0x5800 
#define  ksF6       0x5900 
#define  ksF7       0x5A00 
#define  ksF8       0x5B00 
#define  ksF9       0x5C00 
#define  ksF10      0x5D00 

#define  kAltF7     0x6E00 

#define  kAltA      0x1E00 
#define  kAltB      0x3000 
#define  kAltC      0x2E00 
#define  kAltD      0x2000 
#define  kAltF      0x2100 
#define  kAltH      0x2300 
#define  kAltI      0x1700 
#define  kAltJ      0x2400 
#define  kAltK      0x2500 
#define  kAltL      0x2600 
#define  kAltM      0x3200 
#define  kAltN      0x3100 
#define  kAltO      0x1800 
#define  kAltP      0x1900 
#define  kAltQ      0x1000 
#define  kAltS      0x1F00 
#define  kAltT      0x1400 
#define  kAltU      0x1600 
#define  kAltV      0x2F00 
#define  kAltX      0x2D00 
#define  kAltZ      0x2C00 
#define  kcF        349 

#endif

