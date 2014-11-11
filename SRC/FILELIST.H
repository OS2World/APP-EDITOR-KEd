#ifndef FILELIST_H
#define FILELIST_H

// define USE_FILELIST     // uncomment to use filelists
#ifdef USE_FILELIST
extern vptr filelist_view;
#endif
void filelist_set (vptr view);
char *filelist_next (void);
char *filelist_prev (void);
char *filelist_fetch (void);
void filelist_top (void);
void filelist_bottom (void);
bool filelist_exists (bool create);
#endif
