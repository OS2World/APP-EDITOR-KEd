/******************************************************************
        ED_CMD.H     holds function prototypes for wrapper functions
                     called by keystrokes

        rev 1.0   April 6, 1994
********************************************************************/
#ifndef ED_CMD_H
#define ED_CMD_H

SHORT help(void);

SHORT go_up(void);
SHORT go_down(void);
SHORT go_right(void);
SHORT go_left(void);
SHORT go_eol(void);
SHORT go_bol(void);
SHORT go_pgup(void);
SHORT go_pgdown(void);
SHORT go_bof(void);
SHORT go_eof(void);
SHORT go_next_word(void);
SHORT go_prev_word(void);
SHORT go_block_start(void);
SHORT go_block_end(void);

//SHORT  select_view(void);

SHORT file_quit(void);
SHORT file_save(void);
SHORT file_next(void);
SHORT file_prev(void);
SHORT file_load(void);
SHORT file_save_all(void);

SHORT toggle_insert(void);
                     
SHORT menu_print(void);
SHORT menu_file(void);
SHORT menu_block(void);
SHORT menu_search(void);
SHORT menu_settings(void);
SHORT menu_shell(void);

SHORT ascii_chart(void);
SHORT show_version(void);
SHORT show_files(void);
SHORT show_os_screen(void);

SHORT block_start(void);
SHORT block_end(void);
SHORT block_copy(void);
SHORT block_cut(void);
SHORT block_paste(void);
SHORT block_quote(void);
SHORT block_delete(void);
//SHORT block_unmark();

SHORT reformat(void);

SHORT ked_exit(void);                                                                       
SHORT ked_exit2(void);
SHORT ked_shell(void);
SHORT ked_shell_run(void);

SHORT del_char(void);
SHORT del_line(void);
SHORT del_eol(void);
SHORT del_bol(void);
SHORT del_word_right(void);
SHORT del_word_left(void);

SHORT search_start(void);
SHORT search_next(void);
SHORT search_prev(void);

SHORT macro_begin(void);
SHORT macro_end(void);
SHORT macro_play(void);

SHORT prompt_go_line(void);

#endif
