/********************************************************************
file: BLITT.CPP               Copyright 1989-1991 by Dlugosz Software
  part of TOW  Text Output Windows
  blitt and scroll for window_core class
********************************************************************/
#include "usual.h"
#include "tow.h"
#include "basewin.h"
#include <string.h>  //memmove() etc.

#include "test.h"

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

inline short int min (int a, int b)
{
return (b<a)?b:a;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::blitt(
   short int y_dest, short int x_dest,
   short int v_size, short int h_size,
   window_core* from_w, short int y_start, short int x_start)
{
twocorner temp;
temp.ul.y= Location.ul.y + y_dest;
temp.lr.y= temp.ul.y + v_size;
temp.ul.x= Location.ul.x + x_dest;
temp.lr.x= temp.ul.x + h_size;
twocorner box1;                      // box1 is extent of dest
if (!intersection (Location, temp, &box1)) 
    return;
temp.ul.y= from_w->Location.ul.y + y_start;
temp.lr.y= temp.ul.y + v_size;
temp.ul.x= from_w->Location.ul.x + x_start;;
temp.lr.x= temp.ul.x + h_size;
twocorner box2;                      // box2 is extent of source
if (!intersection (Location, temp, &box2)) 
   return;

DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

                              // adjust size to account for any clipping
v_size= min(box1.lr.y-box1.ul.y, box2.lr.y-box2.ul.y);
h_size= min(box1.lr.x-box1.ul.x, box2.lr.x-box2.ul.x);
h_size *=2;                   //now in bytes
const short int wid_s= from_w->width();
const short int wid_d= width();
unsigned short* dest= get_buffer(y_dest,x_dest);
unsigned short* source= from_w->get_buffer(y_start,x_start);

if (y_dest > y_start)  {      //copy bottom-to-top
   DBG_WIN2(dout<< "wcore::blitt copy bottom_to_top v_size = "<<v_size<<endl);
   dest += wid_d*v_size;
   source += wid_s*v_size;
   while (v_size--) {
      dest -= wid_d;
      source -= wid_s;
      memmove (dest, source, h_size);
      }
   }
else {                         //copy top to bottom
   DBG_WIN2(dout<< "wcore::blitt copy top_to_bottom v_size = "<<v_size<<endl);
   while (v_size--) {
      memmove (dest, source, h_size);
      dest += wid_d;
      source += wid_s;
      }
   }

DBG_WIN2(dout<< "wcore::blitt source : ");
DBG_WIN2(dout<< " ul.x = "<<box2.ul.x<<" ul.y =  "<< box2.ul.y<<endl);
DBG_WIN2(dout<< "wcore::blitt dest : ul.x = "<<box1.ul.x <<" ul.y = "<< box1.ul.y<<endl);
DBG_WIN2(dout<< "****************************************************"<<endl);

display(box1);
}

/************************************************************************
    scroll window in any of the 4 directions, for specified count
    adjust count so that it doesn't overflow the window buffers.
    IE: if a large count is specified, only clear the window

    count should always be a positive number
*************************************************************************/
void window_core::scroll (enum tudlr direction, int count)
{
int y_dest  = border_width[Up];
int x_dest  = border_width[Left];
int y_start = border_width[Up];
int x_start = border_width[Left];

DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
switch (direction) {
   case Down:   y_dest += count;
         //      y_dest = min(count, height());
               if(y_dest > height())
                  y_dest = height();
               DBG_WIN2(dout<<"wcore::scroll Down y_dest = "<<y_dest<<endl);
               break;
   case Left:  
               x_start += count;      // was x_dest
               if(x_start < width())
                  x_start = width();
               DBG_WIN2(dout<<"wcore::scroll Left x_dest = "<<x_dest<<endl);
               break;
   case Up:    y_start += count;
          //     y_start = min(count, height());
               if(y_start > height())
                  y_start = height();
               DBG_WIN2(dout<<"wcore::scroll Up y_start = "<<y_start<<endl);
               break;
   case Right:
               x_dest += count;   
               if(x_dest > width())
                  x_dest = width();
               DBG_WIN2(dout<<"wcore::scroll Right x_start = "<<x_start<<endl);
               break;
   }

int x_size= width()-border_width[Right] -min(x_start,x_dest);  // correct
int y_size= height()-border_width[Down] -min(y_start,y_dest);  // correct

DBG_WIN2(dout<<"wcore::scroll x_size = "<<x_size<<"  y_size = "<<y_size<<endl);
DBG_WIN2(dout<< "wcore::scroll x_start = "<<x_start<<" y_start = "<<y_start);
DBG_WIN2(dout<< "  x_dest = "<<x_dest<<" y_dest = "<< y_dest<<endl);

window_core::blitt (y_dest,x_dest, y_size, x_size, this, y_start, x_start);
}
