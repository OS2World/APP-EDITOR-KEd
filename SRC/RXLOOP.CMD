/* REXX procedure that creates a file xx lines long, numbered  */
/* say "Enter number of lines in file" */
/*  pull numlines  */
/* if numlines = "" then */
/*    err_exit     */

DO times=1 TO 50
stream(times, 'c', 'open write')
pull resp
say stream(times, 's', 'query exists')
pull resp
stream(times, 'c', 'close')
pull resp
rem say "this is line number " times "this is more text also"
END

/*  good_exit */


