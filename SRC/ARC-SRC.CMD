/*   archives KED source files for a primitive version control */

SAY "This program archives all KEd sources into a .ZIP file"
SAY "which is called a string made of the date with "src" appended."
SAY " "
SAY "You must enter some text to describe this archive ! "
SAY " "
SAY "Press ENTER alone to terminate text entry "
SAY " "
/*
goto byebye
trace all
filename = "comment"
stream (filename, 'C', 'open write')

  DO UNTIL (Comment = "")
    PULL Comment
    SAY Comment
    LINEOUT (filename, Comment);
  END

LINEOUT('comment.txt')
exit

byebye:
*/
rawdate = DATE('N');
SAY "Current date is " rawdate
str1 = WORD(rawdate,1);
str2 = WORD(rawdate,2);
str3 = str2||str1;
spec = str3"src.ZIP"
SAY "Current ZIP name is " spec
SAY "About to archive all KEd source files";
SAY 'Press ENTER to continue or Ctrl-C to ABORT';
pull;
pkzip2 spec '*.c *.h *.cpp';
pkzip2 '-a' spec 'makefile';
pkzip2 '-a' spec 'comment.txt';
exit
