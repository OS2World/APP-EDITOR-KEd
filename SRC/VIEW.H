/* VIEW.H */

#ifndef VIEW_H
#define VIEW_H

#ifndef TOW_H
#include "basewin.h"
#endif

struct view_rec {
   USHORT view_num;          // which view is this, used for basewin....
   vptr next;                /* next view_rec attached to same sheet */
   vptr under;               /* next view_rec under this one on the screen */
   vptr over;                /* next view_rec above this one on the screen */
   vptr sheet;               /* the sheet being viewed */
   USHORT number;            /* more than 1 on same sheet? */
   ULONG firstrow;        // mod 12-19-92 was USHORT
   SHORT firstcol;
   USHORT touches[4];
   SHORT length;            // length of view window, default=22 lines
   SHORT width;                
   SHORT top_border;       // screen line for start of view ( 1-24)
   mark_t cursor;          // current location
//   mark_rec mycursor;
   mark_t eol;
   USHORT flags;
                             /* 1-insert mode, 2-auto indent, 4-needs update
                                8-tabs on,    16-save tabs  32-word_wrap */
                                
   byte colors[6];
   SHORT vert_scroll_margin, horz_scroll_margin;
   mark_t block_begin, block_end;                    /* block moves */
   enum block_mode_t { bk_stream, bk_lines, bk_columns } block_mode;
   enum block_active_t { bk_undefined, bk_off, bk_on } block_active;
   SHORT tab_size;
//   USHORT LeftMargin;        /* ... other fields */
   USHORT RightMargin;
   char* QuotePrefix;
   };

extern vptr active_view, top_view, bottom_view;
extern SHORT viewer_count;
//extern SHORT viewer_number;
extern struct view_rec *my_active_view;    /* mod 6-92  */
//extern bool view_is_zoomed;
                                /* these called by high level application */

                                /*  VIEWER.C                              */
vptr new_view_general (SHORT height,SHORT width,SHORT ypos,SHORT xpos,USHORT flags,vptr sheet);
void new_view (SHORT height, SHORT width, SHORT ypos, SHORT xpos);
void new_standard_view (SHORT index);
void delete_view(void);                     /* kill the current viewer */
void view_make_active (vptr new_view);
                                           /* these are used internally */
void view_scroll (enum udlr direction,ULONG distance);
void view_unlink_twll(void);

void view_delete_all (vptr view_v);       /* called by sheet when deleted */
void link_in_top (struct view_rec *v, vptr view_v);

                                         /* view_drw   */
void view_flush (vptr view_v);
void view_cursor_inrange (struct view_rec **view,SHORT *wrow,SHORT *wcol);
//USHORT view_getkey(void);
struct key_rec& view_getkey(void);
                                           /* these are used internally */
void view_redisplay (vptr view_v);
void view_clear (vptr view_v);
                                  /* these are called by the sheet to inform
                                  the viewer of changes that need displaying */
void view_linetouch (vptr view_v, ULONG line);
void view_lineadd (vptr view_v, ULONG line);
//void view_linedel (vptr view_v, ULONG line);
void view_linedel (vptr view_v, USHORT line, SHORT count);

                                         /*   editing commands   */
                                        /* view_com  */
void view_switch_to_next(void);
void view_switch_to_prev(void);
void view_bring_to_front(void);
vptr find_view (char const* name);
bool view_block_active (void);
void view_addkey_any (vptr view_v, char c);
void view_addkey (char c);
void view_addstring (const char* s);
void view_addstring_any (vptr view_v, char const* s);
void view_delchar(void);
void view_backspace(void);
void view_move_cursor (enum udlr);
bool view_stream_cursor (enum udlr);
SHORT view_peek (void);
void view_pageup(void);
void view_pagedown(void);
void view_gohome(void);
void view_goend(void);

void view_gotoline (ULONG line);/* does range checking on the line number */
void view_zapline(void);
bool view_insertmode (SHORT mode); /*  0-off  1-on  2-toggle  3-ask only */

void view_addchar_w_margins(struct view_rec *v);// add char with margins 10-17-93
//int last_word_len(char const *string); /* returns size of last word*/
void indent_left_margin (struct view_rec *v);
                               // indents a line to the Left_Margin
void reform_doc_down(int num_paragraphs);

void  select_view(void);
                                   /*   window commands   */
                                          /* view_pic   */
vptr view_pick(void);                     /* display a list and choose one */

void view_move_window (enum udlr direction,SHORT count);  /* view_win  */
void view_size_window (enum udlr direction,SHORT count);
SHORT window_move(void);                 // wrapper functs
SHORT window_size(void);
                                    /* view_zm  */
SHORT view_zoom_driver(void);
void view_reshape (SHORT height, SHORT width, SHORT y_pos, SHORT x_pos);
void view_setzoom (void);                /* memorize current position */
void view_zoom(void);

                                   /* extended edit functions */
#define VIEW_EXTENDED_COM 1          /*  view_ext  */
void view_delete_to_eol (void);
void view_delete_to_bol (void);
void view_shift (enum udlr direction);
void view_match (void);
void view_delete_word (enum udlr direction);
void view_move_nextword (enum udlr direction);
bool locate_view (char const* s, SHORT option); /* 1-load if needed  2-always reload */
SHORT view_pickword (char *buffer, SHORT bufsize, char const *legalchars);
void view_changecase (SHORT mode);

                                       //   file commands in VIEW_FIL.C  
bool view_dont_throw_away (vptr view_v);     // FALSE if user picked Cancel 
bool view_assure_all_saved (void);           // FALSE if user picked Cancel
bool view_load_file (char const* name, char const* default_dir, char const* default_ext);
bool view_save_file (char const* default_dir, char const* default_ext);
bool view_resave_file(void);          // save, but don't prompt for a name 
bool view_save_file_as(char const* default_dir);
void save_file(void);
void view_save_all(void);       // save all current sheets

                           /* these are called by the sheet to inform
                            the viewer of changes that need displaying */
                                  /*  redraw.c  */
void view_redraw_block (vptr view_v);
void shade_block (vptr view_v, bool print,SHORT start,SHORT end);
void shade_all (struct view_rec*);
//void view_redraw (vptr view_v,SHORT line1,SHORT linecount,
//                               SHORT col1,  SHORT colcount);
void view_redraw (vptr view_v,LONG line1,SHORT linecount,
                               SHORT col1,  SHORT colcount);

                                         /* view_sch header stuff */
SHORT searchmenu ();
bool view_search (const char* search_str, const char* repl_str, SHORT direction,USHORT flags, bool find_next);
void view_search_multifiles (const char* search_str, const char* repl_str, SHORT direction,USHORT flags, bool findnext);
void view_ask_search (void);
void view_search_next (void);
void view_search_prev (void);

#endif
