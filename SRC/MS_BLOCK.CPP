/********************************************************************
    MS_BLOCK.CPP

    implements Microsoft Block commands for KEd 2.17
    Commands:
             Mark block
                  Char        = Shift+ Left Arrow, Right Arrow
                  Word        = Shift+ Ctrl + Left Arrow, Right Arrow
                  Line        = Shift + End, Up Arrow, Down Arrow
                  Screen Up   = Shift + PageUp
                  Screen Down = Shift + PageDn
                  Beg File    = Shift + Ctrl + Home
                  End File    = Shift + Ctrl + End

             Copy block  = Ctrl + Ins
             Cut block   = Shift + Del
             Paste block = Shift + Ins

             When block is marked, any key other than Block Copy or Cut
             UnMarks block
             
             Keycode defines in KEYDEF.CPP
                                          
    v.1.0 8/20/95
             When GetKey parser finds that Shift + Movement key is pressed,
             it sets a flag for MS_Block Start and
             passes key here for input into State Machine.
             If GetKey parser then finds a non-block key, it passes it
             here to unmark the Block.
************************************************************************/
#include "usual.h"
#include "getkey.h"
#include "vmem.h"            // sheet needs this
#include "sheet.h"           // view.h needs this
#include "view.h"           // block.h needs this
#include "block.h"          // block_unmark
#include "ed_cmd.h"

#include "test.h"

SHORT ms_block(USHORT key)
{
   enum MSB { BLOCK_NULL, BLOCK_START, BLOCK_CONTINUE, BLOCK_END};
   static int current_state = BLOCK_NULL;
   UCHAR lokey = LOUCHAR(key);

   DBG_MS_BLOCK1(dout<<" ms_block passed key  = 0x"<<hex<<key<<dec<<endl);
   DBG_MS_BLOCK1(dout<<"   lokey  = 0x"<<hex<<lokey<<dec<<endl);

   if((current_state != BLOCK_NULL) && (lokey != 0xE0)) {
//   if((current_state != BLOCK_NULL) ) {
      DBG_MS_BLOCK1(dout<<" ms_block unmark"<<endl);
      block_unmark();
      current_state = BLOCK_NULL;
   }
   else if(current_state != BLOCK_START) {
      DBG_MS_BLOCK1(dout<<" ms_block start"<<endl);
      block_start();
      current_state = BLOCK_START;
   }


   switch(key) {
         case 0x48E0:
                     go_up();
                     break;
         case 0x49E0:
                     go_pgup();
                     break;
         case 0x4BE0:
                     go_left();
                     break;
         case 0x4DE0:
                     go_right();
                     break;
         case 0x50E0:
                     go_down();
                     break;
         case 0x51E0: 
                     go_pgdown();
                     break;

      //   default
   }
//   if(current_state == BLOCK_END)
      block_end();

return 0;
}
