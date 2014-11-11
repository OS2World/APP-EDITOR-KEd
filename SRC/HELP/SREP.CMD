/*       REXX CMD file to search and replace strings in *.* files */

call RxFuncAdd  'SysLoadFuncs', 'RexxUtil','SysLoadFuncs'
call SysLoadFuncs

rem   setup global vars
filespec  = '*.*'
searchstr = ""
repstr    = ""

call syntax
call Prompt_for_Vars

call SysFileTree filespec, files, 'FSO'
do i=1 to files.0
  say files.i
  if searchstr \= '' then do
/*     call SysFileSearch searchstr, files.i, 'found', N */
     call searchfile(files.i)
/*     do n=1 to found.0  */
/*        say 'Line ' found.n  */
/*     end  */
     say ' '
  end
end

exit

searchfile: arg file
/* trace result   */
lineno = 1
   linestr = linein(file)
 say 'searching file ' file ' for ' searchstr
do until lines(file) = 0
   y = pos(searchstr, linestr)
   if y \= 0 then do
      say ' found ' searchstr 'in line ' lineno linestr
   end
   lineno = lineno + 1
   linestr = linein(file)
end
return


Prompt_for_Vars : procedure expose filespec searchstr repstr
say ' Search what files ? (default is *.*) '
call SysCurPos 5, 38
pull filespec
if filespec = "" then 
   filespec = '*.*'
say ' Search for what string ? (default is "") '
call SysCurPos 6, 43
parse pull searchstr
say ' Replace with string ? (default is "") '
call SysCurPos 7, 43
parse pull repstr
return

syntax : procedure

call syscls
say ' '
say ' This cmd file will search and optionally replace all strings in '
say ' files in the current directory '
say ' '
say ' Press any key to continue '
call SysGetKey('NOECHO')
return
