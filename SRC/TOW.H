/********************************************************************
File:  TOW.H                  Copyright 1986-1991 by Dlugosz Software
  header file for main core of TOW.
   defines simple types, window_core, meta_window

   Definitions:
              x_pos = columns
              y_pos = rows
              ul.x, ul.y = 0,0
********************************************************************/
#ifndef TOW_INCLUDED
#define TOW_INCLUDED
#define TOW_H

#define INTERSECT_INLINE     // inline intersection() from wcore.cpp
#define USE_WCORE

#include "test.h"

//extern unsigned char* screen_save_buf;        // defined in vios.asm
extern "C" USHORT video_int (USHORT AX,USHORT BX=0,USHORT CX=0,USHORT DX=0);

typedef struct pair { short x,y; };
struct point { pair ul; };
typedef struct twocorner { pair ul, lr; };
enum tudlr {Up, Down, Left, Right};

typedef byte textcolor;
                                 // foreground colors
enum fg_colors {BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, 
                LGREY, DGREY, LBLUE, LGREEN, LCYAN, LRED, LMAGENTA, 
                YELLOW, WHITE};
enum bg_colors {_BLACK, _BLUE, _GREEN, _CYAN, _RED,
                _MAGENTA, _BROWN, _LGREY };

#define BLINK       128 /* blink bit */

inline unsigned char attr(textcolor fg, textcolor bg) {return ((bg<<4) | fg);}

class meta_window;                  // forward declaration

class window_core {
private:
   short int maskwidth;  //mask width in words
   unsigned short* eclipse_mask;
   window_core* Next;  //next in display list
   window_core* Parent;
   window_core* First_child; //head of nested display list
   pair Size;
   twocorner Location;
   unsigned short curr_cell;      // cell is "char attr" pair (16 bits)
   byte alt_attr;                // useful for temp color change, like borders
   unsigned short* save_area;     // need short for 16 bits
   void make_mask();
   void clear_mask();
   void extend_mask (unsigned short* mask,twocorner& mask_pos,twocorner& zone);
   static void add_to_mask (unsigned short* mask,twocorner& box, short int maskwidth, twocorner& shadow);
   unsigned short* adjust_mask (int mode, int newval, int* newmaskwid);   //used by move()
   twocorner* adjust_savearea (int mode, int newval);  //used by move()
   void blank_screen (twocorner& box);
   static void stomp(twocorner& shadow, window_core* first, window_core* firstnot);
   static void unstomp(twocorner& shadow , window_core* first);
//   friend void tdraw_screen (twocorner& box);
#ifndef INTERSECT_INLINE
   static bool intersection (const twocorner& a,const twocorner& b,twocorner* result);
#else
   inline static bool intersection (const twocorner& a,const twocorner& b,twocorner* result) {
   return (
           (result->ul.x= (a.ul.x > b.ul.x) ? a.ul.x : b.ul.x /* left */) <=
           (result->lr.x= (a.lr.x < b.lr.x) ? a.lr.x : b.lr.x /* right */) &&
           (result->ul.y= (a.ul.y > b.ul.y) ? a.ul.y : b.ul.y /* top */) <=
           (result->lr.y= (a.lr.y < b.lr.y) ? a.lr.y : b.lr.y /* bottom */));
     }
#endif
protected:
                                         // constructors
   window_core (int hight, int width, int y_pos, int x_pos);
   window_core (window_core* parent, int height, int width, int y_pos, int x_pos);
                                         // class internals
   window_core* next() const { return Next; }
   window_core* parent() const { return Parent; }
   window_core* first_child() const { return First_child; }
   window_core* display_list() { return Parent->First_child; }
   static window_core* default_parent;
  // other settings
   bool My_Mem;     // does this class own the save_area or not, need for Meta
   twocorner range;  //be careful to keep the size correct!
   unsigned short fillval;
//   byte fillval;
   enum { Display=1, Border=2, Refresh=4, SharedSave=8 };
   unsigned status;  //see above enum values
   unsigned short border_width[4];
                                            // display buffer accessor
   const unsigned short* get_buffer (int row, int col) const;
//   unsigned short* get_buffer (int row, int col);
   inline unsigned short* get_buffer (int row, int col) {
                                 return save_area + (row*Size.x) + col; }
   unsigned short* get_buffer() {return save_area;}
   void display (const twocorner& zone);    //uses display coordinates
   void refresh (twocorner zone);           //uses window coordinates
public:
                                      // create and destroy
   window_core (const meta_window&);  //create a master window
   virtual ~window_core ();
                                      // and manipulate
   void move (tudlr direction, int count);
   void resize (unsigned mode, unsigned newval, USHORT fillval);
   void resize (tudlr, int count, unsigned short fillval);
   void resize (tudlr direction, int count, textcolor c, char ch=' ') { resize (direction, count, USHORT((c<<8)|ch)); }
   void hide();
   void show();
   void freeze() {status &= ~Refresh;}
   void thaw() {status |= Refresh; display(Location);}
                                                      // query status
   bool displayed() const {return (status&(Refresh|Display)) == (Refresh|Display);}
   bool visible() const {return (status&Display);}
   bool bordered() const {return (status&Border);}
   window_core* below() const {return (displayed())? Next : 0; }
   unsigned short peek (int row, int col) const { return *get_buffer(row,col); }
                                                      // metrics
   const pair& size() const { return Size; }
   static const pair& display_size() { return default_parent->Size; }
   int height() const {return Size.y;}
   int width() const {return Size.x;}
   const twocorner& location() const { return Location; }
   int y_pos() const {return Location.ul.y;}
   int x_pos() const {return Location.ul.x;}
   int bwidth(tudlr p) const {return border_width[p];}
                                                      // rendering
   void blitt(short int y_dest, short int x_dest, short int v_size, 
              short int h_size,window_core* from_w, short int y_start, 
              short int x_start);
   void scroll (tudlr direction, int count);
   void clear (int r1, int r2, int c1, int c2, USHORT fillval);
   void clear (int r1, int r2, int c1, int c2, int fillval)
      { clear (r1,r2,c1,c2,USHORT(fillval)); }
   void clear (int r1, int r2, int c1, int c2, textcolor c, char ch=' ')
      { clear (r1,r2,c1,c2,USHORT(c<<8)|ch); }
   void clear (USHORT fillval) {clear(0,Size.y-1,0,Size.x-1,fillval);}
   void clear (unsigned fillval) {clear(0,Size.y-1,0,Size.x-1,USHORT(fillval));}
   void clear (textcolor c, char ch=' ') { clear (0,Size.y-1,0,Size.x-1,USHORT(c<<8)|ch);}
                                                    // lines and boxes
   enum SorD {Single,Double};
   void hline (int row, int startcol, int count, SorD mode);
   void vline (int col, int startrow, int count, SorD mode);
   void drawbox (int startrow, int startcol, int height, int width,
            SorD hlines, SorD vlines);
   void border (SorD mode);
   void border(SorD mode, byte attr);   // draw border w/ color
   void unborder ();                    // turn off border if present
                                        // special funcs for KED
   void view_box (int startrow, int startcol, int height, int width,
            SorD hlines, SorD vlines);
   void view_border (SorD mode);
   void view_border(SorD mode, byte attr);   // draw border w/ color
   void view_vline (int col, int startrow, int count, SorD mode);
   void view_hline (int row, int startcol, int count, SorD mode);

//   void clearline(int line);
   unsigned short get_cell(void) const { return curr_cell; }
};


class meta_window {
private:
   bool setup();                   //used by constructor
   unsigned screensize;
   unsigned short fillval;                //    used by setup/shutdown
   unsigned old_cursor_pos;
   bool display_iscolor;                    
   window_core *display_list;
   unsigned short vidbufsize;
   int height, width;
//   unsigned short* screen_save;
   unsigned char* screen_save;        // mod 1-1-95
   twocorner range;

public:                                     // methods
   static char *term_mess;                  //termination message
   unsigned short get_fillval() const { return fillval; }
   unsigned short get_vidbufsize() const { return vidbufsize; }
   int get_screen_height() const { return height; }
   int get_screen_width() const { return width; }
   twocorner get_range() const { return range; }
//   unsigned short* get_save_buf() { return screen_save; }
   unsigned char* get_save_buf() { return screen_save; }     // mod 1-1-95
//   unsigned char* get_save_buf() { return screen_save_buf; }     // mod 1-1-95
   bool iscolor() const { return display_iscolor; }    
   meta_window ();                               // initialize windows system
   ~meta_window();                               // shut down windows system
//   void regenerate() {tdraw_screen (range);}

#ifndef __OS2__                             //hardware specific info
   static unsigned short* address;
#else                    
    static unsigned short _far16* address; 
#endif
};

#endif







