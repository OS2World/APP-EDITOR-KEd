//
//           PROTO.H     dummy file that contains ALL the function
//                       call prototypes in KED v 2.08
//
//                       complete on April 3, 1994
//
//

void err_exit(const char *msg, USHORT number);
vptr new_view_general (SHORT height,SHORT width,SHORT ypos,SHORT xpos,USHORT flags,vptr sheet);
void new_view (SHORT height, SHORT width, SHORT ypos, SHORT xpos);
void new_standard_view (SHORT index);
void delete_view(void);                     /* kill the current viewer */
void view_make_active (vptr new_view);
void view_scroll (enum udlr direction,ULONG distance);
void view_unlink_twll(void);
void view_delete_all (vptr view_v);       /* called by sheet when deleted */
void link_in_top (struct view_rec *v, vptr view_v);
void view_flush (vptr view_v);
bool view_cursor_inrange (struct view_rec **view,SHORT *wrow,SHORT *wcol);
USHORT view_getkey(void);
void view_redisplay (vptr view_v);
void view_clear (vptr view_v);
void view_linetouch (vptr view_v, ULONG line);
void view_lineadd (vptr view_v, ULONG line);
void view_linedel (vptr view_v, USHORT line, SHORT count);
void view_switch_to_next(void);
void view_switch_to_prev(void);
void view_bring_to_front(void);
vptr find_view (char const* name);
bool view_block_active (void);
void view_addkey_any (vptr view_v, char c);
void view_addkey (char c);
void view_addstring (char const* s);
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
void go_end(void);
void go_beg(void);
void view_gotoline (ULONG line);/* does range checking on the line number */
void view_zapline(void);
bool view_insertmode (SHORT mode); /*  0-off  1-on  2-toggle  3-ask only */
void view_addchar_w_margins(struct view_rec *v);// add char with margins 10-17-93
void indent_left_margin (struct view_rec *v);
void move_up(void);
void move_down(void);
void move_right(void);
void move_left(void);
void  select_view(void);
SHORT view_packup (void*);
SHORT view_unpack (void*);
vptr view_pick(void);                     /* display a list and choose one */
void view_move_window (enum udlr direction,SHORT count);
void view_size_window (enum udlr direction,SHORT count);
SHORT view_zoom_driver(void);
void view_reshape (SHORT height, SHORT width, SHORT y_pos, SHORT x_pos);
void view_setzoom (void);                /* memorize current position */
void view_zoom(void);
void view_delete_to_eol (void);
void view_delete_to_bol (void);
void view_shift (enum udlr direction);
void view_match (void);
void view_delete_word (enum udlr direction);
void view_move_nextword (enum udlr direction);
bool locate_view (char const* s, SHORT option); /* 1-load if needed  2-always reload */
SHORT view_pickword (char *buffer, SHORT bufsize, char const *legalchars);
void view_changecase (SHORT mode);
void next_word(void);
void prev_word(void);
bool view_dont_throw_away (vptr view_v);     /* FALSE if user picked Cancel */
bool view_assure_all_saved (void);           /* FALSE if user picked Cancel */
bool view_load_file (char const* name, char const* default_dir, char const* default_ext);
bool view_save_file (char const* default_dir, char const* default_ext);
bool view_resave_file(void);             /* save, but don't prompt for a name */
void save_file(void);
void view_redraw_block (vptr view_v);
void shade_block (vptr view_v, bool print,SHORT start,SHORT end);
void shade_all (struct view_rec*);
void view_redraw (vptr view_v,SHORT line1,SHORT linecount, SHORT col1,  SHORT colcount);
void searchmenu (void);
bool view_search (const char* search_str, const char* repl_str, SHORT direction,USHORT flags, bool find_next);
void view_search_multifiles (const char* search_str, const char* repl_str, SHORT direction,USHORT flags, bool findnext);
void view_search_next (void);
void view_search_prev (void);
void beep_ok(void);
char* num2str (char* s, LONG n, SHORT len, char fillchar);
char* longnum2str (char* s, long n, SHORT len, char fillchar);
char* num2str_fast (char* s, LONG n, SHORT len);
char* longnum2str_fast (char* s, long n, SHORT len);
void left_just (char* s, SHORT len);
SHORT strcmp_ni (char const* s1, char const* s2, SHORT len);
void fillwords (USHORT* dest, USHORT val, USHORT count);
void cdecl memset_even (void* dest, char c, SHORT count);
inline SHORT upcase(SHORT c)
inline bool intersect (twocorner *a, twocorner *b, twocorner *result)
void make_mask (vptr w_v);
void extend_mask (vptr last_v, vptr mask_v, twocorner *mask_pos, twocorner *zone);
void display_off (vptr w_v);
void stomp (twocorner *shadow, vptr w_v, vptr firstnot_v);
void unstomp (twocorner *shadow, vptr w_v, vptr firstnot_v);
void display_on (vptr w_v);
extern void display_window(vptr, twocorner *);
extern void display_window_fast(struct window_rec *, twocorner *);
void blank_screen(vptr w_v, twocorner *box, USHORT fillvalue);
void draw_screen (twocorner* box);
extern void cdecl set_int24(void);
bool critical_handler (bool allow_retry);#ifndef FILELIST_H
void filelist_set (vptr view);
char *filelist_next (void);
char *filelist_prev (void);
char *filelist_fetch (void);
void filelist_top (void);
void filelist_bottom (void);
bool filelist_exists (bool create);
SHORT search (const char* search_str, const char* replace_str,mark_t cursor_mark, mark_t tail_mark, mark_t start_mark, mark_t end_mark,SHORT direction, USHORT flags);
bool regexp_search (char *string, SHORT start_col, char **start, char **end, SHORT len, USHORT flags);
bool regexp_compile (char const *string);
SHORT view_verify_replace (mark_t head, mark_t tail, char const *found_str,SHORT found_len, char const *replace_str, bool inblock);
bool window_init (void);           /* initialize window system */
void window_shutdown(void);
void window_redraw_all(void);   /* VBWIND  */
void display_on (window_t w_v);
void display_off (window_t w_v);
void screen_save (bool on);     /* TRUE-save screen  FALSE-restore screen */
void screen_swap (void);
window_t create_window (USHORT height,USHORT width,USHORT y_pos,USHORT x_pos);
void destroy_window (window_t w_v);
void clear_window (window_t w_v, USHORT r1,USHORT r2,USHORT c1,USHORT c2, USHORT fillvalue);
void put_string (window_t w_v, SHORT row, SHORT col, char const* s);
void put_string_fast (window_t w_v, SHORT row, SHORT col, char const* s);
void put_string_len (window_t w_v, SHORT row, SHORT col, char const* s, SHORT length);
void print_string (window_t w_v, SHORT row, SHORT col, char const* s);
enum just_mode { CenterJust, RightJust, LEFTJust };
void put_just_string (window_t w_v, SHORT row, enum just_mode justify, char const* s);
void pad_window (window_t w_v, SHORT row, SHORT col, char c, SHORT length);
void highlight (window_t w_v,SHORT row,SHORT col, byte attrib,SHORT count);
void window_border (window_t w_v, char const* s, byte attrib);
void window_border_size (window_t w_v,USHORT bwidth[4]);
void window_get_position (window_t w_v, SHORT *row, SHORT *col);
void window_get_size (window_t w_v, SHORT *height, SHORT *width);
void put_win_row (window_t w_v, char const * s,SHORT len,SHORT row);
void shade_win_row (window_t w_v, SHORT row, byte off_color, byte on_color, SHORT b_first, SHORT b_last);
USHORT window_getkey (window_t w_v,SHORT row, SHORT col, byte color);
void move_window (window_t w_v, enum udlr direction, SHORT count);
void resize_window (window_t w_v, SHORT mode, SHORT newval,USHORT fillval);
void reshape_window (window_t w_v, SHORT height, SHORT width, SHORT y_pos, SHORT x_pos);
bool keymove_window (window_t w, SHORT location, SHORT key);
void scroll_window (window_t w_v, enum udlr direction, SHORT count);
USHORT get_cursor(void);
void set_cursor_pos(USHORT position);
void set_cursor(USHORT sline, USHORT eline);
enum err_choices redbox (SHORT locus, SHORT num, enum err_choices choices);
enum err_choices redbox_s (SHORT locus, SHORT num, enum err_choices choices, char const* s);
enum err_choices os2_redbox_s (SHORT locus, SHORT num, SHORT errnum, enum err_choices choices, char const* s);extern char cwd[FNAME_LEN];
char *find_basename (char const *filename);
char *find_extension (char *filename);
void replace_basename (char *filename, char const* newname);
char *find_basesubdir (char *filename);
bool filename_iswild (char const* filename);
void normalize_filename (char *filename, char const* default_path, char const* default_extension);
void add_subdir (char *filename, char const* newnode);
void delta_filename (char *buf, char const* original, char const* delta);
bool filename_samedir (char const* name1, char const* name2);
bool get_directory (SHORT drive_letter, char *s);
bool get_current_directory (char *s);
bool dot_dir (char const* s);
bool dotdot_dir (char const* s);
void normalize_filename_2 (char *filename, char const* default_path,char const* default_extension);
bool dirlist (char const *title, char *filename, enum dirlist_flags flags, SHORT y, SHORT x, SHORT height);
bool dirlist_core (char const *title, char *filename, enum dirlist_flags flags, SHORT y, SHORT x, SHORT height);
void commando (char *path);
window_t make_pathwind (SHORT y, SHORT x, SHORT height, SHORT width, const char *border, SHORT py, SHORT px, SHORT pw);
void set_pathwind (window_t w, SHORT py, SHORT px, SHORT pw);
bool file_findfirst (char const* name, struct dir_entry *e, USHORT dirflags);
bool file_findnext (struct dir_entry *e);
void display_entry (struct dirlist_data *d, struct dir_entry *e, SHORT row);
void redisplay_dirlist (struct dirlist_data *d);
void display_path (char const *filename,SHORT y,SHORT x,SHORT width);
void read_list (struct dirlist_data *d, char *filename);
void show_bytesfree (struct dirlist_data *d);
void select_attr_flags (USHORT *flags);
void update_entry (struct dirlist_data *d, SHORT index);
void sort_dirlist (struct dirlist_data *d);
SHORT picklist (struct picklist_portal *p, window_t w, struct picklist_thunk *t, SHORT count, SHORT location);/* LNED.H */
void lned_addkey (struct ln_dat *, char);
void lned_delkey (struct ln_dat *);
void lned_move_cursor (struct ln_dat *, SHORT);
void lned_redisplay (struct ln_dat *);/********************************************************************
USHORT *dmm_configure(USHORT n, USHORT *temp);
USHORT dmm_alloc (USHORT *Base, USHORT size);
void dmm_free (USHORT *Base, USHORT p);
void shell_and_run (void);
void shell_menu (void);
void os2_shell(void);
void dummy(void);  /* does nothing */
extern void key_macro (const char *), (*KeyBackground)(void);
extern USHORT OldKey;
USHORT getkey(void);
bool key_ready(void);    /* see if a key is ready */
extern void key_capture (char* buf, SHORT buflen, SHORT stopkey, void (* callback) (SHORT status, SHORT len));
bool keybd_setup(void);
bool numlock_set(bool on_off);
void numlock_toggle(void);
void numlock_toggle2(SHORT mode);
mark_t new_mark(void);                   /* create a mark in active sheet */
mark_t new_mark_any (vptr sheet);        /* create a mark in any sheet */
void cancel_mark (mark_t);               /* delete a mark */
struct mark_rec *get_mark (USHORT marknum);  
struct mark_rec *get_mark_fast (struct sheet *cur, USHORT marknum);
struct mark_rec *get_mark_array (vptr sheet);
bool move_mark (mark_t m, enum udlr direction, SHORT distance);
void mark_assign (mark_t dest, mark_t source);
bool mark_greater (mark_t, mark_t);
bool stream_mark (mark_t mark, enum udlr direction);
void mark_addcol (mark_t);
void mark_delcol (mark_t);
void mark_splitline (mark_t);
void mark_joinlines (ULONG row, SHORT col);
void mark_insertline (ULONG row);
void mark_insertlines (ULONG row, USHORT count);
void mark_dellines (ULONG row, SHORT count);
bool stream_mark_fast (struct sheet *sh, struct mark_rec *m, enum udlr direction);
extern struct sheet *my_active_sheet;
void new_sheet(void);          /* create a standard sheet and make it active */
vptr new_sheet_general (USHORT flags);    /* create a sheet */
void delete_sheet (vptr sheet);             /* delete a sheet */
vptr get_line (ULONG n);
vptr get_line_anysheet (struct sheet *s,ULONG n);
struct line_header *line_fetch (vptr sheet, ULONG linenum);
void put_line (ULONG n, vptr line);
void put_line_anysheet (struct sheet* s,ULONG n, vptr line);
SHORT sheet_line_length (struct sheet *sh, ULONG row);   
SHORT sheet_peek (vptr sh, mark_t mark);
void append_line_part (struct sheet *s, vptr line,SHORT c1,SHORT c2);
void sheet_copylines (vptr s_v, ULONG from, ULONG to, USHORT count);
ULONG sheet_linecount (vptr sh);
SHORT sheet_peek (vptr sh, mark_t mark);
SHORT sheet_peek_fast (struct sheet *sh, struct mark_rec *m);
void sheet_ovwtchar (mark_t marknum, char c);
void sheet_ovwtchars (mark_t marknum, char const* string,SHORT length);
void sheet_addchar (mark_t, char);
void sheet_addchars (mark_t position, char const* string,SHORT length);
void sheet_delchar (mark_t);
void sheet_splitline (mark_t);
void sheet_wrapline(mark_t);  // word wrap mode
void sheet_addchars_wordwrap (mark_t position, char const* string,SHORT length);
void sheet_joinlines (mark_t);
void sheet_insert_line (ULONG row, vptr line);
void sheet_insert_blank_line (ULONG row);
void sheet_delete_lines (ULONG row,SHORT count);
void sheet_clear (vptr sheet_v);
vptr zero_line(void);                   /* create a new blank line */
void sheet_print (vptr sh);
void sheet_delete_view (vptr sheet_v, vptr view);
bool sheet_load (vptr sheet_to_load, char const* filename, bool remember_name);
SHORT sheet_save (vptr sheet_to_save, char const* filename);
inline struct mark_rec *get_mark_fast (USHORT marknum)
bool setting_load (char *name);
bool setting_save (char *name);
void setting_defaultcolors (SHORT n);
void edit_string (window_t w, SHORT row, SHORT col, SHORT width, char* s,SHORT maxlen, byte *colorset, USHORT flags);
bool prompt_box (char const* prompt, char *s, SHORT maxlen, char **recall,USHORT flags, SHORT help);
bool prompt_int (char const* prompt, SHORT *n, SHORT min, SHORT max, SHORT help);
bool prompt_long (char const* prompt, long *n, long min, long max, SHORT help);/*           INITKEYS header, holds definitions for default key mapping
enum action do_command(USHORT key);
bool my_new_scrap (void);
bool restore_scrap (void);
bool block_read (char const* filename);
bool block_write (char *filename);
SHORT scrap_write (char const* filename);
void scrap_print (void);
bool cursor_in_block (struct mark_rec *cursor, struct mark_rec *begin, struct mark_rec *end, SHORT blockmode);
void mark_block (SHORT corner); /* 0-begin  1-end */
void block_begin(void);
void block_end(void);
void block_changemode (SHORT mode);
bool block_copy (bool cut);  /* copy or cut a block to scrap */
bool block_paste (bool quote);
void block_zap (void);
void block_adjust (SHORT distance);
void block_print (void);
void block_jump (SHORT which_end);
void block_view (void);
SHORT pulldown_menu(void);
bool tryquit(void);
void tryquit2(void);
void blockmenu(void);
bool filemenu(void);
void printmenu(void);
void aboutmenu(void);
void  filelist_menu (void);
void  closewindow (void);
static void *mylib_allocmem (USHORT nparas);
static void mylib_freemem (void *block);
static SHORT vmem_setup(void);
static void vmem_shutdown (void);
static inline void *vmem_trans(vptr p_in)  /* special function read-only */
vptr vmem_newpage(void);            /* allocate a page */
void vmem_droppage(vptr);           /* deallocate a page */
SHORT vmem_create_space(void);
void vmem_delete_space (SHORT);
void *vmem_alloc(SHORT space, USHORT size, vptr* v);
void vmem_free (vptr);
SHORT vmem_burst_start (SHORT space);
void vmem_burst_end(void);
void* vmem_burst_alloc (USHORT size, vptr* v);
bool get_load_name (char *default_name, char *fname, char const *default_dir, char const *default_ext);
bool get_save_name (char *fname, char const* default_dir, char const* default_ext);
void status_setup(void);
void status_redraw(void);
void status_colorchange(void);
void status_name(void);
void status_row (ULONG row);
void status_col (SHORT col);
void status_delta (bool b); /* file changed mark */
void status_block (SHORT mode_switch);   /* -1 off, or block mode */
void status_key_recording (bool onoff);
void status_insertmode (bool insert_on);
void status_numlockmode (void);
void status_indentmode (bool indent_on);
void status_message (char const* );
void status_note (char const* );  /* send NULL to turn off */
void edset (void);
void edit_colorset (byte c[]);
bool edit_color (byte* c);
