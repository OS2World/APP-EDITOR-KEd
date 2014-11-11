/* REXX   */

Call RxFuncAdd 'SysLoadFuncs','REXXUTIL','SysLoadFuncs';
Call SysLoadFuncs; 

 say "CLEAN.CMD  - deletes files in preparation for KARC" 
 say 'Press any key to continue'

 any = SysGetKey('NOECHO')

 del ked.exe
 del ked.map
 del logfile
 del mem_err
 del edit.err

exit
/*  good_exit */


