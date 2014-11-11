/********************************************************************
file: WMOVE.CPP               Copyright 1986-1991 by Dlugosz Software
   part of TOW   Text Output Winodws
   window movement functions for class window_core

   May need to change some "integer"s to "short"s for OS/2, but
   it seems to work fine at present   RADICK
********************************************************************/
#include "usual.h"
#include "tow.h"
#include <string.h>

#include "test.h"

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::move (tudlr direction, int shiftsize)
{
if (!displayed()) {
   switch (direction) {
      case Left: shiftsize *= -1;
      case Right:
         Location.ul.x += shiftsize;
         Location.lr.x += shiftsize;
         break;
      case Up: shiftsize *= -1;
      case Down:
         Location.ul.y += shiftsize;
         Location.lr.y += shiftsize;
         break;
      }
   return;
   }
/* do normal, long version */
twocorner zoneA, zoneB, nl;
int zonewidth;
bool rt_fwd= TRUE;

switch (direction) {
   case Left: shiftsize *= -1;
   case Right:
      nl.ul.y= zoneA.ul.y= zoneB.ul.y= Location.ul.y;
      nl.lr.y= zoneA.lr.y= zoneB.lr.y= Location.lr.y;
      nl.ul.x= Location.ul.x + shiftsize;
      nl.lr.x= Location.lr.x + shiftsize;  /* new location */
      if (shiftsize < 0) {
         rt_fwd= FALSE;
         shiftsize *= -1;
         }
      if ((zonewidth= shiftsize) > Size.x) zonewidth= Size.x;
      zonewidth--;
      if (rt_fwd) {
         zoneB.ul.x= (zoneB.lr.x= nl.lr.x) - zonewidth;
         zoneA.lr.x= (zoneA.ul.x= Location.ul.x) + zonewidth;
         }
      else {  // left
         zoneA.ul.x= (zoneA.lr.x= Location.lr.x) - zonewidth;
         zoneB.lr.x= (zoneB.ul.x= nl.ul.x) + zonewidth;
         }
   break;
   case Up: shiftsize *= -1;
   case Down:
      nl.ul.x= zoneA.ul.x= zoneB.ul.x= Location.ul.x;
      nl.lr.x= zoneA.lr.x= zoneB.lr.x= Location.lr.x;
      nl.ul.y= Location.ul.y + shiftsize;
      nl.lr.y= Location.lr.y + shiftsize;  // new location
      if (shiftsize < 0) {
         rt_fwd= FALSE;
         shiftsize *= -1;
         }
      if ((zonewidth= shiftsize) > Size.y) zonewidth= Size.y;
      zonewidth--;
      if (rt_fwd) {    // down
         zoneA.lr.y= (zoneA.ul.y= Location.ul.y) + zonewidth;
         zoneB.ul.y= (zoneB.lr.y= nl.lr.y) - zonewidth;
         }
      else {  // up
         zoneB.lr.y= (zoneB.ul.y= nl.ul.y) + zonewidth;
         zoneA.ul.y= (zoneA.lr.y= Location.lr.y) - zonewidth;
         }
      break;
   }
blank_screen (zoneA);
Location= nl;
unstomp (zoneA, Next);
stomp (zoneB, Next, 0);
make_mask();
display (Location);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool bigger;

unsigned short* window_core::adjust_mask (int mode, int newval, int* newmaskwid)
{     // returns a new mask
if (!eclipse_mask) return 0; // no mask to adjust
unsigned short* newmask;
if (mode==1) {                     // vert-- make mask longer or shorter
   if (newval > Size.y) bigger= TRUE;
   unsigned copysize= maskwidth * (bigger ? Size.y : newval);
   newmask= new unsigned short[newval*maskwidth];
   memcpy (newmask, eclipse_mask, 2*copysize);
   if (bigger)
      memset (newmask+copysize, 0, 2* (newval - Size.y) * maskwidth);
   *newmaskwid= maskwidth;  // same as it was
   }
else {                           /* horz-- make mask wider or narrower */
   short int newwidth= ((15+1+newval)>>4);
   unsigned rows= Size.y;
   if (newval > Size.x) bigger= TRUE;
   unsigned length= (bigger) ? maskwidth : newwidth;
   unsigned totalsize= newwidth*rows;  //used in next 2 lines
   newmask= new unsigned short[totalsize];
   if (bigger) memset (newmask, 0, 2*totalsize);
   unsigned short* sourcerow= eclipse_mask;
   unsigned short* destrow= newmask;
   while (rows--) {
      memcpy (destrow, sourcerow, length);
      destrow += newwidth;
      sourcerow += maskwidth;
      }
   *newmaskwid= newwidth;
   }
return newmask;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

twocorner* window_core::adjust_savearea (int mode, int newval)
/* reallocates savearea, adjusts size & location,
   and returns pointer to the affected zone. */
{
static twocorner zone;  //pointer to this is returned
unsigned short* new_savearea;
// bigger is global
// if I got one global var, why bother with the other parameters?
// why not make them all global, since this is just a piece of
// a larger function that has been broken up.

zone= Location;
if (mode==1) {                                          /* vert */
   unsigned copysize= Size.x *(bigger ? Size.y : newval);
   new_savearea= new unsigned short [Size.x * newval];
   memcpy (new_savearea, save_area, 2*copysize);
   zone.ul.y= bigger ? Location.lr.y + 1 : Location.ul.y+newval;
   Location.lr.y= Location.ul.y + newval -1;
   if (bigger) zone.lr.y= Location.lr.y;
   range.lr.y += newval - Size.y;
   Size.y= newval;
   }
else {                                                /* horz mode */
   unsigned row= Size.y;
   unsigned length= (bigger) ? Size.x : newval;
   new_savearea= new unsigned short[row*newval];
   unsigned short* sourcerow= save_area;
   unsigned short* destrow= new_savearea;
   while (row--) {
      memcpy (destrow, sourcerow, 2*length);
      destrow += newval;
      sourcerow += Size.x;
      }
   zone.ul.x= bigger ? Location.lr.x + 1 : Location.ul.x+newval;
   Location.lr.x= Location.ul.x + newval -1;
   if (bigger) zone.lr.x= Location.lr.x;
   range.lr.x += newval - Size.x;
   Size.x= newval;
   }
delete save_area;
save_area= new_savearea;
return &zone;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::resize (unsigned mode, unsigned newval, USHORT fillval)
{                      /* resize modes: 1:vert, 2:horz */
bigger= FALSE;
int newmaskwid;
unsigned short* mask= adjust_mask (mode, newval, &newmaskwid);
twocorner* zone= adjust_savearea (mode, newval);
if (displayed())
   if (bigger) {
      delete eclipse_mask;
      eclipse_mask= mask;
      maskwidth= newmaskwid;  //new mask in place
      extend_mask (mask, Location, *zone);
      stomp (*zone, Next, 0);
      }
   else {
      blank_screen (*zone); //using old mask
      delete eclipse_mask;
      eclipse_mask= mask;
      maskwidth= newmaskwid; //new mask in place
      unstomp (*zone, Next);
      }
//         when a window was resized bigger RIGHT in KED, the existing right
//         border left a "fossil" attribute at that column. I KLUDGE
//         it by clearing that column during resize. Seems to work.....

if (bigger && fillval) {
   DBG_WIN2(dout<<" tow resize bigger"<<endl);
   DBG_WIN2(dout<<" Location.ul.x = "<<(Location.ul.x)<<"  Location.ul.y = "<<(Location.ul.y)<<endl);
   DBG_WIN2(dout<<" Location.lr.x = "<<(Location.lr.x)<<"  Location.lr.y = "<<(Location.lr.y)<<endl);
   DBG_WIN2(dout<<" Size.x = "<<(Size.x)<<"  Size.y = "<<(Size.y)<<endl);

   zone->ul.y -= Location.ul.y;
   if(mode == 1) {                              // vertical
//          zone->lr.y -= (Location.ul.y -1);    // KLUDGE !!
          zone->lr.y -= (Location.ul.y);        // org
   }
   else {
          zone->lr.y -= Location.ul.y;   
   }
   if(mode == 2) {                              // horizontal
          zone->ul.x -= (Location.ul.x + 1);    // KLUDGE !!! 
   }
   else {
         zone->ul.x -= Location.ul.x;    // org
   }
   zone->lr.x -= Location.ul.x;
   DBG_WIN2(dout<<" tow resize bigger clearing  fillval = "<<hex<<fillval<<dec<<endl);
   DBG_WIN2(dout<<" zone->ul.x = "<<(zone->ul.x)<<"  zone->ul.y = "<<(zone->ul.y)<<endl);
   DBG_WIN2(dout<<" zone->lr.x = "<<(zone->lr.x)<<"  zone->lr.y = "<<(zone->lr.y)<<endl);
   clear (zone->ul.y,zone->lr.y,zone->ul.x,zone->lr.x, fillval);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::resize (tudlr direction, int count, USHORT fillval)
{
unsigned mode, newval;
switch (direction) {
   case Up:
      mode= 1;
      newval= Size.y-count;
      break;
   case Down:
      mode= 1;
      newval= Size.y+count;
      break;
   case Left:
      mode= 2;
      newval= Size.x-count;
      break;
   case Right:
      mode= 2;
      newval= Size.x+count;
      break;
   }
resize (mode, newval, fillval);
}
