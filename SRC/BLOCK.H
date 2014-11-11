/* block.h  */

#ifndef BLOCK_H
#define BLOCK_H

SHORT scrap_packup (void*);
SHORT scrap_unpack (void*);

bool my_new_scrap (void);
bool restore_scrap (void);
bool block_read (char const* filename);
bool block_write (char *filename);
SHORT scrap_write (char const* filename);
void scrap_print (void);

bool cursor_in_block (struct mark_rec *cursor, struct mark_rec *begin, struct mark_rec *end, SHORT blockmode);

/* view level */
void mark_block (SHORT corner); /* 0-begin  1-end */
SHORT block_unmark();              
void block_changemode (SHORT mode);
bool block_copy (bool cut);  /* copy or cut a block to scrap */
bool block_paste (bool quote);
void block_zap (void);
void block_adjust (SHORT distance);
void block_print (void);
void block_jump (SHORT which_end);
void block_view (void);

struct scrap_t {
   vptr sh;  /* the sheet */
   bool valid;                      // does scrap have a valid sheet ?
   enum view_rec::block_mode_t mode;
   struct scrap_t *next;
   };

extern struct scrap_t *scrap;

#endif
