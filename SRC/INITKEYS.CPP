/*  INITKEYS.C                                  */
/*  initializes key table (so we can reassign)
    loads key reassign file ( KED.KEY)
    parses valid key assign strings ( parse_key_line)
    reassigns keys

    USES     KEYDEF.C 

    HIGH LEVEL DESIGN 9-1-93
    open key.dat file
    get a line
    any data? ie:non-comments
    get left token 
    get right token
    perform assign


    MID LEVEL DESIGN 9-1-93
    open key.dat file            needs to be in EXE directory
    get a line                   fgets should work
    any data? ie:non-comments
        # is comment char    if not in position 1, should be some valid data
    get left token           get token left of '=' 
        strip out whitespace
    get right token          get token right of '='
        strip out whitespace
    perform assign
        some keys can't be assigned (ie: Enter, Esc, F1)
           so issue an error message
           choice is to cancel processing, or ignore that line and continue
        assign key


    NOTES:
        I want the table to look something like this:
        F1 = Help
        F6 = BlockStart
        F7 = BlockEnd

    FUTURES:
        If I do it right, it should be easy to extend this to 
        multi-key macros.

    rev 1.0  march 21,1993
       can't figure out how to create global key_table that represents
       both character strings and USHORTs

    rev 1.1 Sept 13, 1993
       created array of structures to hold KeyName string and KeyValue int
       should work (struct in keydef.c )
    
    rev 1.2 Sept 16, 1993
       fixed sheet load problem (by using scrap buffer)
       need to integrate keydef struct with this code

    rev 2.0 Mar 15, 1994
       scrap sheet load? faster with fread() ? nah, do it later if
         key loading is too slow
       use function pointers in structure
       after we parse the tokens, then look up the keyname ie: F3 and
       return an int that is array index into the key_name_val struct.
       then set the function pointer there to the token we just
       read.

    rev 2.1 Mar 16, 1994
       can read the internal key_name_val struct and call one of the
       functions via a funcptr. I should not remap every key in the
       editor, only the keys in edit_loop. I can look up the key val in
       USHORT value, but what then? 
       rev 1 
            use key_name_val USHORT values to assign fptr to an internal
            name in edit_loop ie: kAltV = show_version()
       rev 2 
            eliminate edit_loop case: block and directly use the fptrs

   rev 2.2 Mar 18, 1994
      need to assign key value in edit_loop. I can't just look up something
      in the returned ushort from getkey() because the values aren't
      guaranteed to be linear. So I must create a bunch of shared
      USHORTS and lookup the values in my key_struct[] array.

   rev 2.3 Mar 23, 1994
      ok, got the basic functions working. need to make some more functions
      return and take void, because that's the only fptr I've got.
      if key > 0x1000 in edit_loop I try to call the function. 

   rev 2.4 March 27, 1994
      I can lookup and validate the KeyName in the KeyDef table, but
      I can't get the fptr from a string. ie: printmenu  
      This means one thing to the compiler (a function entry point)
      and quite another when reading a string from KED.KEY
      I can write the address of the function into the KeyDef table
      after compile by looking it up the in the MAP file, and then I
      can lookup the function string in the KeyDef table and it'll
      give me the function address. 

      This will probably be too slow to load "on the fly", so I'll
      need a standalone configuration program, which is what I
      wanted to do anyways.

      Now I need functions that will lookup the address of a function
      from KED.KEY and punch in the new address for that Fkey

   rev 2.5 March 31, 1994
      After a study of how MicroEmacs does this, I think I can 
      leave everything in one table. I WILL need a function to 
      return a fptr based on "funcname". 

   rev 2.6 April 11, 1994
      This table will have 2 uses:
      1.  READ at LOAD or CONFIG time to resolve user-defined
          key mappings to function entry points
      2.  READ at EXECUTE time to execute a function based on
          keystroke USHORT value.

      If a user remaps a key, I will need to replace both the
      "funcname" and fptr* with user's choice. Will this cause
      a problem with strings in the structure???

      MEmacs gets around this by having 2 separate tables for
      "funcname" <-> fptr     and
      "keyname"  <-> keyvalue
   
   rev 3.0  June 16, 1994
      created an external setup program with the idea of isolating
      all user configurable stuff there, and write the output to the
      EXE file. Found that the strings were stored in the DATA segment
      (good because I can find this in the EXE) and the function pointers 
      were stored in the BSS segment (BAD because I can't find them in
      the EXE). 
      Current strategy is to have the setup program parse the keyboard
      configuration raw file (check syntax) and then to have the EXE
      read it on startup, then the user can save the compiled key defs
      into the setting file. we'll see.....
     
     The KeyNameVal struct looks like this:
     {"F5"    ,  block_start        ,"block_start"  ,0x3F00 }, 
     {"F6"    ,  block_end          ,"block_end"    ,0x4000 },  
                         
      The default KED_KEY.DAT file will look like this:
        F6 = BlockStart
        F7 = BlockEnd

      So when a user chooses something like this:
        F6 = file_menu
        F7 = file_save

     First I need to see if "F6" is a valid keyname, and store the offset 
     for that entry in KeyNameVal, then I need to look up
     "file_menu" and see if that is a valid func ptr name, and store that
     func ptr and "func name".
     Then I would replace the func ptr and "func_name" at the offset for
     "F6" with the new values.

     NEED FIXED LENGTH STRINGS IN THE TABLE?   yes....


*/
/***************************************************************
load_keys()                                 
  |   top level public function.
  static bool load_key_file (char const* filename);
  |   loads KED.KEY file
  static bool parse_key_lines(const char *s);
    |   separates line into lvalue and rvalue
    static USHORT find_comment(const char *s);
    |    returns ushort pos of comment char, or end of string
    static SHORT lookup_keyname(const char *s);
    |    returns index of keyname ("F2") in KeyNameVal, or -1 if not found
    static fptr get_func_ptr(const char *s);        
         returns func ptr for funcname ("file_save"), or NULL if not found

    static SHORT lookup_funcname(const char *s)
    returns index of funcname ("file_save") in KeyNameVal, or -1 if not found

        Hex keycodes and associated structs are in KEYDEF.C

*****************************************************************/
#include "usual.h"
#include "keycodes.h"
#include "vmem.h"
#include "sheet.h"
#include "window.h"
#include "view.h"
#include "initkeys.h"
#include "misc.h"        // beep_ok
#include "block.h"       // need scrap funcs for key.dat load
#include <string.h>
#include <stdio.h>
#include <mem.h>         // memset

#include "test.h"

#define TESTKEYS

static bool parse_key_lines(const char *s);
static bool load_key_file (char const* filename);
static USHORT find_comment(const char *s);
static SHORT lookup_keyname(const char *s);
static SHORT lookup_funcname(const char *s);
static fptr get_func_ptr(const char *s);
static void replace_keydef(SHORT index, fptr new_func, const char* new_funcname);

/************************************************************************/
bool load_keys()
{
char *keyfile = "C:\\DLUGED\\ED-1292\\ked208\\KED.KEY";
bool ok= FALSE;
struct sheet *sh;
struct line_header *line_hdr;
char *string;

//  for(int i = 0; KeyNameVal[i].num != 0x0000; i++) {
//    DBG_KEY(dout<<"intern key_struct num "<<i<<"  = "<<KeyNameVal[i].name<<
//    "  fptr = "<<&(KeyNameVal[i].func)<<"  val = "<<KeyNameVal[i].num<<endl);
//  }

  ok = load_key_file(keyfile);
//  assert(ok);      // just for testing
  if(ok) {
     sh = (struct sheet *)Mem::vmem_trans(scrap->sh);
     for(SHORT n =0; n < sh->line_count-1; n++) {
        vptr line = get_line_anysheet(sh, n);
        line_hdr = (struct line_header *)Mem::vmem_trans(line);
        string = (char *)(line_hdr+1);     // finally we've got the string
        string[(line_hdr->length +1)] = '\0';
        DBG_KEY1(dout<<"\n"<<endl);
        DBG_KEY1(dout<<"line "<<n<<" = °"<<string<<"° length = "<<strlen(string)<<endl);
        parse_key_lines(string);
     }
  } 
//  DBG_KEY1(dout<<"\n"<<endl);
  restore_scrap ();
exit:
return ok;
}
/**********************************************************************/
bool load_key_file (char const* filename)
{
bool b;

my_new_scrap ();
b= sheet_load (scrap->sh, filename, FALSE);
return b;
}


void replace_keydef(SHORT index, fptr new_func, const char* new_funcname)
/********************************************************
   replace existing func ptr and funcname for a KeyNameVal record.
********************************************************/
{
   strcpy(KeyNameVal[index].funcname, new_funcname);
   KeyNameVal[index].func = new_func;
}


/********************************************************
  finds a string ( ie: "file_menu" or "file_quit") in the KeyNameVal array
  and returns SHORT index in the array, or -1 if not
  found
********************************************************/
static SHORT lookup_funcname(const char *s)
{
  bool result = FALSE;

  for(int i = 0; KeyNameVal[i].num != 0x0000; i++) {
     if(!stricmp(s, KeyNameVal[i].funcname)) {
        result = TRUE;
        break;
     }
  }
  if(result == TRUE)
     return i;
  else 
     return -1;
}

/********************************************************
  finds a string ( ie: F2 or AltS ) in the KeyNameVal array
  and returns SHORT index in the array, or -1 if not
  found
********************************************************/
static SHORT lookup_keyname(const char *s)
{
  bool result = FALSE;

  for(int i = 0; KeyNameVal[i].num != 0x0000; i++) {
     if(!stricmp(s, KeyNameVal[i].name)) {
        result = TRUE;
        break;
     }
  }
  if(result == TRUE)
     return i;
  else 
     return -1;
}

/*************************************************************/
//   finds start of comment ( '#' ) and returns scalar of it's position
//   in the string or strlen(s) if not found
static USHORT find_comment(const char *s)
{
USHORT pos = 0;

while((*s != '#') && (*s != '\0')) {  
   *(s++);
   pos++;
  }
if(pos != 0) {               // trailing comment
  while(*(--s) == ' ') {
     --pos;
  }
}
return pos;
}

/*********************************************************************
    breakdown a line into 3 tokens ie:
    fkey = "AltF" , equals = "=",  function = "show_version"
    only 3 tokens are possible, any more are ignored
    Once we have the tokens, vaildate keyname ("F2") and funcname
    ("file_save") and go ahead and replace the original key definition
*******************************************************************/
static bool parse_key_lines(const char *s)
{
bool ok = FALSE;
char scan[80];
char *str = scan;
char *key;
char *equals;
char *funcname;
char seps[] = " ";            // valid separators
fptr func_ptr = NULL;
USHORT pos = find_comment(s);
memset(scan, '\0', 79);

if(pos != 0) {
   strncpy(scan, s, pos+1);
//   DBG_KEY1(dout<<"\n"<<endl);
   DBG_KEY1(dout<<"keyline str °"<<str<<"° strlen = "<<strlen(str)<<" pos = "<<pos<<endl);
}
                // now we have valid keycode assign string w/no comments
if(pos != 0) {                 // get the tokens
   key = strtok(scan, seps);
   equals = strtok(NULL, seps);
   funcname = strtok(NULL, seps);

   DBG_KEY2(dout<<"TOKENS: key = °"<<key<<"°       funcname = °"<<funcname<<"°"<<endl);
   if(!strcmp(equals, "=")) {
      SHORT i = lookup_keyname(key);
      if(i != -1) {                // valid keyname
         DBG_KEY2(dout<<"intern key_struct num "<<i<<"  = "<<KeyNameVal[i].name<<
                 hex<<"   val = "<< KeyNameVal[i].num<<dec<<endl);
         func_ptr = get_func_ptr(funcname);
      }
      if(func_ptr == NULL) {
         DBG_KEY2(dout<<"invalid funcname  "<<funcname<<endl);
      }
      else {                                // valid funcname
         DBG_KEY2(dout<<"found function ptr via funcname "<<funcname<<endl);
         DBG_KEY2(dout<<"original funcname ="<<KeyNameVal[i].funcname<<endl);
         replace_keydef(i, func_ptr, funcname);
         DBG_KEY2(dout<<"replaced with funcname "<<funcname<<endl);
      }      
   }   // end if "="
}      // end if pos == 0

return ok;
}

/***********************************************************************
        lookup address of function name ie: "file_menu" in the KeyNameVal 
        table and return that function ptr if found. If not found, 
        return NULL
*************************************************************************/
fptr get_func_ptr(const char *s)
{
  int i = 0;

  DBG_KEY(dout<<" in get_func_ptr passed string = °"<<s<<"°"<<endl);

//  while((stricmp(s, KeyNameVal[i].funcname) != 0) )) {
  while(i < 147) {                                // KLUDGE !!!!
    if(stricmp(s, KeyNameVal[i].funcname) == 0) {
      DBG_KEY2(dout<<" get_func_ptr found match in table "<<endl);
      break;
    }
    else {
//      DBG_KEY(dout<<" get_func_ptr strcmp index = "<<i<<endl);
      i++;
    }
  }

  DBG_KEY1(dout<<" get_func_ptr looked at "<<i<<" array elements"<<endl);

  if(KeyNameVal[i].func != NULL) {
  DBG_KEY2(dout<<"in get_func_ptr num "<<i<<" = "<<KeyNameVal[i].name<<
      " fptr2 = "<<KeyNameVal[i].func<<"  val = "<<hex<< KeyNameVal[i].num<<dec<<endl);
//  DBG_KEY2(dout<<"\n"<<endl);
//  DBG_KEY2(dout<<"address in get_func_ptr func = "<< KeyNameVal[i].func<<endl);
  return KeyNameVal[i].func;
  }
  else
     return NULL;        // didn't find it
}
/**********************************************************************/
