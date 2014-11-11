/*SHORT search (const char* search_str, const char* replace_str,
     mark_t cursor_mark, mark_t tail_mark, mark_t start_mark, mark_t end_mark,
     SHORT direction, USHORT flags, 
     SHORT (*verify_replace) (mark_t head, 
                              mark_t tail, 
                              char const *found_str, 
                              SHORT found_len, 
                              char const *replace_str, 
                              bool inblock));
*/
SHORT search (const char* search_str, const char* replace_str,
     mark_t cursor_mark, mark_t tail_mark, mark_t start_mark, mark_t end_mark,
     SHORT direction, USHORT flags);

//enum search_flags { SEARCH_BLOCK=16, QUERY_REPLACE=32, REGEXP_SEARCH=64,
//                    SEARCH_IGNORE_CASE=128, SEARCH_WORDS_ONLY=256 };
#define SEARCH_BLOCK  16
#define QUERY_REPLACE 32
#define REGEXP_SEARCH 64
#define SEARCH_IGNORE_CASE 128
#define SEARCH_WORDS_ONLY 256

bool regexp_search (char *string, SHORT start_col, char **start, char **end, SHORT len, USHORT flags);
bool regexp_compile (char const *string);

extern SHORT found_count, replace_count; /* for use by VIEW_SCH.C */

#ifdef REGEXP
extern char *start_sub[], *end_sub[];  /* from REGEXP.C */
#endif

                             /* view_sch func needed by search.c */
SHORT view_verify_replace (mark_t head, mark_t tail, char const *found_str, 
                      SHORT found_len, char const *replace_str, bool inblock);

