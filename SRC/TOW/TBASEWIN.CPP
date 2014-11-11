/********************************************************************
file: BASEWIN.CPP             Copyright 1994 by Kudzu Software
   rev 1.0 Jan 3, 1994
  implementation for a basic windows class.
  Shares much of the code with SIMPWIN, but this avoids changing
  the simpwin class...

  origin row, col is 1,1 if window is bordered (the usual case)
  otherwise origin is 0,0

  rev 1.5 April 24, 1994
  because window_core::scroll did not clear the area exposed when it 
  scrolled forced me to subclass it under basewin::scroll and add
  functions to clear a line and clear a column.

  rev 1.6 April 27, 1994
  need to refresh ONLY the printed area for clearline.
  can probably factor out common "box" and "display" functions
  from the "put" family of methods for slightly smaller code

  rev 2.0 May 10, 1994
  KEd now scrolls correctly Left and Right, but I needed to
  kludge basewin::scroll by not calling wcore::scroll so 
  it won't work for the base lib.
  ALSO, refresh of the cleared vertical line is VERY slow.
  Added a put_fill form with length specifier if the string
  is not null terminated.

********************************************************************/
#include "usual.h"
#include "tow.h"
#include "basewin.h"
#include <string.h>   //strlen() needed
#include <mem.h>      // memset needed for put_w_tab
#include <ctype.h>    // isspace macro
#include "misc.h"     // fillwords

#ifndef __OS2__
extern "C" void fillwords (void far* dest, unsigned val, unsigned count);
#endif

#include "test.h"

#pragma intrinsic strlen

basewin::basewin(void)
//  : window_core(default_parent, meta_window::height, meta_window::width, 0, 0)
  : window_core(default_parent, Screen.get_screen_height(),Screen.get_screen_width(), 0, 0)
{
  setup();
}

/*********************************************************************/

//#ifdef FUCKUP
#ifdef MEMCPY_C
static void memcpy_even (void* dest, void const* source, unsigned count)
{    
while (count--) {
   *(byte*)dest= *(byte*)source;
   dest= (byte*)dest + 2;
   source= (byte*)source + 1;
   }
}
#endif

#ifdef FUCKUP
static void memcpy_even2 (byte* dest, byte const* source, USHORT count)
{    
while (count--) {
   *dest= *source;
   dest= dest + 2;
   source++;
   }
}
#endif

#ifdef FUCKUP
//#ifdef MEMCPY_C
static void memcpy_even3 (byte* dest, byte const* source, USHORT count)
{  
for(int n = 1; n <= count; n++) {
   *dest= *source;
   dest= dest + 2;
   source++;
   }
}
#endif

#ifdef FUCKUP
//#ifdef MEMCPY_C
//static void memcpy_even4(USHORT* dest, byte const* source, USHORT count)
void memcpy_even (USHORT* dest, byte const* source, USHORT count)
{    
for(int n = 1; n <= count; n++) {
   *dest= *source;
   dest= dest + 2;
   source++;
   }
}
#endif

#ifdef FUCKUP
//#ifdef MEMCPY_C
//static void memcpy_even5 (USHORT* dest, byte const* source, USHORT count)
void memcpy_even (USHORT* dest, byte const* source, USHORT count)
{    
for(int n = count; n > 0; n--) {
   *dest= *source;
   dest++;
   source++;
   }
}
#endif
/*************************************************************************
        PUT a string to a window  
        original version moved to put(row, col, char*, length)
        to shorten lib
*********************************************************************/
void basewin::put (int row, int col, char const* s)
{
int length = strlen(s);
put(row, col, s, length);
}

#ifdef FUCKUP
/******************************************************************
                   "pad" a window with a character"
        used in KEd for compatibility
**************************************************************/
void basewin::pad(int row, int col, char fill, int len)
{
unsigned short default_cell = (0x1E00 | 0x00B0);       // test pattern
//unsigned short default_cell = get_cell();
int maxlen= get_width() - col;
if (len > maxlen) 
   len = maxlen;  //don't write beyond edge

if((row < get_height()) && (row >= 0)) {   // check for legal row
   fillwords (get_buffer(row, col), default_cell, len);
}
if (displayed()) {
   twocorner box;
   box.ul.y= box.lr.y= y_pos() + row;
   box.ul.x= x_pos() + col;
   box.lr.x= box.ul.x + len-1;
   display(box);
   }
}
#endif
#ifdef FUCKUP
/********************************************************************
      PUT a string to a window, and expand tabs
      string must be null terminated
      SHOULD THIS BE HERE ??       5-29-94

      get the passed string, look at each char. If a tab, insert
      spaces and pass to put_fill

      startcol = window has scrolled left or right, and the calculations
      need to be done from the 1st actual col of "normal" win
********************************************************************/
void basewin::put_w_tabs (int row, int col, int startcol, char const* s, int slen)
{
  #define LEN 133
  #define TAB 0x09
  #define SPACE 0x20
  char tempstr[LEN];        // arbitrary length.....
  char *p = tempstr;
  int i = 0;

  memset(tempstr, 0x00, LEN);           // zero out tempstr DON'T NEED ??

  DBG_WIN2(dout<<" put_w_fill string len = "<<(strlen(s))<<endl);
  DBG_WIN2(dout<<" put_w_fill string  = "<<s<<endl);

if(slen > 0) {
  for( ; (s != 0) && (i<get_width()); s++) {        
//  DBG_WIN3(dout<<" put_w_fill expanding tabs "<<endl);
     if (*s == TAB) {               // found a tab
        do {
           p[i++] = SPACE;
        } while(((i+startcol) % 4) != 0);
        s++;                        // increment past the tab
     }
    p[i++] = *s;
  }
  put_fill(row, col, p, strlen(s));  // mod 7-27-94 added len param
}
}
#endif
#ifdef FUCKUP
/*************************************************************************
      PUT a string to a window, pad remainder of line with spaces  
      if string is NULL, do nothing
      KEd needs this  string NEED NOT be nul terminated         
**********************************************************************/
void basewin::put_fill (int row, int col, char const* s, int length)
{
   if(length != 0)
      put_fill(row, col, s, length);
   else
      put_fill(row, col, "",0);            // null string
}
#endif
/*************************************************************************
      PUT a string to a window, pad remainder of line with spaces  
      KEd needs this                
      string does not need to be nul terminated                               

      rev 2.0 August 13, 1994
      When window is resized bigger, default attribute needs to be
      written to each line, so I have to ALWAYS write the fillword cell
      to the whole line.
**********************************************************************/
//void basewin::put_fill (int row, int col, char const* s, int length)
void basewin::put_fill (int row, int col, byte const* s, int length)
{
int maxlen= get_width() - col;
//int len;
int len = (length >= 0) ? length : 0;
//unsigned short default_cell = (0x1E00 | 0x00B0);       // test pattern

DBG_WIN(dout<<"put_fill attr = "<<hex<<(get_cell())<<dec<<" row = "<<row<<" col = "<<col<<" length = "<<maxlen<<endl);
                                      // "pad" rest of line with default fill
//if((maxlen > len) || (s == NULL)) {
//    fillwords (get_buffer(row, col+length), get_cell(), fill_len);
                        // kludge, because scrolling a window did not
                        // repaint the colors for the line chars 7-3-94
    fillwords (get_buffer(row, col), get_cell(), maxlen);  
//}                                          
//put(row, col, s, length);           
put(row, col, s, len);           
}

/*************************************************************************
        PUT string, specify length  
        this is useful if you are not sure the string is nul-terminated

        this is now the general case, most other put() routines 
        front-end this

        coordinates are checked, but this will write on top and bottom
        borders
************************************************************************/
void basewin::put (int row, int col, byte const* s, int len)
{
if((col > width()) || (col < 0))         // check for legal col
   return;
int maxlen= get_width() - col;
if (len > maxlen) 
   len = maxlen;                         //don't write beyond edge

if((row <= wheight-1) && (row >= 0)) {   // check for legal row
//   if((s != NULL) && (len > 0)) {
   if((*s != 0) && (len > 0)) {                   // MOD 9-24-94
      memcpy_even (get_buffer(row,col), s, len);
      DBG_WIN(dout<<"tbasewin put row = "<<row<<"  str = °"<<s<<"°"<<endl);
   }
}
if (displayed()) {
   twocorner box;
   box.ul.y= box.lr.y= y_pos() + row;
   box.ul.x= x_pos() + col;
//   box.lr.x= box.ul.x + len-1;
   box.lr.x= box.ul.x + maxlen-1;  // mod 5/8/94 always update whole line
   DBG_WIN(dout<<" basewin put update coords "<<endl);
   DBG_WIN(dout<<" ul.x = "<<(box.ul.x)<<" ul.y = "<<(box.ul.y)<<endl);
   DBG_WIN(dout<<" lr.x = "<<(box.lr.x)<<" lr.y = "<<(box.lr.y)<<endl);
   display(box);
   }
}

/*************************************************************************
             print a string to a window and format it to fit
               if it has embedded CR or LF chars 
               USED for OS/2 error messages

               needs more work 5-22-94
*********************************************************** */
void basewin::print (int row, int col, char const* s)
{
int maxlen= get_width() - col;
int myrow = row;
int mycol = col;
char tmpstr[120];
char tmpstr2[57];
bool output = FALSE;
int n = 0;

memset(tmpstr, 0x00,119);     // should be long enough....
memset(tmpstr2, 0x00,56);     // sub string output win is 58 cols
strncpy(tmpstr, s, 119);

while(tmpstr[n] != 0) {                             // wipe all CR and LF
  if ((tmpstr[n] == 0x0d) || (tmpstr[n] == 0x0a)) {
     tmpstr[n] = ' ';
  }
  n++;
}     
n = 0;         // go back to start of str

while ((tmpstr[n] != 0) && (!output)) {
   if (mycol > (maxlen)) { //output 1st part of str and move 1 row down 
      while(!isspace(tmpstr[n])) {          // find last whitespace
           n--;
      }
      strncpy(tmpstr2, tmpstr, n-1);        // copy 1st part to tmp
      DBG_WIN2(dout<<" print outputting "<<tmpstr2<<endl);
      put(myrow, 2, tmpstr2);               // and output it
      memset(tmpstr2, 0x00,56);          
      myrow++;                             // next row
      strncpy(tmpstr2, tmpstr + n, 56);    // copy 2nd part to tmp
      DBG_WIN2(dout<<" print outputting "<<tmpstr2<<endl);
      put(myrow, 2, tmpstr2);              // and output it
      output = TRUE;
   }
   n++;
   mycol++;
}
   if(!output) {                       // str shorter than window
      put(myrow, 2, tmpstr);           // and output it
   }
}

/*********************************************************************
      put a string centered in a window. If string is too long, 
      truncate output
*********************************************************************/
void basewin::put_centered (int row, char const* s)
{
int length = strlen(s);
int maxlen= get_width();
int col = ((maxlen+2) - length) / 2;
if (col + length > maxlen) {
   length = maxlen - col;  //don't write beyond edge
}
put(row, col, s, length);
}

/**********************************************************************
       Title the window on the top or bottom border

       enum param for Left, Center, Right.  string MUST be null terminated
       mod 5-18-94
           added to enum to allow output on bottom border

       May 22, 1994
       string length is not really checked now..... ok ?
************************************************************************/
void basewin::title (title_pos position, const char* title_str)
{
   int length = strlen(title_str);

   switch(position) {
      case TLeft:
               put(0, 2, title_str);  
               break;
      case TCenter:
               put_centered(0, title_str);   
               break;
      case TRight:
               put(0, width()-length-2, title_str);  
               break;
      case BLeft:
               put(wheight-1, 2, title_str);  
               break;
      case BCenter:
               put_centered(wheight-1, title_str);   
               break;
      case BRight:
               put(wheight-1, width()-length-2, title_str);  
               break;
   }
}
/*************************************************************************/
#ifndef KED
void basewin::put_combined (int row, int col, unsigned const* s, unsigned count)
{
if(count > get_width())
   count = get_width();
memcpy (get_buffer(row,col), s, count);
if (displayed()) {
   twocorner box;
   box.ul.y= box.lr.y= y_pos() + row;
   box.ul.x= x_pos() + col;
   box.lr.x= box.ul.x + count-1;
   display(box);
   }
}
#endif

/**************************************************************************
  highlight the window at row, col with attrib
  do not change displayed characters.
  flush changes to screen
************************************************************************/
void basewin::highlight (int row, int col, byte attrib, int count)
{
if(count > get_width())
   count = get_width();

byte* dest= (byte *)get_buffer(row,col);
dest++;                  //point to attrib byte, which follows the char byte.
DBG_WIN1(dout<<"basewin highlight row = "<<row<<" col = "<<col<<" count = "<<count<<endl);
for (int index= 2*(count-1); index >= 0; index-=2) {
    dest[index]= attrib;
}
if (displayed()) {
   twocorner box;
   box.ul.y= box.lr.y= y_pos() + row;
   box.ul.x= x_pos() + col;
   box.lr.x= box.ul.x + count-1;
   display (box);
   }
}

/**************************************************************
        return UCHAR at specified row, col coordinates
***************************************************************/
unsigned char basewin::peek_char(int row, int col)
{
  unsigned short char_attr = *get_buffer(row, col);
  return LOUCHAR(char_attr);
}

/**************************************************************
        return UCHAR attribute at specified row, col coordinates
***************************************************************/

unsigned char basewin::peek_attr(int row, int col)
{
  unsigned short char_attr = *get_buffer(row, col);
  return HIUCHAR(char_attr);
}

/***************************************************************
    resize window - this is just a shell for basewin
    to record the new window size, window_core::resize 
    does all the work
***************************************************************/
void basewin::change_size (tudlr direction, int count, unsigned fillval)
{
unsigned mode, newval;
switch (direction) {
   case Up:
      mode= 1;
      newval= height()-count;
      wheight -= count;
      break;
   case Down:
      mode= 1;
      newval= height()+count;
      wheight += count;
      break;
   case Left:
      mode= 2;
      newval= width()-count;
      wwidth -= count;
      break;
   case Right:
      mode= 2;
      newval= width()+count;
      wwidth += count;
      DBG_WIN2(dout<< "basewin::change_size Right count = "<< count<<" wwidth = "<<wwidth<<endl);
      break;
   }
resize (mode, newval, fillval);
}

// ////////////////////////////////////////////////////////////////////
#ifdef FUCKUP

       // THIS IS NOT NEEDED FOR KED  5-22-94

/************************************************************************
   scrolls contents of window in any four directions. 
   calls window_core::scroll to do the work, then clears the now
   "undead" line or column with the default "cell" that was last
   used to clear the window
********************************************************************/
void basewin::scroll(enum tudlr direction, int count)
{
int row, col;
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_WIN2(dout<<"basewin::scroll direction "<<direction<<"  count = "<< count<<endl);

window_core::scroll(direction, count);

switch (direction) {
   case Down:  row = 1;                    
               DBG_WIN2(dout<<"basewin::scroll Down count = "<< count<<endl);
         //      while(how_much--) {
        //          clearline(row);
                  DBG_WIN2(dout<<"basewin::scroll clearline "<<row<<endl);
                  row++;
        //       }
               break;
   case Left:  col = get_width() - 1;         // col = 1;
               DBG_WIN2(dout<<"basewin::scroll Left count = "<< count<<endl);
         //      while(how_much--) {
         //         clearcol(col);
         //         DBG_WIN3(dout<<"basewin::scroll LEFT clearcol "<< col<<endl);
                  col--;   //col++;
         //      }
               break;
   case Up:    row = get_height() - 1;
               DBG_WIN2(dout<< "basewin::scroll Up count = "<< count<<endl);
          //     while(how_much--) {
          //        clearline(row);
                  DBG_WIN2(dout<< "basewin::scroll clearline "<< row<<endl);
           //       row--;
           //    }
               break;
   case Right: col = 1;      // col = get_width() - 1;
               DBG_WIN2(dout<< "   basewin::scroll RIGHT width = "<<col<<endl);
           //    while(how_much--) {
            //      clearcol(col);
           //       DBG_WIN3(dout<<"basewin::scroll clearcol "<< col<<endl);
           //       col++;
           //    }
               break;
   }
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}

/******************************************************************
   clear the line in the logical buffer, don't flush the change
   to the screen.  Used by basewin::scroll.
**************************************************************/
void basewin::clearline(int line)
{
twocorner box;
unsigned short* buf= get_buffer(line ,1);
int count= get_width() - 1;
unsigned short default_cell = get_cell();
//unsigned short default_cell = (0x1E00 | 0x00B0); // test pattern
fillwords (buf, default_cell, count);
box.ul.x = box.ul.y = 1;
box.lr.y = get_height();
box.lr.x = get_width();
refresh(box);
}

/******************************************************************
   clear the column in the logical buffer, don't flush the change
   to the screen.  Used by basewin::scroll.
**************************************************************/
void basewin::clearcol(int col)
{
twocorner box;
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
unsigned short* buf;
int count= get_height();
unsigned short default_cell = get_cell();
//unsigned short default_cell = (0x1E00 | 0x00B0);   // test pattern
   while(count--) {
      buf = get_buffer(count,col);
      *buf = default_cell;
   }
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
box.ul.x = box.ul.y = 1;
box.lr.y = get_height();
box.lr.x = get_width();
refresh(box);
}

#endif       // #not used 6-18-94
