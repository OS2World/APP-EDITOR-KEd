/********************************************************************
file: WLINE.CPP               copyright 1990,1991 by Dlugosz Software
   part of TOW   Text Output Windows
   functions to draw lines and boxes on a window
   also has border() and unborder() functions
********************************************************************/
#include "usual.h"
#include "tow.h"

#include "test.h"

struct dt { byte letter, attribute; };   //the real display format

#define position(a,b) ((dt*)get_buffer(a,b))


typedef byte chart[2][2];  //a chart to use for intersection characters.
   //The first subscript is the horizontal mode, the second is the
   //vertical mode.  The lookup is the tee-fitting to use in this intersection.
   //initialize with:  single horz single vert, single horz double vert,
   //    double horz single vert, double horz double vert.

static chart middle=   { {197,215},{216,206} };  //full crossing Å × Ø Î
static chart top=      { {194,210},{209,203} };  // 'T' shape    Â Ò Ñ Ë
static chart bottom=   { {193,208},{207,202} };  //             Á Ð Ï Ê
static chart left=     { {195,199},{198,204} };  //              Ã Ç Æ Ì
static chart right=    { {180,182},{181,185} };  //             ´ ¶ ¶ ¹
static chart ULcorner= { {218,214},{213,201} };  //             Ú Ö Õ É
static chart URcorner= { {191,183},{184,187} };  //             ¿ · ¸ »
static chart LLcorner= { {192,211},{212,200} };  // 'L' shape    À Ó Ô È
static chart LRcorner= { {217,189},{190,188} };  //             Ù ½ ¾ ¼

//ordinary line types
const byte horiz_single= 196; // Ä
const byte horiz_double= 205; // Í
const byte vert_single= 179;  // ³
const byte vert_double= 186;  //  º

static window_core::SorD iv_report;

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef __OS2__
static bool near is_vert (byte c)
#else
static bool is_vert (byte c)
#endif
{
if ((c==vert_single) || (c== *URcorner[0])) {
   iv_report= window_core::Single;
   return TRUE;
   }
if ((c==vert_double) || (c== *URcorner[3])){
   iv_report= window_core::Double;
   return TRUE;
   }
return FALSE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifdef FUCKUP                                  // future func
#ifndef __OS2__
static bool near is_horiz (byte c)
#else
static bool is_horiz (byte c)
#endif
{
if (c==horiz_single) {
   iv_report= window_core::Single;
   return TRUE;
   }
if (c==horiz_double) {
   iv_report= window_core::Double;
   return TRUE;
   }
return FALSE;
}
#endif
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::hline (int row, int startcol, int count, SorD mode)
{   // draw horizontal line
int lastcol= startcol+count-1;
int currentcol = startcol;          // mod RADICK
twocorner box;                          //what part of the screen to update
box.ul.y= box.lr.y= row;
box.ul.x= startcol;
box.lr.x= lastcol;
dt* p= position (row, startcol);
chart* teechart;
char plot= mode == Single ? horiz_single : horiz_double;
                                         //do the first character
if (is_vert(p->letter)) {
                                        //is this a left edge or a corner?
   if (row == 0 || !is_vert(position(row-1,startcol)->letter))
                                       //line does not continue above
      teechart= &ULcorner;
   else if (row == height()-1 || !is_vert(position(row+1,startcol)->letter))
                                      //line does not continue below
      teechart= &LLcorner;
   else                              //line continues in both directions
      teechart= &left;
   p->letter= (*teechart)[mode][iv_report];
   p->attribute = alt_attr;                 // MOD Radick
   }
else {
   p->letter= plot;
   p->attribute = alt_attr;                 // MOD Radick
}
p++;
currentcol++;          // mod RADICK
                                     // do all middle characters
while (--count > 1) {
   if (is_vert(p->letter)) {
                                 //is this a plain line or a top or bottom T?
//      if (row == 0 || !is_vert(position(row-1,startcol)->letter))
      if (row == 0 || !is_vert(position(row-1,currentcol)->letter))
                                //line does not continue above
         teechart= &top;
//      else if (row == height()-1 || !is_vert(position(row+1,startcol)->letter))
      else if (row == height()-1 || !is_vert(position(row+1,currentcol)->letter))
                               //line does not continue below
         teechart= &bottom;
      else                     //line continues in both directions
         teechart= &middle;
      p->letter= (*teechart)[mode][iv_report];
      p->attribute = alt_attr;                 // MOD Radick
      }
   else {
      p->letter= plot;
      p->attribute = alt_attr;                 // MOD Radick
   }
   p++;
   currentcol++;  // mod RADICK
   }
                                           //do the last character
if (is_vert(p->letter)) {
                                          //is this a right edge or a corner?
   if (row == 0 || !is_vert(position(row-1,lastcol)->letter))
                                         //line does not continue above
      teechart= &URcorner;
   else if (row == height()-1 || !is_vert(position(row+1,lastcol)->letter))
                                        //line does not continue below
      teechart= &LRcorner;
   else                                //line continues in both directions
      teechart= &right;
   p->letter= (*teechart)[mode][iv_report];
   p->attribute = alt_attr;                 // MOD Radick
   }
else {
   p->letter= plot;
   p->attribute = alt_attr;                 // MOD Radick
}
refresh (box);             
}

/********************************************************************
   draw a vertical line - don't worry about line crossings, unlike
   the horizontal line func.

   original routine did nothing with attributes, the char was placed
   on the screen in whatever attr was at that location.

   I've added a parameter "alt_attr" to the base TOW class to also set 
   the attr, so I can draw a border with a diff color than the rest of 
   the window. If alt_attr is not set in a border() call, then it should
   be the same as the default window attr used in clear()
************************************************************************/
void window_core::vline (int col, int startrow, int count, SorD mode)
{
twocorner box;
box.ul.x= box.lr.x= col;
box.ul.y= startrow;
box.lr.y= startrow+count-1;
const byte plot= mode == Single ? vert_single : vert_double;
while (count--) {
   position(startrow, col)->letter= plot;
//   position(startrow, col)->letter= '°';           // testing
   position(startrow++, col)->attribute = alt_attr;
}
refresh (box);                   
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::drawbox (int startrow, int startcol, int height, int width,
            SorD hlines, SorD vlines)
{
vline (startcol, startrow, height, vlines);
vline (startcol+width-1, startrow, height, vlines); // BUG in right border!
hline (startrow, startcol, width, hlines);
hline (startrow+height-1, startcol, width, hlines);
}

/******************************************************************
      border box for KED - 6-4-94
      new version puts the status line on the bottom border, 
      so eliminate bottom hline
*****************************************************************/
void window_core::view_box (int startrow, int startcol, int height, int width,
            SorD hlines, SorD vlines)
{
DBG_WIN1(dout<<" view_box passed col = "<<startcol<<" startrow = "<<startrow<<" height = "<<height<<" width = "<<width<<endl);
view_vline (startcol, startrow, height, vlines);  
hline (startrow, startcol, width, hlines);
}

/********************************************************************/
void window_core::view_border(SorD mode, byte attr)
{
    alt_attr = attr;
    DBG_WIN1(dout<< "in border color passed attr = "<<attr<<endl);
    window_core::view_border(mode);
    alt_attr = HIUCHAR(curr_cell);
}

void window_core::view_border (SorD mode)
{
view_box (0,0, height(), width(), mode, mode);
status |= Border;                            // mod RADICK
}

/********************************************************************
    used in KEd for vertical border line that appears to intersect
    with status row
*******************************************************************/
void window_core::view_vline (int col, int startrow, int count, SorD mode)
{
chart* teechart;

twocorner box;
box.ul.x= 0;
box.lr.x= width();
box.ul.y= startrow+count-2;
box.lr.y= startrow+count;

DBG_WIN1(dout<<" view_vline passed col = "<<col<<" startrow = "<<startrow<<" count = "<<count<<endl);
DBG_WIN1(dout<<" view_vline coords lr.x = "<<box.lr.x<<" ul.y = "<<box.ul.y<<" lr.y = "<<box.lr.y<<endl);

teechart= &LLcorner;
position(startrow + count-1, col)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col)->attribute = alt_attr;

teechart= &right;        // char that intersects with KED status in col 2
position(startrow + count-1, col+1)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col+1)->attribute = alt_attr;

teechart= &LRcorner;
position(startrow + count-1, col+width()-1)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col+width()-1)->attribute = alt_attr;

teechart= &left;        // char that intersects with KED status in width - 2
position(startrow + count-1, col+width()-2)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col+width()-2)->attribute = alt_attr;

vline (col, startrow, height()-1, mode);
vline (col+width()-1, startrow, height()-1, mode);  

refresh (box);                   
}

/*****************************************************************
    draw a border in a diff color than the rest of the window.
    use the "alt_attr" memeber in window_core, and then reset that
    back to the default "fillval" when finished
*****************************************************************/
void window_core::border(SorD mode, byte attr)
{
    alt_attr = attr;
//    DBG_WIN1(dout<< "in border color passed attr = "<<attr<<endl);
    window_core::border(mode);
    alt_attr = HIUCHAR(curr_cell);
}
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void window_core::border (SorD mode)
{
drawbox (0,0, height(), width(), mode, mode);
status |= Border;                            // mod RADICK
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
#ifndef KED
void window_core::unborder ()
{
if(bordered()) {                      // nothing to do otherwise
   twocorner box;
   box.ul.x= 0;
   int cols = box.lr.x = width();
   box.ul.y= 0;
   int rows = box.lr.y= height();
   const byte plot = ' ';
                                        // erase the border
   while (cols--) {                        // top and bottom
      position(0, cols)->letter= plot;
      position(rows-1, cols)->letter= plot;
   }
   while (rows--) {                        // left and right sides
      position(rows, 0)->letter= plot;
      position(rows, box.lr.x-1)->letter= plot;
   }
   refresh (box);                   
   status ^= Border;                            // mod RADICK
}
}
#endif
/**********************************************************************/

#ifdef FUCKUP
/********************************************************************
    used in KEd for horizontal border line that appears to intersect
    with status row
*******************************************************************/
void window_core::view_hline (int row, int startcol, int count, SorD mode)
{
chart* teechart;

twocorner box;
box.ul.x= 0;
box.lr.x= width();
box.ul.y= startrow+count-2;
box.lr.y= startrow+count;

DBG_WIN1(dout<<" view_vline passed col = "<<col<<" startrow = "<<startrow<<" count = "<<count<<endl);
DBG_WIN1(dout<<" view_vline coords lr.x = "<<box.lr.x<<" ul.y = "<<box.ul.y<<" lr.y = "<<box.lr.y<<endl);

teechart= &LLcorner;
position(startrow + count-1, col)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col)->attribute = alt_attr;

teechart= &LRcorner;
position(startrow + count-1, col+width()-1)->letter= (*teechart)[mode][mode];
position(startrow + count-1, col+width()-1)->attribute = alt_attr;

refresh (box);                   
}
#endif
