// Tow OS/2 compatible routines  
//
//    Don Radick  12/93
//
//    compatible with Borland C++ OS/2 v 1.0

#define INCL_VIO
#define INCL_KBD
#define INCL_NOPMAPI
#include <os2.h>
#include <string.h>
#include "usual.h"
#include "tow.h"

#include "test.h"

#ifdef MEMCPY_C                             // func prototype in misc.h
void fillwords (void* dest, USHORT value, USHORT count)
{

  unsigned short* mydest = (unsigned short*)dest;    // works fine
  while (count--) 
    *(mydest++) = value;
}
#endif

/**********************************************************************/
extern "C" USHORT video_int (USHORT AX,USHORT BX,USHORT CX,USHORT DX)
{
  int Service = AX >> 8;
  VIOMODEINFO   viomode;
  VIOCURSORINFO cursordata;
  USHORT usRow, usCol;

  switch(Service)
  { 
     case 15:
             viomode.cb = sizeof(viomode);      // get vid mode
             rc = VioGetMode(&viomode, 0);
             DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" video int rc = "<<rc<<endl);
             if(viomode.fbType & VGMT_OTHER)
                return  0x0700;
             else
                return  0x0300;
         //  break;                  // unreachable code warn
     case 2:
             cursordata.attr   = -1;         // hide cursor
             cursordata.yStart =  0;         // top
             cursordata.cx     = 0;          /* use default width  */
             cursordata.cEnd   = 0;          /* bottom  */
             rc = VioSetCurType(&cursordata, 0);
             DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" video int rc = "<<rc<<endl);
             break;
     case 3:
             usCol = (DX & (0x00 << 8));   /* zero out 8 high bits   */
             usRow = (DX >>8);  /* shift right - zeros 8 high bits */
             rc = VioSetCurPos(usRow-1, usCol, 0);
             DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" video int rc = "<<rc<<endl);
             break;

             // video_int (0x0200,0,0,(height<<8));  //move cursor off screen
             // video_int (0x0200,0,0,old_cursor_pos);  //restore cursor pos
     default:
             break;
   }
return 0;
#pragma warn -par         // we don't use "BX, CX, DX" params
}
#pragma warn +par  

/***************************************************************************/
extern "C" unsigned read_cursor_position()
{
  USHORT usRow;
  USHORT usCol;
  USHORT pos;

  rc = VioGetCurPos( &usRow, &usCol, 0);
  pos = ((usRow << 8) | usCol);
  return pos;
}


#ifndef KED
/************************************************************************/
extern "C" int inchar(void)
{
   KBDKEYINFO kbci;
   int xch;

   rc = KbdCharIn(&kbci, IO_WAIT, 0);    
   if(kbci.chChar == 0xe0)
//      xch = (kbci.chChar | (kbci.chScan << 8));
      xch = (kbci.chScan | (0x01 << 8));
   else
      xch = (kbci.chChar);                  // the base TOW lib likes this

   return xch;
}
#endif

/***************************************************************************
     MOD 1-1-95   
         added basic v2.10 calls to avoid VioShowBuf() bug.

         continued garbage until I added:
              viopbuf.pBuf = (PCH)(viomode.buf_addr); 
         at which point I get NO screen output

***************************************************************************/
bool meta_window::setup()
{     // set up structure based on current mode
   unsigned short _far16 **temp = &address;        // BCC optimize workaround
   VIOMODEINFO   viomode;
   VIOCONFIGINFO vioconfig;

    display_iscolor= FALSE;

    viomode.cb = sizeof(viomode);
    rc = VioGetMode(&viomode, 0);
    vidbufsize  = viomode.buf_length;

    screen_save= new unsigned char[vidbufsize];

    rc = VioReadCellStr(screen_save, &vidbufsize, 0, 0, 0);  

    rc = VioGetBuf((PULONG16)temp , &vidbufsize, 0);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" video getbuf rc = "<<rc<<endl);

    screensize= 2* viomode.row * viomode.col;          // mod 1-1-95
    vioconfig.cb = sizeof(vioconfig);
    rc = VioGetConfig(0, &vioconfig, 0);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" video get config rc = "<<rc<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid address    = 0x"<<hex<<(ULONG)address<<dec<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid buf_length = "<<(viomode.buf_length)<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid adapter    = "<<(vioconfig.adapter)<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid display    = "<<(vioconfig.display)<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid rows       = "<<(viomode.row)<<endl);
    DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid cols       = "<<(viomode.col)<<endl);

if((vioconfig.adapter == 0x0000) || (vioconfig.display == 0))
   display_iscolor = FALSE;
else
   display_iscolor = TRUE;

height = viomode.row;
width = viomode.col;
range.lr.x= width-1;
range.lr.y= height-1;
range.ul.x = 0;
range.ul.y = 0;
return TRUE;
}

/***************************************************************************/
extern "C" void fill_screen(unsigned fillval)
{
//  unsigned char bcell[2];
  char bcell[2];
  bcell[0] = LOUCHAR(fillval);
  bcell[1] = HIUCHAR(fillval);

  rc = VioScrollUp(0, 0, -1, -1, -1, bcell, 0);
  DBG_VID(dout<<__FILE__<<" line "<<__LINE__<<" vid scrollup  rc = "<<rc<<endl);
}

#ifdef MEM_DBG
ostream& checkmem2(ostream& os)
{
  USHORT stack_free = 0;
  stack_free = stackavail();
  os <<  "    Stack  "<< stack_free << "   free bytes  ";
  switch(_heapchk()) {
        case _HEAPEMPTY:
             os << " No HEAP allocs "; 
             break;
        case _HEAPBADNODE:
             os << " HEAP          CORRUPTED";            
             break;
        case _HEAPOK:
             os << " HEAP OK ";
             break;
   }
//  os << endl;
  return os;
}
#else
void checkmem2(void) { }
#endif




















