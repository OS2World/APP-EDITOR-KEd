
#ifndef EDX_H
#define EDX_H

//void edit_string (window_t w, SHORT row, SHORT col, SHORT width, char* s,
//                          SHORT maxlen, byte *colorset, USHORT flags);
//void edit_string (SHORT row, SHORT col, SHORT width, char* s,
//                          SHORT maxlen, byte *colorset, USHORT flags);
USHORT edit_string (SHORT row, SHORT col, SHORT width, char* s,
                          SHORT maxlen, byte *colorset, USHORT flags);

bool prompt_box (char const* prompt, char *s, SHORT maxlen, char **recall,
                          USHORT flags, SHORT help);
/* flags:                           | prompt_box uses these flags always:
   1: caplock        2: insert mode | 16: display first 32: return terminator
   4: right justify  8: centering   | 256: allow window to move
   64: zap on first  128: numbers only */

bool prompt_int (char const* prompt, SHORT *n, SHORT min, SHORT max, SHORT help);
bool prompt_long (char const* prompt, long *n, long min, long max, SHORT help);

#endif
