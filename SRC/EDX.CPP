/*****************************************************
File: EDX.C         Copyright 1989 by Dlugosz Software
   functions for field editing.  This uses LNED to
   build a useful edit function.
*****************************************************/
#include "usual.h"
#include "getkey.h"
#include "keycodes.h"
                    //#include "lned.h"
#include "edx.h"
#include "setting.h"
#include "misc.h"
#include "help.h"
#include "ascii.h"
#include <stdlib.h>    /* uses atol() and atoi() */
#include <string.h>    /* need strlen() */

#include "basewin.h"
#include "lned.h"

#include "test.h"

#define colors settings.colors[DIALOG_COLORS]
#define pos(n) settings.location[DIALOG_LOCATION].n

static struct ln_dat fld;
static bool firstkey= FALSE;

//basewin prmptw (4, 40, 4, 5, colors[NORMAL_COLOR]);  // change to basewin* ??
basewin *prmptw = NULL; 

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static USHORT getkey_cursor (void)
{
   USHORT key;
if (firstkey && fld.curpos != 0) {             /* display special cursor */
   SHORT col= fld.col+fld.curpos+fld.llb;
   prmptw->put(fld.row, col, "\xae");    // should work
//   prmptw->put(fld.row, col, "TEST");   // test
   DBG_PROMPT(dout<<"getkey_cursor putting "<<endl);
//   fld.w->put(fld.row, col, "\xae");
//   key= window_getkey (&prmptw, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);
//   mykey_rec= window_getkey (&prmptw, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);

   mykey_rec= window_getkey (prmptw, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);
   key = mykey_rec.value;
//   put_string (fld.w, fld.row, col, " ");  /* assume cursor was at the end */
   prmptw->put(fld.row, col, " ");  /* assume cursor was at the end */
//   fld.w->put(fld.row, col, " ");  /* assume cursor was at the end */
   return key;
   }
else {
//   mykey_rec = window_getkey (&prmptw, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);
   mykey_rec = window_getkey (prmptw, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);
   DBG_PROMPT(dout<<"getkey_cursor else "<<endl);
//   mykey_rec = window_getkey (fld.w, fld.row, fld.col+fld.curpos+fld.llb, fld.cursor_color);
   key = mykey_rec.value;
}
   return key;
//   return getkey();
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  zap_string(void)
{
fld.string[0]= ' ';
fld.curlen= 0;
fld.curpos= 0;
lned_addkey(&fld, 0);
lned_redisplay (&fld);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  do_basics (USHORT key)
{
bool processed= TRUE;
switch (key) {
   case kLeft:
      lned_move_cursor (&fld, fld.curpos-1);
      break;
   case kINS:
      fld.ins_mode= !fld.ins_mode;
      break;
   case kDEL:
      if (fld.curlen) lned_delkey(&fld);
      break;
   case kBS:
      if (fld.curlen && fld.curpos) {
      lned_move_cursor (&fld, fld.curpos-1);
      lned_delkey (&fld); }
      break;
   case kRight:
      if (firstkey) break;  /* suppress the key */
      lned_move_cursor (&fld, fld.curpos+1);
      break;
   case kHOME:
      lned_move_cursor (&fld, 0);
      break;
   case kEND:
      lned_move_cursor (&fld, fld.curlen);
      break;
   case kF2:                  // kF+2:  // org
      zap_string();
      break;
#ifndef NO_ASCIIBOX
   case kAltA:             // 286:  /* Alt-A for ASCII box */
      {
      char str[4], *s= str, ch;
      DEF_HELP (ASCII_HELP,1,25,10);
      NEW_HELP;
      ch= (char)(fld.curpos >= fld.curlen) ? (char)0 : fld.string[fld.curpos];
      switch (ascii_chart (ch, str)) {
         case 4: /* character */
            lned_addkey (&fld,*s);
            lned_move_cursor (&fld, fld.curpos+1);
            break;
         case -1:
            break;
         default:
            while (*s) {
               lned_addkey (&fld,*s++);
               lned_move_cursor (&fld, fld.curpos+1);
               }
            break;
         }
      OLD_HELP;
      }
      break;
#endif
   default: processed= FALSE;
   }
return processed;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static bool  normal_key (USHORT key, USHORT flags)
{
   UCHAR mykey = LOUCHAR(key);

   if (flags & 128) {
      if (((mykey >= '0') && (mykey <= '9')) || 
         ((mykey == '-') || (mykey == '+')))
         return TRUE;
      else return FALSE;
   }
//   else if ((mykey >= 32) && (mykey <= 255)) 
   else if (mykey >= 32) 
         return TRUE;
      else return FALSE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//void edit_string (window_t w, SHORT row, SHORT col, SHORT width, char *s, SHORT len, byte *colorset, USHORT flags)
//void edit_string (SHORT row, SHORT col, SHORT width, char *s, SHORT len, byte *colorset, USHORT flags)
USHORT edit_string (SHORT row, SHORT col, SHORT width, char *s, SHORT len, byte *colorset, USHORT flags)
/* flags:
   1: caplock        2: insert mode
   4: right justify  8: centering
  16: display first 32: return terminator
  64: zap on first  128: digits only (and - sign)
  256: allow window to move
  */
{
USHORT key = 0x0000;
bool processed=TRUE;

firstkey= (flags&64);
fld.maxlen= len;
fld.string= s;
fld.curlen= strlen (s);
fld.curpos= fld.curlen;
fld.fld_length= width;
                    //fld.w= w;
fld.row= row;
fld.col= col;
fld.ins_mode= flags&2;
fld.justmode= (flags&4) ? ln_dat::jright: ((flags&8) ? ln_dat::jcenter: ln_dat::jleft);
fld.fill_char= ' ';
fld.cursor_color= colorset[HIGHLIGHT_COLOR];
s[fld.curlen]= fld.fill_char;

lned_addkey (&fld,0);  /* compute llb */
if (flags&16) {
    lned_redisplay (&fld);
}

do {
   key= getkey_cursor ();
   if (!(processed= do_basics(key)))
      if (normal_key (key,flags)) {
//         if ((flags&1) && key >= 'a' && key <= 'z')  //mod 12-6-93 no ucase
//            key &= ~32;
         if (firstkey) zap_string ();
         lned_addkey (&fld, (char)key);
         lned_move_cursor (&fld, fld.curpos+1);
         processed= TRUE; 
      }
      else if (key == 16 && (!(flags & 128))) { /* Ctrl-P */
         key= getkey_cursor ();
         if (key >= 256) {  /* code as 2 keys */
            key &= 0xff;
            lned_addkey (&fld, '\x1b');
            lned_move_cursor (&fld, fld.curpos+1);
            }
         lned_addkey (&fld, (char)key);
         lned_move_cursor (&fld, fld.curpos+1);
         processed= TRUE; 
      }
      else if (key == kESC) {
         processed = FALSE;
      }                             // throw away non-valid keys mod 8-25-93
      else if ((key != Enter) && (key != GREY_Enter)) {
//      else if ((key != Enter)) {
         processed = TRUE;
      }
      firstkey= FALSE;
   } while (processed);
s[fld.curlen]= '\0';
//if (flags & 32)
//   OldKey= key;
//return;
return key;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool prompt_box (char const* prompt, char *s, SHORT maxlen, char **recall, USHORT flags, SHORT help_screen)
{
/* flags: (same as in edit_string)  | prompt_box uses these flags always:
   1: caplock        2: insert mode | 16: display first 32: return terminator
   4: right justify  8: centering   | 256: allow window to move
   64: zap on first 128: only digits */

USHORT key;
SHORT size= strlen (prompt)+2;
const SHORT prompt_width = 60;
//#ifndef NO_HELP
//DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" locus "<<(help.locus)<<" screen = "<<(help.screen)<<endl);
DEF_AUTOHELP (DIALOG_HELP,0);
DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" locus "<<DIALOG_HELP<<" screen = 0"<<endl);

if (help_screen) {
   DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" org help_screen = "<<help_screen<<endl);
//   help.screen= help_screen;   /* warning: assume details of help system */
   help.screen= 1;            // don't know WHY this works mod 8-25-93
   help.y= pos(y)+4; /* set help window y-pos under prompt box */
   help.x= pos(x);
   NEW_HELP;
}
//#endif
size= maxlen+3;
if (size > prompt_width) 
  size= prompt_width;
else if (size < (SHORT)strlen (prompt)+2) 
  size= strlen (prompt)+2;

DBG_PROMPT(dout<<"prompt_box prompt str = "<<prompt<<endl);
prmptw = new basewin(4, prompt_width, 4, 5, colors[NORMAL_COLOR]);  
prmptw->put(1, 1, prompt);
prmptw->border(window_core::Single, colors[BORDER_COLOR]);
prmptw->show();

fld.w = prmptw;

DBG_PROMPT(dout<<"prompt_box passing str = "<<s<<" to edit_string "<<endl);
key = edit_string (2, 1, size-2, s, maxlen, colors, flags|16|32|256);
prmptw->hide();
delete prmptw;
//prmptw->hide();
//#ifndef NO_HELP
if (help_screen) {
   OLD_HELP;
   }
//#endif
//return getkey() != kESC;
//DBG_HYPR1(dout<<__FILE__<<" line "<<__LINE__<<" locus "<<(help->locus)<<" screen = "<<(help->screen)<<endl);
return key != kESC;
#pragma warn -par         // we don't use "**recall" param 
}
#pragma warn +par  

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool prompt_int (const char *prompt, SHORT *n, SHORT min, SHORT max, SHORT help)
{
SHORT value;
char s[7], image[7];

strcpy (s, num2str (image,*n,6,'\0'));
do {
   if (!prompt_box (prompt,s,6,NULL,64|128, help)) return FALSE;
   value= atoi (s);
   } while (value < min || value > max);
*n= value;
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool prompt_long (const char *prompt, long *n, long min, long max, SHORT help)
{
long value;
char image[12], s[12];

//strcpy (s, longnum2str (image,*n,11,'\0'));
strcpy (s, num2str (image,*n,11,'\0'));
do {
   if (!prompt_box (prompt,s,11,NULL,64|128, help)) return FALSE;
   value= atol (s);
   } while (value < min || value > max);
*n= value;
return TRUE;
}
