/********************************************************
            SHEET.H  
**********************************************************/
#ifndef SHEET_H
#define SHEET_H

typedef USHORT mark_t;  /* index into mark list */
struct mark_rec {
//   ULONG row;
   ULONG row;
   SHORT  col;
   };

//const TIERCOUNT = 16384 / (VMEM_PAGESIZE/16);  // was 32
//#define TIERCOUNT 64                // mod 12-19-92 barfs at >128k lines
#define TIERCOUNT 128                // mod 12-19-92 barfs at >256k lines
#define TIER_SIZE  (VMEM_PAGESIZE/4)       // how many lines per tier = 2048
#define MAXLINES (TIERCOUNT * TIER_SIZE)

struct sheet {
   USHORT mark_count, mark_capacity;           /* marks */
   vptr marks_v;                               /* list of all the marks */
   SHORT space;                                /* buffer  */
   ULONG line_count;
   vptr tiers[TIERCOUNT];
                                          /* attached to a view window */
   USHORT view_count;         /* used to give each viewer a unique number */
   vptr viewer;                           /* first viewer in a linked list */
   char filename[FNAME_LEN];                  /* the file name */
   char load_filename[FNAME_LEN];
   bool changed;
   USHORT flags;           /* flags:  1-OK to have no viewers; Preserve
                                            2-OK to discard    */
   vptr next;                                /* list of all sheets */
   };

struct line_header {
   SHORT  length;                          
   USHORT capacity;             
   };
                                         /* MARK.C */
mark_t new_mark(void);                   /* create a mark in active sheet */
mark_t new_mark_any (vptr sheet);        /* create a mark in any sheet */
void cancel_mark (mark_t);               /* delete a mark */
                  /* called by anyone to find out about the mark position */
struct mark_rec *get_mark (USHORT marknum);  
ULONG get_mark_row(USHORT marknum);             // return row for the mark
struct mark_rec *get_mark_fast (struct sheet *cur, USHORT marknum);
struct mark_rec *get_mark_array (vptr sheet);
                                        /* called by anyone to move a mark */
bool move_mark (mark_t m, enum udlr direction, SHORT distance);
void mark_assign (mark_t dest, mark_t source);
bool mark_greater (mark_t, mark_t);
bool stream_mark (mark_t mark, enum udlr direction);


/* these are called by the sheet to adjust the mark to changes in the sheet */
void mark_addcol (mark_t);
void mark_delcol (mark_t);
void mark_splitline (mark_t);
void mark_joinlines (ULONG row, SHORT col);
void mark_insertline (ULONG row);
void mark_insertlines (ULONG row, USHORT count);
void mark_dellines (ULONG row, SHORT count);
bool stream_mark_fast (struct sheet *sh, struct mark_rec *m, enum udlr direction);

extern vptr active_sheet;                /* sheet.c */
//extern vptr all_sheets;
extern SHORT sheet_mem;
extern struct sheet *my_active_sheet;
                                            /* called by anyone */
void new_sheet(void);          /* create a standard sheet and make it active */
vptr new_sheet_general (USHORT flags);    /* create a sheet */
void delete_sheet (vptr sheet);             /* delete a sheet */
vptr get_line (ULONG n);
vptr get_line_anysheet (struct sheet *s,ULONG n);
struct line_header *line_fetch (vptr sheet, ULONG linenum);
struct line_header *line_fetch2 (ULONG linenum);
void put_line (ULONG n, vptr line);
void put_line_anysheet (struct sheet* s,ULONG n, vptr line);
SHORT sheet_line_length (struct sheet *sh, ULONG row);   
SHORT sheet_peek (vptr sh, mark_t mark);
void sheet_insert_line (ULONG row, vptr line);
                                    /* these could be called by anyone,
                                      but are made for the Block system */
void append_line_part (struct sheet *s, vptr line,SHORT c1,SHORT c2);
void sheet_copylines (vptr s_v, ULONG from, ULONG to, USHORT count);
                               // special for large block copies
void bulk_append_lines(struct sheet *dest, ULONG start, ULONG end);
                                    /* these could be called by anyone,
                                      but are usually handled by higher
                                      level functions in the viewer */
ULONG sheet_linecount (vptr sh);
SHORT sheet_peek (vptr sh, mark_t mark);
SHORT sheet_peek_fast (struct sheet *sh, struct mark_rec *m);
void sheet_ovwtchar (mark_t marknum, char c);
void sheet_ovwtchars (mark_t marknum, const char* string,SHORT length);

void sheet_addchar (mark_t, char);
void sheet_addstr(mark_t position, const char* string, const SHORT length);

void sheet_delchar (mark_t);
void sheet_splitline (mark_t);
void sheet_joinlines (mark_t);
void sheet_insert_line (ULONG row, vptr line);
void sheet_insert_blank_line (ULONG row);
void sheet_delete_lines (ULONG row,SHORT count);
void sheet_clear (vptr sheet_v);
vptr zero_line(void);                   /* create a new blank line */
void sheet_print (vptr sh);

void sheet_wrapline(mark_rec& cursor,mark_rec& wrap_loc);   //word wrap mode
void sheet_addchars_wordwrap (mark_t position, const char* string,SHORT length);
void move_word_to_prev_line(mark_rec& wrap_loc);
//void move_word_to_next_line(const mark_rec& wrap_loc);
void move_word_to_next_line(mark_rec& wrap_loc,line_header& l);

                          /* this is called by the viewer that has just
                             been deleted, in order to unlink it from the
                             list of viewers belonging to this sheet. */
void sheet_delete_view (vptr sheet_v, vptr view);

                                   /* FILE.C */
bool sheet_load (vptr sheet_to_load, char const* filename, bool remember_name);
SHORT sheet_save (vptr sheet_to_save, char const* filename);
     /* if `filename' is NULL, it will save as the remembered name. */
    /* returns: 0-OK.  1-cannot create  2-error writing */

inline struct mark_rec *get_mark_fast (USHORT marknum)
{
struct sheet *cur= (struct sheet *)Mem::vmem_trans (active_sheet);
struct mark_rec *marks= (struct mark_rec *)Mem::vmem_trans (cur->marks_v);
return marks+marknum;
}

#endif
