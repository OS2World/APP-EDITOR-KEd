/*  File VBWIND.H   */

#ifndef VBWIND_H
#define VBWIND_H

#ifndef TOW_H
#include "tow.h"
#endif

#ifdef FUCKUP
class Vid 
{
private:
//    USHORT w_mode;     // color = 1
    USHORT buf_length;
public:
    Vid() { }   //ctor
    ~Vid() { }
    bool init(void);
   static   void screen_swap(void);
};
#endif
//extern  Vid vid;                         // define in EDIT.C
                                       /* implemented in WDISP.C */
void draw_screen (twocorner* box);
#endif
