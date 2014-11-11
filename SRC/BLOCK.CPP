/*****************************************************
file: BLOCK.C       Copyright 1989 by Dlugosz Software
   block cut/paste etc.
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include "view.h"
                        //#include "keycodes.h"
#include "status.h"
#include "redbox.h"
#include <stdlib.h>  /* need malloc() and free() */
#include "misc.h"    /* need fillwords() */
#include <string.h>  /* need memcpy() */
#include "block.h"
                      //#include "status.h"
//#include "tow.h"      // checkmem()  only
#include "test.h"

struct scrap_t *scrap= NULL;

#define locus 4

/*************************************************************************/

bool my_new_scrap (void)       /* create a new scrap buffer */
{
struct scrap_t *newscrap= (struct scrap_t *)malloc(sizeof(struct scrap_t));
if (!newscrap) {
   redbox (locus, 200, eh_abort);
   return FALSE;
   }
newscrap->next= scrap;  /* stack old scrap */
scrap        = newscrap;
scrap->mode  = view_rec::bk_lines;
scrap->valid = FALSE;
scrap->sh    = new_sheet_general (2|4);
DBG_BLOCK1(dout<<" new_scrap sheet = "<<(scrap->sh)<<endl);
   /* flags to allow sheet to exist without viewers,
      and to suppress redbox on throwing away unsaved changes */
return TRUE;
}

/* **************************************** */

bool restore_scrap (void)
{
struct scrap_t *oldscrap= scrap;
if (scrap == NULL) {
   redbox (locus, 201, eh_abort);
   return FALSE;
   }
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
delete_sheet (scrap->sh);
DBG_BLOCK1(dout<<"restore_scrap sheet = "<<(scrap->sh)<<endl);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
scrap= scrap->next;
free (oldscrap);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
return TRUE;
}

SHORT block_unmark()
{
   struct view_rec *v;
   v= (struct view_rec *)Mem::vmem_trans (active_view);
   v->block_active= view_rec::bk_off;   /* disable block */
   status_block (-1);
   shade_all (v);             /* and clear block attributes */
return 0;
}   

/* **************************************** */

bool cursor_in_block (struct mark_rec *cursor, struct mark_rec *begin, struct mark_rec *end, SHORT blockmode)
{
switch (blockmode) {
   case view_rec::bk_stream:
      if (cursor->row == begin->row && cursor->col < begin->col) 
         return FALSE;
      if (cursor->row == end->row && cursor->col >= end->col) 
         return FALSE;
      if (cursor->row < begin->row || cursor->row > end->row) 
         return FALSE;
      return TRUE;
   case view_rec::bk_lines:
      if (cursor->row > begin->row && cursor->row < end->row) 
         return TRUE;
      return FALSE;
   case view_rec::bk_columns:
      if (cursor->row < begin->row || cursor->row > end->row) 
         return FALSE;
      if (cursor->col < begin->col || cursor->col >= end->col) 
         return FALSE;
      return TRUE;
   default:  /* bad parameter */
      assert (FALSE);
      return FALSE;  /* shut up compiler warning */
   }
}

/* **************************************** */
/*   sheet level commands                   */
/* **************************************** */
/*******************************************************************
   copy a block FROM the active_sheet TO the scrap_sheet

   this is WAY too slow with the default append_line_part() call
   need to use vmem_burst_alloc like xfile to speed up

********************************************************************/


static bool  block_prim_copy (mark_t startmark, mark_t endmark,
                              view_rec::block_mode_t mode, bool cut)
{
struct mark_rec m1,m2;
ULONG r;
vptr line;
struct sheet *s= (struct sheet *)Mem::vmem_trans (scrap->sh);
struct view_rec *v;

if (scrap->sh == active_sheet) {
   redbox (locus, 202, eh_cancel);
   return FALSE;
   }
sheet_clear (scrap->sh);
scrap->mode= mode;
m1= *get_mark (startmark);
m2= *get_mark (endmark);
assert (m1.row <= m2.row);
r= m1.row;
switch (mode) {
   case view_rec::bk_lines:
         bulk_append_lines(s, r, m2.row);
  //    while (r < m2.row) {
  //       line= get_line (r);
  //       append_line_part (s, line, 0,-1);
  //       r++;
  //       }
      break;
   case view_rec::bk_stream:
      if (m1.row == m2.row) {
         line= get_line (r); /* stream on one line only? */
         append_line_part (s, line, m1.col, m2.col);
         }
      else {                              /* end of first line */
         line= get_line (r++);
         append_line_part (s, line, m1.col, -1);
         bulk_append_lines(s, r, m2.row);
      //   while (r < m2.row) {                    /* middle lines */
      //      line= get_line (r);     
      //      append_line_part (s, line, 0, -1);
      //      DBG_SHEET(dout<<" block_prim_copy stream mode line = "<<r<<endl);
      //      r++;
      //      }
         line= get_line (r);                        /* part of last line */
         append_line_part (s, line, 0, m2.col);
         }
      break;
   case view_rec::bk_columns:
      if (m2.col <= m1.col) /* no copy/cut if columnar block not properly defined */
         break;
      while (r <= m2.row) {
         line= get_line (r);
         append_line_part (s, line, m1.col, m2.col);
         r++;
         }
      break;
   default:
      assert (FALSE);
   }
if (cut)
   block_zap ();
else {
   v= (struct view_rec *)Mem::vmem_trans (active_view);
   v->block_active= view_rec::bk_off;   /* disable block */
   status_block (-1);
   shade_all (v);             /* and clear block attributes */
   }
scrap->valid = TRUE;
return TRUE;
}

/*********************************************************************/
SHORT scrap_write (char const* filename)
{
if (scrap == NULL) {
   redbox (locus, 201, eh_abort);
   return FALSE;
   }
return sheet_save (scrap->sh, filename);
}

/*************************************************************************
 if a block is defined, write it out without affecting the scrap.
 otherwise, write out the scrap.  
*********************************************************************/
bool block_write (char *filename)
{
bool b;
if (view_block_active()) {
   my_new_scrap();
   block_copy (FALSE);
   b= scrap_write (filename);
   b= restore_scrap();
   }
else b= scrap_write (filename);
return b;
}

/* **************************************** */

bool block_read (char const* filename)
{
bool b;
struct sheet *cur;
my_new_scrap ();
b= sheet_load (scrap->sh, filename, FALSE);
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_BLOCK1(dout<<" block_read, sheet_load ok, block_sheet = "<<(scrap->sh)<<endl);
if (b) {
   block_paste (FALSE);
   cur = (struct sheet *)Mem::vmem_trans (active_sheet);
   cur->changed = TRUE;
   DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
   DBG_BLOCK1(dout<<" block_read, block_paste ok "<<endl);
}
restore_scrap ();
DBG_MEM(memout<<" file " <<__FILE__<<" line "<<__LINE__<<checkmem2<<endl);
DBG_BLOCK1(dout<<" block_read, restore_scrap ok "<<endl);
return b;
}

/*********************************************************************/
void mark_block (SHORT corner)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
mark_t m = 0;
bool ok;

if (v->block_active == view_rec::bk_undefined) {
   DBG_BLOCK1(dout<<"mark_block creating new block "<<endl);  
   v->block_begin= new_mark();                /* allocate the block marks */
   v->block_end= new_mark();
   }
switch (corner) {
   case 0:  m= v->block_begin;   break;
   case 1:  m= v->block_end;     break;
   default:  assert(FALSE);
   }
mark_assign (m, v->cursor);           // set cursor to begin or end of block
ok= mark_greater(v->block_end, v->block_begin);
if (ok && v->block_mode == view_rec::bk_columns) {/* test for column mode */
   SHORT begin_col= get_mark (v->block_begin)->col;
   SHORT end_col= get_mark (v->block_end)->col;
   ok= begin_col < end_col;
   }
if (ok) {
   DBG_BLOCK1(dout<<"mark_block ok TRUE "<<endl);
   v->block_active= view_rec::bk_on;
//   view_redraw_block (active_view);
   shade_block (active_view, TRUE,0,0);
   }
else {
   if (v->block_active == view_rec::bk_on) /* turning it off */
      shade_all (v);
   v->block_active= view_rec::bk_off;
   DBG_BLOCK1(dout<<"mark_block turn block off "<<endl);
   }
status_block (v->block_active==view_rec::bk_on ? v->block_mode : -1);
}

/* **************************************** */
#pragma warn -eas
void block_changemode (SHORT mode)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
view_rec::block_active_t a= v->block_active;
assert (mode >= 0 && mode < 3);
v->block_mode= mode;
status_block (a==view_rec::bk_on ? mode : -1);
if (a == view_rec::bk_on) {
    // view_redraw_block (active_view);
   shade_block (active_view, TRUE,0,0);
  }
}
#pragma warn .eas

/* **************************************** */

bool block_copy (bool cut)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
if (v->block_active != view_rec::bk_on) {
   redbox (locus, 1, eh_cancel);
   return FALSE;
   }
if (!cut) 
   v->block_active= view_rec::bk_off;
status_block (-1);
return block_prim_copy (v->block_begin, v->block_end, v->block_mode, cut);
}

/* **************************************** */

bool block_paste (bool quote)
{
struct view_rec *v;
//struct mark_rec m;
struct sheet *s;
ULONG line_count;
struct line_header *h;
v= (struct view_rec *)Mem::vmem_trans (active_view);

if (!scrap) {  /* error trap */
   redbox (locus, 201, eh_abort);
   return FALSE;
   }
if (scrap->sh == active_sheet) {
   redbox (locus, 202, eh_cancel);
   return FALSE;
   }
s= (struct sheet *)Mem::vmem_trans (scrap->sh);
if (s == my_active_sheet) {
   redbox (locus, 202, eh_cancel);
   return FALSE;
   }

if (v->block_active == view_rec::bk_on) {
    if(!scrap->valid) {
       block_copy(FALSE);         // put marked block into scrap
    }       
}

line_count= s->line_count;
   mark_t cursor= v->cursor;
   struct mark_rec *m= get_mark (cursor);
   SHORT orig_col= m->col;
   ULONG lastline, row= 0, orig_row= m->row;

if (scrap->mode == view_rec::bk_columns) {          // column mode only
   vptr line;
//   struct line_header *h;
   bool insertblock= v->flags&1;

   lastline= ((struct sheet *)Mem::vmem_trans (v->sheet))->line_count-1;
   while (row < line_count) {
      if (m->row > lastline) {
         sheet_insert_blank_line (++lastline);   /* add new line if needed */
         m->row= orig_row + row;
         }
      line= get_line_anysheet (s, row);
      h= (struct line_header *)Mem::vmem_trans (line);
      if (insertblock) {
         sheet_addstr (cursor, (char *)(h+1), h->length);
      }
      else 
         sheet_ovwtchars (cursor, (char *)(h+1), h->length);
      m= get_mark(cursor);
      m->col= orig_col; /* restore cursor moved by sheet_addstr() */
      m->row++;
      row++;
      }
   m->row= orig_row;  /* place cursor at top of block */
   /* view_redisplay (active_view); */
   fillwords (v->touches, 0xffff, 4);  //inline
   v->flags |= 4;
   return TRUE;
}                                       // end column mode

if (scrap->mode == view_rec::bk_stream) {    // normal paste
   struct line_header *h;
   if (line_count == 1) {
      h= (struct line_header *)Mem::vmem_trans (get_line_anysheet (s,0));
      sheet_addstr (v->cursor,(char *)(h+1),h->length);
      return TRUE;
      }
   else {                          /* take care of first & last line parts */
      vptr line1, line2;
      line1= get_line_anysheet (s, 0);
      line2= get_line_anysheet (s, line_count-1);
      h= (struct line_header *)Mem::vmem_trans (line1);
      sheet_addstr (v->cursor, (char*)(h+1), h->length);
      sheet_splitline (v->cursor);
      h= (struct line_header *)Mem::vmem_trans (line2);
      sheet_addstr (v->cursor, (char*)(h+1), h->length);
      line_count -= 2;
      }
}
                         /* now drop through to moving whole lines */
m= get_mark (v->cursor);
if(v->flags & 32) {                           /* word_wrap mode */
  vptr line1;
  for(ULONG tmprow = 1; tmprow <= line_count; tmprow++) {
      line1= get_line_anysheet (s, tmprow);
      h= (struct line_header *)Mem::vmem_trans (line1);// DON'T THINK THIS WRAPS
      sheet_addstr (v->cursor, (char*)(h+1), h->length);
      sheet_splitline (v->cursor);
  }
}
else {
   sheet_copylines (scrap->sh, (scrap->mode == view_rec::bk_lines)?0:1, m->row, line_count);
}
   if (quote) {
     mark_t org_cursor = v->cursor;          // save org cursor for quote
     struct mark_rec* org_pos = get_mark(org_cursor); 
     org_pos->row = orig_row;
     org_pos->col = orig_col;
     int l = strlen(v->QuotePrefix);
     for(int n = 1; n < s->line_count; n++) {
         sheet_addstr(org_cursor, v->QuotePrefix, l);
         org_pos->row++;
         org_pos->col -= l;
      }
   }

return TRUE;
}

/* **************************************** */

void block_zap (void)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
mark_t startmark= v->block_begin,
       endmark  = v->block_end;
struct mark_rec start,end;

if (v->block_active != view_rec::bk_on) return;
start= *get_mark (startmark);
end=   *get_mark (endmark);
switch (v->block_mode) {
   case view_rec::bk_lines:
      sheet_delete_lines (start.row,end.row-start.row);
      break;
   case view_rec::bk_stream:
      if (start.row == end.row)   /* one line stream */
         while (start.col++ < end.col)
            sheet_delchar (startmark);
      else {
         if (start.col > 0) {     /* multiple line stream */
            sheet_splitline (startmark);
            start.row++;
            end.row++;
            }
         if (end.col > 0) {
            sheet_splitline (endmark);
            end.row++;
            }
         sheet_delete_lines (start.row, end.row - start.row);
         if (end.col > 0) sheet_joinlines (endmark);
         }
      break;
   case view_rec::bk_columns: {
      SHORT col_count, line_count= end.row - start.row + 1;
      LONG orig_row= start.row;

      while (line_count--) {
         col_count= end.col - start.col;
         while (col_count--)                /* delete column's range in row */
            sheet_delchar (startmark);
         get_mark(startmark)->row++;
         }
      get_mark (startmark)->row= orig_row;  /* restore original cursor row */
      }
      break;
   }
v->block_active= view_rec::bk_off;
shade_all (v);
status_block (-1);
}

/* **************************************** */

void block_adjust (SHORT dist)
{
SHORT i;
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);
mark_t mark;
struct mark_rec *array, *m;
ULONG startrow, endrow;

array= get_mark_array (active_sheet);
v= (struct view_rec *)Mem::vmem_trans (active_view);
if (v->block_active != view_rec::bk_on) return;
startrow= array[v->block_begin].row;
endrow= array[v->block_end].row;
if (v->block_mode == view_rec::bk_lines || array[v->block_end].col == 0) 
    endrow--;

mark= new_mark ();
m= &array[mark];
m->row= startrow;
if (dist > 0)
   while (startrow++ <= endrow) {
      m->col= 0;
      for (i= 0; i < dist; i++)
         sheet_addchar (mark, ' ');
      m->row++;
      }
else
   while (startrow++ <= endrow) {
      m->col= 0;
      for (i= 0; i > dist; i--)
         sheet_delchar (mark);
      m->row++;
      }
cancel_mark (mark);
}

/* **************************************** */

void block_jump (SHORT pos)
{
struct view_rec *v= (struct view_rec *)Mem::vmem_trans (active_view);

if (v->block_active == view_rec::bk_undefined) return;
if (pos == 0) /* beginning of block */
   mark_assign (v->cursor, v->block_begin);
else mark_assign (v->cursor, v->block_end);
}

/* **************************************** */

void block_view (void)
{
active_sheet= scrap->sh;
new_standard_view (1);
}

/* **************************************** */

void block_print (void)
{
if (view_block_active ()) {
   my_new_scrap ();
   block_copy (FALSE);
   sheet_print (scrap->sh);
   restore_scrap ();
   }
}

/* **************************************** */

void scrap_print (void)
{
if (scrap == NULL)
   redbox (locus, 201, eh_abort);
else sheet_print (scrap->sh);
}

