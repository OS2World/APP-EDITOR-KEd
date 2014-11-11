bool get_load_name (char *default_name, char *fname, char const *default_dir, char const *default_ext);
   /* default_name on input is name to start out with, on output is
         what was typed.  It may be NULL for no default.
      fname on output is the actual normalized filename that was picked.
      default_dir may be NULL, in which case fname is not normalized and
         default_ext is ignored as well.
   */

bool get_save_name (char *fname, char const* default_dir, char const* default_ext);
   /* default_dir may be NULL, in which case the name is not normalized
         and default_ext is ignored as well.
   */
