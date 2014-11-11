//extern window_t status_window;
   /* the status window has special privileges in the viewer system.  It is
      always brought in front of any viewer window, so it is never covered.
      You can do the same by doing a display_on() of this window after
      doing a display_on() of some other window that may have covered it.
      Notice that the window lib has been modified so display_on() checks
      for a NULL argument and does nothing, so this window can be NULL if
      it is not needed.  */

void status_setup(void);
   /* this is called once at the beginning of the program, before any
      view is created. */
void status_redraw(void);
   /* this is called every time the active viewer changes */

void status_quick_redraw(ULONG row, SHORT col, bool changed); // ked 2.10 

void status_colorchange(void);
   /* the color settings of the status window or the
      active viewer changed */

/* these functions are called to update individual status elements */
void status_name(void);
   /* called when the name of a file changes, as with a Save */
void status_row (ULONG row);
void status_col (SHORT col);
void status_delta (bool b); /* file changed mark */
void status_block (SHORT mode_switch);   /* -1 off, or block mode */
void status_key_recording (bool onoff);
void status_insertmode (bool insert_on);
void status_numlockmode (void);
//void status_insertmode (bool insert_on);
void status_indentmode (bool indent_on);
void status_message (char const* );
void status_note (char const* );  /* send NULL to turn off */

#ifdef PACKUP
SHORT status_packup (void *);
SHORT status_unpack (void *);
#endif
