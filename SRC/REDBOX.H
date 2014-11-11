#ifndef REDBOX_H
#define REDBOX_H

enum err_choices {
   eh_continue=1,
   eh_abort=2,
   eh_retry=4,
   eh_cancel=8,
   eh_yes=16,
   eh_no=32
   };

enum err_choices redbox (SHORT locus, SHORT num, enum err_choices choices);
enum err_choices redbox_s (SHORT locus, SHORT num, enum err_choices choices, char const* s);
   /* the _s function will display the string before the message.  It is
      typically the name of the offending file, and is clipped on the left
      if it does not fit. */

                            /* OS/2 only   */
enum err_choices os2_redbox_s (SHORT locus, SHORT num, SHORT errnum, enum err_choices choices, char const* s);

#endif