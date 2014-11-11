/*****************************************************
file: ASSERT.C      Copyright 1989 by Dlugosz Software
   window-based assert() function
*****************************************************/

#include "usual.h"
#include "vmem.h"
                       //#include "window.h"
#include "getkey.h"
#include "keycodes.h"
                        //#include "borders.h"
#include "misc.h"
#include "setting.h"
#include <stdlib.h>   /* need abort() for assert */

#include "basewin.h"

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void my_assert (SHORT expr, char* test, char* filename, SHORT linenum)
{
if (!expr) {
   basewin assertw (4, 60,10 ,10);
   assertw.clearwin(settings.colors[REDBOX_COLORS][NORMAL_COLOR], ' ');
   assertw.border(window_core::Double);    

//   window_t w= create_window (4,60, 10,10);
   char image[6];
   SHORT key;
//   clear_window (w, 0,3,0,59, (settings.colors[REDBOX_COLORS][NORMAL_COLOR] <<8)|' ');
//   window_border (w, fat_border, settings.colors[REDBOX_COLORS][BORDER_COLOR]);
//   display_on (w);
//   put_string (w, 0, 2, "Assert Fails!");
//   put_string (w, 1,1, filename);
//   put_string (w, 1,15,num2str (image, linenum, 5, 0));
//   put_string_len (w, 2, 1, test, 58);
   assertw.put(0, 2, "Assert Fails!");
   assertw.put(1,1, filename);
   assertw.put(1,15,num2str (image, linenum, 5, 0));
   assertw.put(2, 1, test);
   assertw.show();

   key= getkey();  /* pause */
   if (key == kESC) 
       abort();
//   destroy_window (w);

   }
return;
}
