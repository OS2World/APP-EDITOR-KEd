/* KED WPS Installation Code    */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say "This REXX program will create a WPS object on your Desktop"
say "for KEd."
say " "
say "Press Y to continue, or any other key to exit"
parse upper value SysGetKey('NOECHO') with key
say "You pressed " key

if key = 'Y' then do                     
   KEDPATH = DIRECTORY() 
   retcode =  SysCreateObject("WPProgram","KEd_216","<WP_DESKTOP>",,
                   "OBJECTID=<WP_KED216>;"||,
                   "PROGTYPE=WINDOWABLEVIO;"||,
                   "EXENAME="KEDPATH"\KED.EXE;"||,
                   "STARTUPDIR="KEDPATH";")      
end

if retcode = 1 then
    say 'KEd installed on Desktop'
else
    say 'KEd was not installed on the Desktop'

exit


