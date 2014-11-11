/********************************************************************
File:  BASEWIN.H                   Copyright 1994 by Kudzu Software
  header file for basic windows class for TOW

  Basic Windows Class
  This is a basic window based on TOW.  It offers 4 functions for
printing on windows.   put() takes a location and a string.  Another
form of put() takes a length and will write exactly that many characters
(does not look for '\0' char).  highlight() will work like a highlighting
marker in a book:  it changes the color of a horizontal stripe.
put_combined() will write chars and attribs together.

********************************************************************/
#ifndef BASEWIN_H
#define BASEWIN_H

#include "usual.h"
#include "tow.h"

const int MAX_WINDOWS = 100;        // should be enough.....
                                                           // Top or Bottom
enum title_pos {TLeft,TCenter,TRight,BLeft,BCenter,BRight};// title positions

class basewin : public window_core
{
private:
   int currentrow;
   int currentcol;
   int wheight;          // "raw" coordinates, doesn;t care about border
   int wwidth;
   void setup() {  wwidth = window_core::width();
                   wheight = window_core::height();  }
public:
   basewin (int hight, int width, int y_pos, int x_pos) 
     : window_core(default_parent, hight, width, y_pos, x_pos) { setup(); }
   basewin (int hight, int width, int y_pos, int x_pos, textcolor clr) 
     : window_core(default_parent, hight, width, y_pos, x_pos) 
     {  setup(); clear(clr); }
   basewin (window_core& parent, int hight, int width, int y_pos, int x_pos) 
     : window_core(&parent, hight, width, y_pos, x_pos) { setup();}
   basewin();
   virtual ~basewin () {}
   void title (title_pos, const char* title_str);
   void put (int row, int col, char const* s);
   void put_w_tabs (int row, int col, int startcol, char const* s, int slen); // expand tabs
//   void put_fill (int row, int col, char const* s); // pad line with spaces
//   void put_fill (int row, int col, char const* s, int length); 
   void put_fill (int row, int col, byte const* s, int length); 
   void put (int row, int col, byte const* s, int len);
   void put (int row, int col, char const* s, int len) { put(row,col,(byte *)s,len); }
   void print (int row, int col, char const* s);
//   void pad (int row, int col, char fill, int len); 
   void put_centered (int row, char const* s);
   void put_combined (int row, int col, unsigned const* s, unsigned len);
   void highlight (int row, int col, byte attrib, int count);
   inline void clearwin(textcolor fg=WHITE,textcolor bg=_BLUE,byte fill=' '){
                     clear(USHORT((attr(fg,bg) << 8) | fill)); }
   int get_width(void) const {if(bordered()) return wwidth-1; else return wwidth;}
   int get_height(void) const {if(bordered()) return wheight-1; else return wheight;}
// int get_width(void) const {return (bordered()) ? wwidth-1 : wwidth);} //NO!
// int get_height(void) const {return (bordered() ? wheight-1 : wheight);}

   unsigned char peek_attr(int row, int col);
   unsigned char peek_char(int row, int col);
   void clearline(int line);
   void clearcol(int col);
   void change_size (tudlr direction, int count, unsigned fillval);
//   void scroll(enum tudlr direction, int count);
};

void simple_error_box (char const* message);

/******************************************************************/
//KED specific things

extern basewin* vwptr[MAX_WINDOWS];
extern basewin* statwptr[MAX_WINDOWS];             // status win ptr
extern meta_window Screen;
struct key_rec& window_getkey(basewin* win ,SHORT row, SHORT col, byte color);

#endif
