/* LNED.H */
#ifndef LNED_H
#define LNED_H

#ifndef BASEWIN_H
#error            you must include basewin.h before this header!
#endif

struct ln_dat {
//   char *string;
   byte *string;       // mod 7-27-94
//   vptr w;
   basewin *w;
   SHORT maxlen, curlen, curpos, fld_length, llb, row,col;
   bool ins_mode;
   enum { jleft,jright,jcenter } justmode;
   char fill_char;
   byte cursor_color;
   };

void lned_addkey (struct ln_dat *, char);
void lned_delkey (struct ln_dat *);
void lned_move_cursor (struct ln_dat *, SHORT);
void lned_redisplay (struct ln_dat *);

#endif
