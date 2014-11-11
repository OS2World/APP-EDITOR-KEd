/*****************************************************
file: MARK.C        Copyright 1989 by Dlugosz Software
   keep track of marks in a sheet
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "sheet.h"
#include <string.h>  /* need memcpy() */

/**********************************************************************/

mark_t new_mark_any (vptr sheet_v)
{                                 /* allocate a new mark on any sheet*/
struct sheet *cur= (struct sheet *)Mem::vmem_trans(sheet_v);
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans(cur->marks_v);
USHORT n;
                                             /* try to find an unused one */
for (n= 0;  n < cur->mark_count;  n++) {
   if (marks[n].row == 0xffffffff) {             /* found one! */
      marks[n].row= 0;                      /* don't leave marked as unused */
      return n;
      }
   }
                                            /* nope.  get a fresh one */
n= cur->mark_count++;
if (cur->mark_count == cur->mark_capacity) {   /* grow the array */
   vptr new_list_v;
   struct mark_rec *new_list;
   USHORT new_capacity= cur->mark_capacity += cur->mark_capacity/2;
   new_list= (struct mark_rec *)vmem_alloc (cur->space, new_capacity * sizeof (struct mark_rec), &new_list_v);
                                  /* copy the old list into the new list */
   memcpy (new_list, marks, cur->mark_capacity * sizeof (struct mark_rec));
   vmem_free (cur->marks_v);
                                          /* put the new one in place */
   cur->mark_capacity= new_capacity;
   cur->marks_v= new_list_v;
   marks= new_list;
   }
marks[n].row= 0;  /* make sure it is not a 0xffff */
marks[n].col= 0;  /* temporary for testing */
return n;
}

/* **************************************** */

mark_t new_mark (void)
{
return new_mark_any (active_sheet);
}

/********************************************************************** */

void cancel_mark (mark_t n)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;     // mod 8-25-93
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);

marks[n].row= 0xffffffff;
}

/* ****************************************************************** */

struct mark_rec *get_mark (USHORT marknum)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur = my_active_sheet;                  /* mod 8-25-93  */
//struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);

                                           /* debugging stuff */
//assert (marknum < cur->mark_count);     // MOD 9-25-92
//assert (marks->row != 0xffff);

return marks+marknum;
}

/***********************************************************************
     return ULONG row for this mark
**************************************************************************/
ULONG get_mark_row(USHORT marknum)
{
struct sheet *cur = my_active_sheet;                  /* mod 8-25-93  */
struct mark_rec *marks= (mark_rec *)Mem::vmem_trans(cur->marks_v);

                                           /* debugging stuff */
//assert (marknum < cur->mark_count);     // MOD 9-25-92
//assert (marks->row != 0xffff);

return (marks+marknum)->row;
}

/* **************************************** */

struct mark_rec *get_mark_fast (struct sheet *cur,USHORT marknum)
{
  /* passes sheet pointer for slightly faster performance  */
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
return marks+marknum;
}

/* **************************************** */

struct mark_rec *get_mark_array (vptr sheet)
{
struct sheet *cur= (struct sheet *)Mem::vmem_trans (sheet);
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
return marks;
}

/* **************************************** */

bool move_mark (mark_t m, enum udlr direction, SHORT distance)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;      /* mod 8-25-93  */
struct mark_rec *mark= m + (struct mark_rec *)Mem::vmem_trans (cur->marks_v);

switch (direction) {
   case UP:
      if (mark->row >= distance) {
         mark->row -= distance;
         return TRUE;
         }
      break;
   case RIGHT:
      {
      SHORT newcol= mark->col + distance;
      if (newcol < 16000) {
         mark->col = newcol;
         return TRUE;
         }
      }
      break;
   case DOWN:
      {
      ULONG newrow= mark->row + distance;
      if (newrow < cur->line_count) {
         mark->row= newrow;
         return TRUE;
         }
      }
      break;
   case LEFT:
      if (mark->col >= distance) {
         mark->col -= distance;
         return TRUE;
         }
      break;
   }
return FALSE;
}
#ifdef FUCKUP
/* **************************************** */

bool stream_mark (mark_t mark, enum udlr direction)
{
/* Moves mark through file sequentially, wrapping at each line */
/* Accesses mark directly */
/* returns FALSE if mark not moved */
//struct sheet *sh= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *sh= my_active_sheet;        // mod 8-25-93
struct mark_rec *m= get_mark (mark);

switch (direction) {
   case UP:
      if (m->row == 0) return FALSE;
      else m->row--;
      break;
   case DOWN:
      if (m->row >= sh->line_count-1) return FALSE;
      else m->row++;
      break;
   case LEFT:
      if (m->col == 0) {
         if (m->row == 0) return FALSE;
         m->col= sheet_line_length (sh,--m->row);
         }
      else m->col--;
      break;
   case Right:
      if (m->col >= sheet_line_length (sh, m->row)) {
         if (m->row >= sh->line_count-1) return FALSE;
         m->row++;
         m->col= 0;
         }
      else m->col++;
      break;
   }
return TRUE;
}
#endif
/* **************************************** */
bool stream_mark_fast (struct sheet *sh, struct mark_rec *m, enum udlr direction)
{
/* Moves mark through file sequentially, wrapping at each line */
/* Accesses mark directly */
/* returns FALSE if mark not moved */

switch (direction) {
   case UP:
      if (m->row == 0) return FALSE;
      else m->row--;
      break;
   case DOWN:
      if (m->row >= sh->line_count-1) return FALSE;
      else m->row++;
      break;
   case LEFT:
      if (m->col == 0) {
         if (m->row == 0) return FALSE;
         m->col= sheet_line_length (sh,--m->row);
         }
      else m->col--;
      break;
   case RIGHT:
      if (m->col >= sheet_line_length (sh, m->row)) {
         if (m->row >= sh->line_count-1) return FALSE;
         m->row++;
         m->col= 0;
         }
      else m->col++;
      break;
   }
return TRUE;
}

/* **************************************** */
#ifdef FUCKUP
void mark_addcol (mark_t m)
{
struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
//struct sheet *cur= my_active_sheet;
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
ULONG row= mark[m].row;
SHORT col= mark[m].col;
SHORT loop= cur->mark_count;

while (loop--) {
   if (mark->row == row && mark->col >= col)
      mark->col++;
   mark++;
   }
}
#endif
/* **************************************** */
#ifdef FUCKUP
void mark_delcol (mark_t m)
{
struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
//struct sheet *cur= my_active_sheet;
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
ULONG row= mark[m].row;
SHORT col= mark[m].col;
SHORT loop= cur->mark_count;

while (loop--) {
   if (mark->row == row && mark->col > col)
      mark->col--;
   mark++;
   }
}
#endif
/* **************************************** */

void mark_splitline (mark_t m)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;      // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
ULONG row= mark[m].row;
SHORT col= mark[m].col;
SHORT loop= cur->mark_count;

while (loop--) {                // move any other marks on this line
   if (mark->row == row) {
      if (mark->col >= col) {  /* bump to next line */
         mark->row++;
         mark->col -= col;
         }
      }
   /* else if (mark->row > row && mark->row != 0xffff) mark->row++; */
   mark++;
   }
}

/* **************************************** */

void mark_joinlines (ULONG row, SHORT col)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;      // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
SHORT loop= cur->mark_count;

while (loop--) {
   if (mark->row == row) {
      mark->row--;
      mark->col += col;
      }
   mark++;
   }
}

/* **************************************** */

void mark_insertline (ULONG row)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;         // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
SHORT loop= cur->mark_count;

while (loop--) {
   if (mark->row >= row && mark->row != 0xffffffff) {
      mark->row++;
   }
   mark++;
   }
}

/* **************************************** */

void mark_insertlines (ULONG row, USHORT count)
{
//struct sheet *cur= my_active_sheet;         // mod 8-25-93
//struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
//SHORT loop= cur->mark_count;
while (count--) {
   mark_insertline (row);
//   loop= cur->mark_count;
//   while (loop--) {
//      if (mark->row >= row && mark->row != 0xffffffff) {
//         mark->row++;
//      }
//      mark++;
//   }
}
}

/* **************************************** */

void mark_dellines (ULONG row, SHORT count)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;         // mod 8-25-93
struct mark_rec *mark= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
SHORT loop= cur->mark_count;
ULONG bottomrow= row+count, linecount= cur->line_count;

while (loop--) {
   if (mark->row >= row && mark->row != 0xffffffff) {
      if (mark->row >= bottomrow) 
         mark->row -= count;              /* moves up */
      else {                              /* on a row that was deleted */
         if (mark->row >= linecount) mark->row= linecount-1;
         else mark->row= row;
         mark->col= 0;
         }
      }
   mark++;
   }
}

/* **************************************** */

void mark_assign (mark_t dest, mark_t source)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;       // mod 8-25-93
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
marks[dest]= marks[source];
}

/* **************************************** */

bool mark_greater (mark_t a, mark_t b)
{
//struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct sheet *cur= my_active_sheet;      // mod 8-25-93
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
struct mark_rec *m1= marks+a, *m2= marks+b;
return (m1->row > m2->row || (m1->row == m2->row && m1->col > m2->col));
}

/* **************************************** */
