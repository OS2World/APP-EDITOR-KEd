/*************************************************************
            KEYCODES.H                             

      abbreviations are fairly obvious           

      k    = key
      kc   = Ctrl   with another key
      ks   = Shift  with another key
      kAlt = Alt    with another key
      k***e=        on 101 enhanced keyboard, the separate 6 key block
                    that has "home end pgup, pgdn and separate cursor
                    key block beneath

     KEY VALUES and STRINGS in KEYDEF.C

*********************************************************************/
#ifndef KEYCODES_H
#define KEYCODES_H

const UCHAR      CTRLKEY  =  0xA0;  // for KEd 2.10 see keydef.c and getkey.c
const UCHAR      ALTKEY   =  0xB0;  // for KEd 2.10 see keydef.c and getkey.c

const USHORT     kLeft    =  0x4b00;        /* "special" keys */
const USHORT     kRight   =  0x4d00; 
const USHORT     kUp      =  0x4800; 
const USHORT     kDown    =  0x5000; 

const USHORT     kLefte    =  0x4bE0;        /*  keys on enhanced kybd only*/
const USHORT     kRighte   =  0x4dE0; 
const USHORT     kUpe      =  0x48E0; 
const USHORT     kDowne   =  0x50E0; 
const USHORT     PageUpe  =  0x49E0; 
const USHORT     PageDowne=  0x51E0; 
//const USHORT     BegFilee =  0x84E0; 
//const USHORT     EndFilee =  0x76E0; 
const USHORT     kHOMEe   =  0x47E0; 
const USHORT     kENDe    =  0x4fE0; 

const USHORT     kBS      =  0x0e08; 
const USHORT     kcBS     =  127; 
                                       //const USHORT     Enter    =  13; 
const USHORT     Enter      =  0x1C0D; 
const USHORT     GREY_Enter =  0xE00D; 
                                      //const USHORT     kESC     =  0x001B; 
const USHORT     kESC     =  0x011B; 
const USHORT     Space    =  0x3920; 
const USHORT     kHOME    =  0x4700; 
const USHORT     kEND     =  0x4f00; 
const USHORT     PageUp   =  0x4900; 
const USHORT     PageDown =  0x5100; 
//const USHORT     kTAB     =  0x0009; 
const USHORT     kTAB     =  0x0f09; 
const USHORT     kF3      =  0x3D00; 
const USHORT     kF4      =  0x3E00;
const USHORT     FileList =  0x4400; 
const USHORT     kINS     =  0x5200; 
const USHORT     kDEL     =  0x5300; 
const USHORT     Help     =  0x3B00; 
const USHORT     kF2      =  0x3C00; 
const USHORT     kAltA    =  0x1E00; 
const USHORT     ksF7     =  0x5A00; 
const USHORT     kAltF7   =  0x6E00; 
const USHORT     kcF7     =  0x6400; 
/**********************************************************************/

#ifdef FUCKUP

//const USHORT     kINS     =  0x5200; 
const USHORT     kNUMLOCK =  0x52E0;       /* wild ass value - MAY CAUSE PROBS */
//const USHORT     kDEL     =  0x5300; 
const USHORT     BegFile  =  0x8400; 
const USHORT     EndFile  =  0x7600; 
const USHORT     kBACKTAB =  271; 

const USHORT     Lefte    =  0x4bE0;        /*  keys on enhanced kybd only*/
const USHORT     Righte   =  0x4dE0; 
const USHORT     Upe      =  0x48E0; 
const USHORT     kDOWNe   =  0x50E0; 
const USHORT     PageUpe  =  0x49E0; 
const USHORT     PageDowne=  0x51E0; 
const USHORT     BegFilee =  0x84E0; 
const USHORT     EndFilee =  0x76E0; 
const USHORT     kHOMEe   =  0x47E0; 
const USHORT     kENDe    =  0x4fE0; 

const USHORT     kF       =  0x3A00;       /* some modules have this.... */
//const USHORT     Help     =  0x3B00; 
//const USHORT     kF2      =  0x3C00; 
//const USHORT     kF3      =  0x3D00; 
//const USHORT     kF4      =  0x3E00; 
const USHORT     BlockStart= 0x3F00; 
const USHORT     BlockEnd =  0x4000; 
const USHORT     kF7      =  0x4100; 
const USHORT     kF8      =  0x4200; 
const USHORT     kF9      =  0x4300; 
//const USHORT     FileList =  0x4400;

const USHORT     ksF      =  0x0009;      /* shifted keys              */
const USHORT     ksF1     =  0x5400; 
const USHORT     ksF2     =  0x5500; 
const USHORT     ksF3     =  0x5600; 
const USHORT     ksF4     =  0x5700; 
const USHORT     ksF5     =  0x5800; 
const USHORT     ksF6     =  0x5900; 
const USHORT     ksF7     =  0x5A00; 
const USHORT     ksF8     =  0x5B00; 
const USHORT     ksF9     =  0x5C00; 
const USHORT     ksF10    =  0x5D00; 

const USHORT     kAltF7   =  0x6E00; 

//const USHORT     kAltA    =  0x1E00; 
const USHORT     kAltB    =  0x3000; 
const USHORT     kAltC    =  0x2E00; 
const USHORT     kAltD    =  0x2000; 
const USHORT     kAltF    =  0x2100; 
const USHORT     kAltH    =  0x2300; 
const USHORT     kAltI    =  0x1700; 
const USHORT     kAltJ    =  0x2400; 
const USHORT     kAltK    =  0x2500; 
const USHORT     kAltL    =  0x2600; 
const USHORT     PrevFile =  0x3200; 
const USHORT     NextFile =  0x3100; 
const USHORT     kAltO    =  0x1800; 
const USHORT     kAltP    =  0x1900; 
const USHORT     QuitFile =  0x1000; 
const USHORT     Save     =  0x1F00; 
const USHORT     kAltT    =  0x1400; 
const USHORT     kAltU    =  0x1600; 
const USHORT     kAltV    =  0x2F00; 
const USHORT     kAltX    =  0x2D00; 
const USHORT     kAltZ    =  0x2C00; 
const USHORT     kcF      =  349; 

const USHORT     DelLine  =  0x0019;
const USHORT     FindNext =  0x000E;     // Ctrl-N
const USHORT     FindPrev =  0x0016;     // Ctrl-V
const USHORT     DelEOL   =  0x0005;
const USHORT     kcB      =  0x0002;
const USHORT     kcP      =  0x0010;
const USHORT     PrevWord =  0x7300; 
const USHORT     NextWord =  0x7400;

#endif

#endif
