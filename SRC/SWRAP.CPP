/*****************************************************
file: SWRAP.CPP       Copyright 1989 by Kudzu Software
      add word wrap to sheet

      need functions to move last word(s) on line to next line
                        move first word(s) on line to prev line

      9/16/95
          basic functions to wrap a line downward on char 
          entry are done. I think the del_char stuff is good also.

          I need functions that:
              when a space in entered "early" in a line and
              the word it creates is short enough to be wrapped
              to the preceding line.

              when a space is deleted in col 0, and there is a 
              word in col 1 that should be combined with the last
              word in the line above.
              In this case, the last word in the line above should
              be wrapped to the line below.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"   /* sheet calls view functions */
#include "misc.h"   // fillwords
#include <string.h>

#include "test.h"

extern struct sheet *cur; 
extern SHORT xbuf;

/*************************************************************************/
#ifdef WORD_WRAP
void sheet_addchars_wordwrap (mark_t position, const char* string,SHORT length)
{   // used by sheet_wrapline
                                    /* quick & dirty for now */
   DBG_SHEET(dout<< "addchar_wordwrap length = "<<length<<"  string = °"<<string<<"°"<<endl);
while (length--) 
    sheet_addchar (position, *(string + length));
}
//#endif


/*************************************************************************
   move first word(s) on _this_ line to the previous line
   delete moved word on _this_ line
*************************************************************************/
void move_word_to_prev_line(mark_rec& wrap_loc)
{
cur = my_active_sheet;       
SHORT col= wrap_loc.col;
ULONG row= wrap_loc.row;

vptr line_v= get_line (row);
struct line_header *h= (line_header *)Mem::vmem_trans (line_v);
vptr prev_line_v= get_line (row-1);
struct line_header *prevh= (line_header *)Mem::vmem_trans (prev_line_v);
USHORT tempm = new_mark();
struct mark_rec *temp = get_mark(tempm);

  DBG_SHEET(dout<<"move_word_to_prev wrap.row= "<<row<<" wrap.col= "<<col<<endl);
  temp->row = wrap_loc.row - 1;              // add word to prev line
  temp->col = prevh->length;                // add to end of line
                                     
  char* moved_word = (char *)(h+1);
//  moved_word[wrap_loc.col + 1] = '\0';
  DBG_SHEET(dout<<"move_word_to_prev len = "<<wrap_loc.col<<"  moved_word = ±"<<moved_word<<"±"<<endl);
  sheet_addchars_wordwrap(tempm, moved_word, wrap_loc.col);

  temp->row = wrap_loc.row;                       // wipe word from cur line
  temp->col = wrap_loc.col - 1;

  DBG_SHEET(dout<<"move_word_to_prev moved_word  ok"<<endl);
  while(temp->col >= 0) {
     sheet_delchar(tempm);
     temp->col--;
  }
    
  cancel_mark(tempm);
}


/*************************************************************************
   move last word(s) on _this_ line to the next line
*************************************************************************/
void move_word_to_next_line(mark_rec& wrap_loc,line_header& l)
{
SHORT col = wrap_loc.col;
ULONG row = wrap_loc.row;
DBG_SHEET(dout<<"move_word_to_next_line wrap_loc.col = "<<col<<" wrap_loc.row = "<<row<<endl);
SHORT len= l.length - col;  
struct line_header* h = line_fetch(active_sheet, row);
struct line_header* next_h = line_fetch(active_sheet, row+1);
USHORT tempm = new_mark();
struct mark_rec *temp = get_mark(tempm);

  if(next_h->length == 0) {          // fucks up on key entry wrap ?
    DBG_SHEET(dout<<"move_word_to_next_line adding BLANK line"<<endl);
    sheet_insert_blank_line(row+2);
  }

  temp->row = 1 + row;
  temp->col = 0;
  char* moved_word = (char *)(h+1) + col;
  DBG_SHEET(dout<<"move_word_to_next_line moved_word = ±"<<moved_word<<"±"<<" len = "<<len<<endl);
                                                   
//  if(moved_word[len] != 0x20) {                    // add space if needed
//     if(0x20 != sheet_peek(active_sheet, tempm)){
//        if(next_h->length != 0) {
//           DBG_SHEET(dout<<"move_word_to_next_line ADDING SPACE"<<endl);
//           sheet_addchar(tempm, 0x20);
//        }
//     }
//  }
  //  sheet_addchars_wordwrap(tempm, moved_word, len); // org, works ok
  sheet_addstr(tempm, moved_word, len);    // mod 11/22/95

  cancel_mark(tempm);
}


void sheet_wrapline (mark_rec& cursor, mark_rec& wrap_loc)
/****************************************************************
 split the current line at the mark position 
 move any words on line AFTER the mark to next line and del from cur line
 used for  word wrap version  2-8-94 
 
 View_Addchars_w_Margins calls this, passing a cursor REF
 This function will directly modify the cursor location if necessary.

 View_Addchars_w_margins will only call this if a line is too long.

*******************************************************************/
{
cur = my_active_sheet;       
vptr line_v= get_line (wrap_loc.row);
struct line_header *h= (struct line_header *)Mem::vmem_trans (line_v);
SHORT len= h->length - wrap_loc.col;  

DBG_SHEET(dout<<"in sheet_wrap, str = "<<(char *)(h+1)<<endl);
DBG_SHEET(dout<<"in sheet_wrap, wrap_loc.row= "<<wrap_loc.row<<" wrap_loc.col= "<<wrap_loc.col<<" line_count = "<<cur->line_count<<endl);
DBG_SHEET(dout<<"in sheet_wrap, cursor.row= "<<cursor.row<<" cursor.col= "<<cursor.col<<endl);
// DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);

   if((wrap_loc.row + 1) == cur->line_count) {     // last line in sheet
      DBG_SHEET(dout<<"wrapping last line, row = "<<wrap_loc.row<<endl);
      DBG_SHEET(dout<< "line = °"<<((char *)(h+1))<<"° h->length = "<< h->length<<endl);
      sheet_insert_blank_line (1+wrap_loc.row);
      if (wrap_loc.col >= h->length) {               // no words to wrap..
         DBG_SHEET(dout<< "cursor col > h->len inserting blank line"<<endl);
         return;
      }
   }
   if(wrap_loc.col < h->length) {      // split in middle..  col < h->length
      DBG_SHEET(dout<< "wrap cursor col < h->len  wrap_col = "<<wrap_loc.col<<" cursor.col = "<<cursor.col<<endl);
      move_word_to_next_line(wrap_loc, *(h));
      if(wrap_loc.row == cursor.row) {
         if(cursor.col > wrap_loc.col) {           // move the cursor
            cursor.row++;                         
            cursor.col -= wrap_loc.col;  // was (h->length - wrap_loc.col) ;
            DBG_SHEET(dout<< "wrap cursor  cursor.col = "<<cursor.col<<" cursor.row = "<<cursor.row<<endl);
         }
      }
            //mark_splitline (marknum);              // tell marks about it      
//      move_word_to_next_line(wrap_loc, *(h));

      DBG_SHEET(dout<<"after wrap  row= "<<wrap_loc.row<<" col= "<<wrap_loc.col<<" line_count = "<<cur->line_count);
      DBG_SHEET(dout<< " line = °"<<((char *)(h+1))<<"°  h->length = "<<h->length<<endl);

      h->length -= len;              // wipe word from line 1
      char *s = (char *)(h+1);
      s[h->length] = 0x00;

      DBG_SHEET(dout<< "cursor.row= "<<cursor.row<<" col "<<cursor.col<<" row "<<wrap_loc.row<<" now = °"<<s<<"° h->length = "<<h->length<<endl);
      if(cursor.col > (h->length)) {   // wrap cursor if its in word that was wrapped
         DBG_SHEET(dout<< "sheet_wrap LINE "<<__LINE__<<" wrap row = "<<wrap_loc.row<<" col = "<<wrap_loc.col<<endl);
         if(cursor.row == wrap_loc.row) {
            DBG_SHEET(dout<< "SCREWING WITH CURSOR "<<endl);
            cursor.row++;
            cursor.col -= h->length;
         }
     }
  }   
  DBG_SHEET(dout<< " END sheet_wrap cursor.row= "<<cursor.row<<" col= "<<cursor.col<<endl);
//  DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
}

#endif
