/* keymac.h
   functions for keyboard macros
*/

bool play_key_macro (void);
void key_record (USHORT key);

SHORT keymac_packup (void *buf);
SHORT keymac_unpack (void *buf);
