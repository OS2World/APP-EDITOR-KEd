/*****************************************************
file: KEYMAC.C      Copyright 1989 by Dlugosz Software
   keyboard macros ability
*****************************************************/
#include "usual.h"
#include "vmem.h"
#include "getkey.h"
#include "redbox.h"
#include "keymac.h"
#include "status.h"
#include <string.h>
#include <stdlib.h>    /* malloc/free for macro string */

#include "test.h"

bool recording = FALSE;
bool play_macro = FALSE;
#define MAX_MAC_BUF 80

//USHORT *macro= NULL;
//static USHORT *macro_string= NULL;
static USHORT record_stop, record_len, record_maxlen;
//static USHORT record_buf[MAX_MAC_BUF];
static key_rec record_buf2[MAX_MAC_BUF];
static void (*record_callback)(SHORT status, SHORT len);
#define err_locus 9

/************************************************************************/
#ifdef FUCKUP
//void key_macro (const char* keys)
void key_macro (USHORT* keys)
{
                        /* installs the string as a type-ahead macro. */
SHORT len, oldlen;

if (!keys) 
   return;                          /* ignore a NULL request */
len = record_len;
DBG_MACRO(dout<<"\n"<<" in key_macro "<<endl);
//if (macro_string) {              //if there is a pending macro, concatenate it
//   oldlen= strlen(macro);
//   strcpy (macro_string, macro);                  /* take up the slack */
//   macro= macro_string= (char *)realloc(macro_string, oldlen+len+1);
//   memcpy (macro_string+oldlen, keys, len);
//   macro_string[oldlen+len]= '\0'; }
//else {                                            /* a fresh macro */
//   macro= macro_string= (char *)malloc(len+1);
//   macro= macro_string= (USHORT *)malloc(len+1);
//   memcpy (macro, keys, len);
//   macro[len]= '\0'; }
//   macro[len]= 0x0000; 
//   }
}
#endif

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

//void key_capture (char* buf, SHORT buflen, SHORT stopkey, void (* callback) (SHORT status, SHORT len))
//void key_capture (USHORT* buf, SHORT buflen, USHORT stopkey, void (* callback) (SHORT status, SHORT len))
void key_capture (USHORT stopkey, void (* callback) (SHORT status, SHORT len))
{
//record_buf= buf;
record_stop= stopkey;
record_len= 0;
record_maxlen=  MAX_MAC_BUF;
record_callback= callback;
recording= TRUE;
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
//USHORT read_from_macro (void)
key_rec read_from_macro (void)
{
static int read_pos = 0;
//USHORT key= record_buf[read_pos];
key_rec tempkey_rec= record_buf2[read_pos];
read_pos++;
DBG_MACRO(dout<<"\n\n"<<" in read_from_macro "<<endl);
DBG_MACRO(dout<<" read_pos = "<<read_pos<<" record_len = "<<record_len<<endl);
DBG_MACRO(dout<<" read_from_macro key = "<<hex<<tempkey_rec.value<<endl);
DBG_MACRO(dout<<" record_buf = ");
     for(int x = 0; x <= record_len; x++) {
         DBG_MACRO(dout<<" "<<hex<<record_buf2[x].value);
     }
     DBG_MACRO(dout<<dec<<endl);

   if(read_pos == record_len ) {
   DBG_MACRO(dout<<"\n\n"<<" in read_from_macro FINISHED "<<endl);
//   free (macro_string);         // KLUDGE - use a bool instead
//   macro= macro_string= NULL;
   play_macro = FALSE;
   read_pos = 0;
   }
return tempkey_rec;
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     the playback mechanism               */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

bool play_key_macro (void)
{
DBG_MACRO(dout<<"\n\n"<<" in play_key_macro "<<endl);
//key_macro (buf);
//key_macro (record_buf);
play_macro = TRUE;
return TRUE;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
/*     the record mechanism                 */
/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

static void done_with_record (SHORT status, SHORT len)
{
/* this is passed to key_capture.  It is called when the macro is done
   being recorded.  */
DBG_MACRO(dout<<"\n\n"<<" Done with Key recording "<<endl);
status_key_recording (FALSE);  /* un-display 'recording' indicator */
if (len == 0) return;
//record_buf[len]= 0x0000;
record_buf2[len].value= 0x0000;
record_buf2[len].state= 0x0000;
recording= FALSE;
#pragma warn -par         // we don't use "status" param 
}
#pragma warn +par        


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */

void key_record (USHORT key)
{            /* start recording a macro and assign to that key */
if (recording) {
   redbox (err_locus, 200, eh_cancel);
   return;
   }
recording= TRUE;
status_key_recording (TRUE);  /* display 'recording' indicator */
DBG_MACRO(dout<<"\n\n"<<" recording Key Macro "<<endl);
DBG_MACRO(dout<<" key record_stop = "<<hex<<key<<dec<<endl);
//key_capture (buf, 80, key, done_with_record);  /* start capturing keystrokes */
key_capture (key, done_with_record);  /* start capturing keystrokes */
}


//void put_key_in_macro_buf(USHORT mykey)
void put_key_in_macro_buf(key_rec mykeyrec)
{
    DBG_MACRO(dout<<"\n"<<" in put_key_in_macro_buf "<<endl);
    DBG_MACRO(dout<<"\n"<<" recording key = "<<hex<<mykeyrec.value<<dec<<endl);
    if (mykeyrec.value == record_stop) {
       DBG_MACRO(dout<<" Key Macro end key "<<endl);
       record_callback (0, record_len);
       recording= FALSE;
       goto func_exit;
    }
    if (record_len < record_maxlen) {            /* record the key */
           record_buf2[record_len++]= mykeyrec;
           DBG_MACRO(dout<<"\n"<<" macro_buf key    now "<<record_len<<" long");
           for(int x = 0; x <= record_len; x++) {
              DBG_MACRO(dout<<" "<<hex<<record_buf2[x].value);
           }
           DBG_MACRO(dout<<dec<<endl);

           DBG_MACRO(dout<<" macro_buf status now "<<record_len<<" long");
           for(int y = 0; y <= record_len; y++) {
              DBG_MACRO(dout<<" "<<hex<<record_buf2[y].state);
           }
           DBG_MACRO(dout<<dec<<endl);
   }
   else {                                     /* no more room */
      record_callback (1, record_len);
      recording= FALSE;
    }
func_exit:
}                                            // end if (recording)

