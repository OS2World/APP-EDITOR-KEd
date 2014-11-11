/*****************************************************
File: LNED.C        Copyright 1989 by Dlugosz Software
   A modular line editor core
*****************************************************/
#include "usual.h"
#include "vmem.h"
//#include "lned.h"
#include "basewin.h"
#include "lned.h"

#include "test.h"

extern basewin *prmptw;          // change this to basewin*  ??

#define MIN(a,b) ((a<b)?(a):(b))
//#define MAX(a,b) ((a<b)?(b):(a))


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  draw_left (struct ln_dat *data)
{
//char *start;        // mod 7-27-94
byte  *start;
SHORT count;

if (data->llb <= 0) { /* beginning of field to cursor */
   start= data->string+(-data->llb);
   count= data->curpos+data->llb +1;
//   put_string_len (data->w, data->row, data->col, start, count);
   prmptw->put(data->row, data->col, start, count);
//   data->w->put(data->row, data->col, start, count);
   }
else {
//   pad_window (data->w, data->row, data->col, data->fill_char, data->llb);
   prmptw->put_fill(data->row, data->col, "", data->llb);
//     data->w->put_fill(data->row, data->col, "", data->llb);
//   put_string_len (data->w, data->row, data->col+data->llb, data->string, data->curpos+1);
   prmptw->put(data->row, data->col+data->llb, data->string, data->curpos+1);
//   data->w->put(data->row, data->col+data->llb, data->string, data->curpos+1);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  draw_one (struct ln_dat *data)
{
SHORT x= data->llb+data->curpos;

if (x>=0 && x<data->fld_length) {
//   put_string_len (data->w, data->row, data->col+x, data->string+data->curpos, 1);
   DBG_PROMPT(dout<<"lned_draw_one "<<endl);
   prmptw->put(data->row, data->col+x, data->string+data->curpos, 1);
 }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  draw_right (struct ln_dat *data)
{
SHORT right,rstop,sstop,x;

x= data->curpos+data->llb+data->col;

rstop= data->col+data->fld_length;
if (x >= rstop) return;  /* nothing to print */
sstop= data->curlen+data->llb+data->col;
right= MIN(sstop,rstop);
//put_string_len (data->w, data->row, x, data->string+data->curpos, right-x);
prmptw->put(data->row, x, data->string+data->curpos, right-x);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void  bumpdown (struct ln_dat *data)
{
SHORT loop;
char *cur;

if (data->curlen < data->maxlen) data->curlen++;
loop= data->curlen-1;  /* points past string */
cur= data->string+loop;
loop -= data->curpos;
while (loop-- > 0) {
   *cur= *(cur-1);
   cur--;
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void lned_addkey (struct ln_dat *data, char key)
{
#define ASSIGN (data->string)[data->curpos]= key

SHORT old_llb= data->llb, old_len= data->curlen;
bool bumped= FALSE;

if (!key) {    /* do checking for cursor position */
   DBG_PROMPT(dout<<"lned_addkey no key"<<endl);
   if (data->curlen < data->fld_length)
   switch (data->justmode) {
      case ln_dat::jcenter: 
              data->llb= (data->fld_length-data->curlen) >>1; break;
      case ln_dat::jright: 
              data->llb= data->fld_length-data->curlen;  break;
      case ln_dat::jleft: 
              data->llb= 0;
      }
   else data->llb= 0;
   lned_move_cursor (data, data->curpos);
   return;
   }
                                      /* the normal use of adding a key */
if (data->curpos < data->curlen) {
   if (data->ins_mode) {
      bumpdown(data);
      bumped= TRUE;
      if (old_len < data->fld_length && 
          data->justmode == ln_dat::jright ||
          (data->justmode == ln_dat::jcenter && 
          (old_len&1) != (data->fld_length&1)))
               data->llb--;
      }
   ASSIGN; }
else {                                 /* add beyond current end of string */
   SHORT gap= data->curpos-data->curlen;
   while (gap--)   /* fill in gap from end of current string to char to insert*/
      (data->string)[data->curlen++]= ' ';
   data->curlen= data->curpos+1;
   ASSIGN;
   if (data->curlen < data->fld_length) {
      switch (data->justmode) { /* if no fit, do like left just. */
         case ln_dat::jcenter: 
                 data->llb= (data->fld_length-data->curlen) >>1; break;
         case ln_dat::jright: 
                 data->llb= data->fld_length-data->curlen;
         }
      } /* end of change llb */
   }
if (old_llb != data->llb)
   if (data->curlen-old_len==1 && old_llb-data->llb==1) 
       draw_left(data);
   else 
       lned_redisplay(data);
else
   if (!bumped) draw_one (data);
   else draw_right (data);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void lned_delkey (struct ln_dat *data)
{
SHORT loop= data->curpos, end= data->curlen-1;
char *cur= data->string + loop;

if (loop > end+1) return;
while (loop++ <= end) {
   cur[0]= cur[1];
   cur++; }
*(--cur)= data->fill_char;               /* *cur= data->fill_char; */
if (end<data->fld_length &&
   (data->justmode == ln_dat::jright ||
   (data->justmode == ln_dat::jcenter && 
          (end&1) != (data->fld_length&1)))) {
   data->llb++;
   draw_left (data);
   }
else draw_right (data);
data->curlen--;
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void lned_move_cursor (struct ln_dat *data, SHORT pos)
{
SHORT x;

if (pos < 0) pos=0;
else if (pos >= data->maxlen) pos= data->maxlen-1; /* range check */
data->curpos= pos;
/* check for scrolling */
x= pos+data->llb;
if (x < 0) {
   data->llb -= x;
   lned_redisplay (data);
   }
else if (x >= data->fld_length) {
   data->llb -= x-data->fld_length+1;
   lned_redisplay (data);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void lned_redisplay (struct ln_dat *data)
{
//char *start;     // mod 7-27-94
byte  *start;
SHORT rstop,sstop,right,tail;

rstop= data->fld_length;
sstop= data->curlen+data->llb;
tail= rstop-sstop;
right= MIN(sstop,rstop);
if (data->llb <= 0) {
   start= data->string+(-data->llb);
//   put_string_len (data->w, data->row, data->col, start, right);
   DBG_PROMPT(dout<<"lned_redisplay  data->llb <= 0 str = °"<<start<<"°"<<endl);
   DBG_PROMPT(dout<<"lned_redisplay  data->llb <= 0 data->string = °"<<(data->string)<<"°"<<endl);
   DBG_PROMPT(dout<<"lned_redisplay  data->llb = "<<(data->llb)<<endl);
   DBG_PROMPT(dout<<"lned_redisplay  prmptw width = "<<(prmptw->get_width())<<endl);
   DBG_PROMPT(dout<<"lned_redisplay  data->w width = "<<(data->w->get_width())<<endl);
   prmptw->put(data->row, data->col, start, right);
//   data->w->put(data->row, data->col, start, right);
//   data->w->put(data->row, data->col, "this is a test", right);
   }
else {
// pad_window (data->w, data->row, data->col, data->fill_char, data->llb);
//   prmptw->pad(data->row, data->col, data->fill_char, data->llb);
   prmptw->put_fill(data->row, data->col, "", data->llb);
   rstop-=data->llb;
// put_string_len (data->w, data->row, data->col+data->llb, data->string, right-data->llb);
   DBG_PROMPT(dout<<"lned_redisplay else  str = °"<<(data->string)<<"°"<<endl);
   prmptw->put_fill(data->row, data->col+data->llb, data->string, right-data->llb);      // MOD 7-28-94
   }
if (tail > 0) {
//   pad_window (data->w, data->row, data->col+data->llb+data->curlen, data->fill_char, tail);
//   prmptw->pad(data->row, data->col+data->llb+data->curlen, data->fill_char, tail);
   prmptw->put_fill(data->row, data->col+data->llb+data->curlen, "", tail);
//   DBG_PROMPT(dout<<"lned_redisplay  tail > 0 str = °"<<(data->string)<<"°"<<endl);
   DBG_PROMPT(dout<<"lned_redisplay  tail > 0 "<<endl);
   }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
