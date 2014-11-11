/*****************************************************
file: REDRAW.C      Copyright 1989 by Dlugosz Software
   This is a function to redraw part of a viewer window.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
#include "misc.h"
#include "basewin.h"
#include "status.h"    // status_redraw in flush 6-4-94

#include "test.h"
                          // need if inline line_fetch
//static struct line_header empty_header= {0,0};
/* **************************************** */

static void  shade_rectangle (view_rec *v, SHORT row,SHORT col, 
                              SHORT rowcount, SHORT colcount, byte color)
{
//assert (row >= 0 && row < vwptr[v->view_num]->get_height());
//assert (col >= 0 && col < vwptr[v->view_num]->get_width());
//assert (row+rowcount < vwptr[v->view_num]->get_height());
//assert (col+colcount < vwptr[v->view_num]->get_width());


DBG_WIN1(dout<<"shade rect "<<"start row= "<< row<<" start col= "<<col<<" rowcount= "<<rowcount<<" colcount= "<<colcount<<" attrib= "<<hex<<color<<dec<<endl);

if((row < 0) || (row > vwptr[v->view_num]->get_height())) {
   DBG_WIN1(dout<<"shade_rectangle        FAILS 1"<<endl);
   return;
}
if((col < 0) || (col > vwptr[v->view_num]->get_width())) {
   DBG_WIN1(dout<<"shade_rectangle        FAILS 2"<<endl);
   return;
}
if((row + rowcount) > vwptr[v->view_num]->get_height()) {
   DBG_WIN1(dout<<"shade_rectangle        FAILS 3"<<endl);
   return;
}
if((col + colcount) > vwptr[v->view_num]->get_width()) {
   DBG_WIN1(dout<<"shade_rectangle        FAILS 4"<<endl);
   return;
}

while (rowcount--) {            
     vwptr[v->view_num]->highlight(row++, col, color, colcount);
   }
}

/* **************************************** */

void shade_all (struct view_rec *v)
{
shade_rectangle (v, v->top_border, 1, v->length, v->width, v->colors[0]);
vwptr[v->view_num]->show();
}

/* **************************************** */

void  shade_block (vptr view_v, bool print,SHORT start,SHORT end)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (view_v);
struct mark_rec a, b,  *m= get_mark_array(v->sheet);
SHORT lastrow= v->length+v->top_border;
ULONG temp_row = 0;

   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" top_border = "<<(v->top_border)<<" firstrow= "<<(v->firstrow)<<endl);
   temp_row = (v->top_border > v->firstrow) ? (v->top_border - v->firstrow) : (v->firstrow - v->top_border);
a= m[v->block_begin];
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" a.row = "<<a.row<<endl);
b= m[v->block_end];
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" b.row = "<<b.row<<endl);
//   if((v->firstrow - v->top_border) > a.row)
   if(temp_row > a.row)
     a.row = 0;
   else
     a.row -= v->firstrow - v->top_border;
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" a.row = "<<a.row<<endl);
//   if((v->firstrow - v->top_border) > b.row)
   if(temp_row > b.row)
     b.row = 0;
   else
     b.row -= v->firstrow - v->top_border;
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" b.row = "<<b.row<<endl);
a.col -= v->firstcol - 1;
b.col -= v->firstcol - 1;
  DBG_WIN1(dout<<"shade_block a.row = "<<a.row<<" b.row= "<<b.row<<" a.col= "<<a.col<<" b.col= "<<b.col<<" start= "<<start<<" end= "<<end<<endl);
if (!print) {                              // block in top part of window
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" !print = TRUE "<<endl);
   if (a.row > end || b.row < start) {   
      DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" row = "<<start<<" col = 1"<<" rowcount = "<<(end-start+1)<<" colcount = "<<(v->width)<<endl);
      shade_rectangle (v, start, 1, end-start+1, v->width, v->colors[0]);
      goto display;
   }
}
DBG_WIN1(dout<<" shade_block line "<<__LINE__<<endl);
             // shade entire window with default viewer attribute
             // (wipes out any old block)
shade_rectangle (v, v->top_border, 1, v->length, v->width, v->colors[0]);

if (a.row >= lastrow) 
   goto display;
else if (a.row < v->top_border) {
   a.row= v->top_border;
   if (v->block_mode == view_rec::bk_stream) a.col= 0;
   }
if (b.row < v->top_border) 
   goto display;
else if (b.row >= lastrow) {
   if (v->block_mode == view_rec::bk_lines) b.row= lastrow;
   else b.row= lastrow-1;
   if (v->block_mode == view_rec::bk_stream) b.col= v->width+1;
   }
if (a.col < 1) a.col= 1;
if (b.col < 1) b.col= 1;
if (a.col >= v->width) a.col= v->width-1;
if (b.col >= v->width) b.col= v->width+1;
switch (v->block_mode) {
   case view_rec::bk_columns:
      shade_rectangle (v, a.row, a.col, b.row-a.row+1, b.col-a.col, v->colors[2]);
      break;
   case view_rec::bk_lines:
      shade_rectangle (v, a.row, 1, b.row-a.row, v->width, v->colors[2]);
      break;
   case view_rec::bk_stream:
      if (a.row == b.row) {
        DBG_WIN1(dout<<" shade_block 3"<<endl);
                                                   // shade only 1 row
        vwptr[v->view_num]->highlight(b.row, a.col, v->colors[2], b.col-a.col);
      }
      else {                                       // shade several rows
         DBG_WIN1(dout<<" shade_block line "<<__LINE__<<endl);
         shade_rectangle (v, a.row+1, 1, b.row-a.row-1, v->width, v->colors[2]);
         DBG_WIN1(dout<<" shade_block line "<<__LINE__<<endl);
         shade_rectangle (v, b.row, 1, 1, b.col-1, v->colors[2]);
         DBG_WIN1(dout<<" shade_block line "<<__LINE__<<endl);
         shade_rectangle (v, a.row, a.col, 1, v->width-a.col+1, v->colors[2]);
      }
      break;
   }
display:
if (print) {
   DBG_WIN1(dout<<"shade_block line "<<__LINE__<<" print = TRUE "<<endl);
   vwptr[v->view_num]->show();
}
}


void view_redraw (vptr v_v, LONG line1,SHORT linecount, SHORT col1, SHORT colcount)
{
/******************************************************************
*  PURPOSE:     redraws part of the active view, only deals with lines in the
*               window save area (not viewer lines anywhere else)
                view_flush (in view_drw.cpp) is called for full window redraw
*  PARAMETERS:  vptr v_v  = which view
*               line1     = window line to start at
*               linecount = how many lines to redraw
*               col1      = screen column to start at
*               colcount  =  how many columns to redraw
*  MODIFIES:    does not modify view_rec 
*               DOES modify screen (ie: display)
*  CALLS:       shade_block
*               display_window
*  CALLED BY:   view_scroll() in viewer.c*
*******************************************************************/
SHORT rowloop;     
LONG row;
ULONG cursheetline;  // need LONG -this must be able to get ANY line in sheet
char *tempstr = "";
//struct sheet *s = my_active_sheet;      // MOD 9-7-94
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (v_v);
//struct mark_rec *m= get_mark_fast (s,v->cursor);       // MOD 9-7-94
bool block_active= v->block_active == view_rec::bk_on;
struct line_header *h;

   DBG_WIN2(dout<< "view_redraw passed linecount "<<linecount); 
   DBG_WIN2(dout<< "  colcount "<<colcount<<" line1 = "<<line1<<endl); 
   DBG_WIN2(dout<<"view_redraw org v->firstrow = "<<(v->firstrow)<<" org v->firstcol = "<<(v->firstcol)<<endl);
//   DBG_WIN2(dout<<"view_redraw m->row = "<<(m->row)<<" m->col = "<<(m->col)<<endl);


if ((line1 < 0) || (line1 > v->length)){  //make sure parameters are in range 
//   cursheetline = v->firstrow - 1;        // line < 0 for Ctrl-PgDn
   linecount += line1;
   line1= 0;
//   DBG_WIN2(dout<<"view_redraw line "<<__LINE__<<" line1 < 0  cursheetline = "<<cursheetline<<endl);
}
else {
    cursheetline= line1 + v->firstrow;       // need for short files....
    DBG_WIN2(dout<<"view_redraw line "<<__LINE__<<" cursheetline = "<<cursheetline<<endl);
}
if (col1 < 0) {
   colcount += col1;
   col1= 0;
   }

if ((linecount + line1 > v->length) || (linecount < 0)) {
//    linecount= v->length-line1;
    linecount = v->length;
    DBG_WIN2(dout<< "view_redraw line "<<__LINE__<<"linecount now =  "<<linecount<<endl); 
}

if ((colcount + col1 > v->width) || (colcount < 0)) {
    colcount= v->width-col1;
    DBG_WIN2(dout<< "view_redraw colcount now =  "<<colcount<<endl); 
}

row= line1 + v->top_border;  /* window row to display at */
rowloop= linecount ;             
cursheetline= line1 + v->firstrow;
//startcol= col1 + v->firstcol;

DBG_WIN1(dout<< "view_redraw cursheetline= "<<cursheetline<<"  v->firstrow = " << v->firstrow);
DBG_WIN1(dout<< "  wrow= "<< row <<"  line1= "<<line1<<"  rowloop= "<< rowloop<<endl);
//DBG_VIEW1(dout<<" view_redraw  active_view = "<<v_v<<" active_sheet = "<<(v->sheet)<<endl);

while (rowloop--) {
   USHORT linelength;
   h= line_fetch (v->sheet, cursheetline);
   if(h->length > v->firstcol) {          // normal, ie: length=5, firstcol=1
      linelength = h->length - v->firstcol;
      if(h->length == 0) 
        linelength = 0;
   }
   else {
    linelength = 0;       // scrolled right, ie:firstcol=5, length=4
   }
//   if(linelength == 0) 
//      tempstr = "";
//   else
//      tempstr = (char *)(h+1);

    tempstr = (linelength != 0) ? (byte *)(h+1) : "";  // str="" if len=0

    DBG_WIN2(dout<< "view_redraw sheetline = "<<cursheetline<<"  length = "<<h->length<< "  wrow = "<<row);
    DBG_WIN2(dout<< "  line = °"<<tempstr<<"°"<<endl);

   vwptr[v->view_num]->put_fill(row, 1, v->firstcol + tempstr, linelength);
   row++;
   cursheetline++;
}                            /* end while rowloop   */
if (block_active) {
    row= line1 + v->top_border;        // reset row
    DBG_WIN1(dout<<" redraw calling shade_block row start ="<<row<<" row end ="<<(row+linecount-1)<<" NO PRINT "<<endl);
    shade_block (v_v, FALSE, row, row+linecount-1);
}
//if (visible) 
    vwptr[v->view_num]->show();
             //   status_redraw();
    statwptr[v->view_num]->show();
}
