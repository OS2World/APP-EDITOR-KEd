/*       REXX CMD file to search and replace strings in *.* files */

call RxFuncAdd  'SysLoadFuncs', 'RexxUtil','SysLoadFuncs'
call SysLoadFuncs

rem   setup global vars
in_name  = ""
out_name = ""

call syntax
call Prompt_for_Vars

call SysFileTree in_name, files, 'FSO'
do i=1 to files.0
  say "Processing  Input " files.i
     call get_input_line(files.i)
     say ' '
end

exit


/************************************************************************/
get_input_line: arg file
/* trace result   */
lineno = 1
   linestr = linein(file)
 say 'processing file ' file ' line ' lineno
do until lines(file) = 0
   call process_line linestr
   lineno = lineno + 1
   linestr = linein(file)
end
return

/************************************************************************/
process_line: parse arg in_str
/* trace result   */
comment = '$$'
header  = '$'
titlestr = 'TITLE'
outstr = ""

   y = pos(comment, in_str)        /* find comment lines '$$' */
   if y = 0 then do                /* didn't find comment, so maybe output */
      yy = pos(header, in_str)     /* find header lines '$' */
      if yy \= 0 then do
         zz = pos(titlestr, in_str)     /* strip and output section title */
         if zz \= 0 then 
            call output_title in_str    /* no $ found */
      end         
      if yy = 0 then do
          outstr = strip_bold(in_str)
          outstr1 = strip_index(outstr)
          outstr2 = strip_index(outstr1)   /* some lines have 2 indexs */
          outstr3 = strip_see(outstr2)     /* strip "see file menu.." lines */
          say outstr3
      end
   end
return 

/*******************************************************************/
output_title : parse arg in_str
   parse arg '$' 'TITLE' rest
   out_title1 = strip(rest,,)
   out_title2 = strip(out_title1,'B','"')
   say ' '
   say ' '
   say ' '
   say '             ' out_title2
   say ' '
return 

/*******************************************************************/
strip_bold : parse arg in_str
              /*                        strip \b, \n,*/
bold  = '\b'
non   = '\n'
str1 = ""
str2 = ""

   g = pos(bold, in_str)
   if g > 0 then do
      str1 = delstr(in_str, g, 2)
      gg = pos(non, str1)
      if gg > 0 then do
         str2 = delstr(str1, gg, 2)
         return str2               
      end
   end
   return in_str
return 

/*******************************************************************/
strip_index : parse arg in_str
              /*     strip 1st part  [EDITOR_FCTNKEYS|Function Keys] */

local_str = in_str
sep   = '|'
index = '['
endindex = ']'
str1 = ""
str2 = ""
/* more_index = TRUE
/*   trace result    */
do while more_index = TRUE
*/
   g = pos(index, local_str)
   if g > 0 then do
      gg = pos(sep, local_str)
      if gg > 0 then do
         str1 = delstr(local_str, g, (gg - g)+1)
         ggg = pos(endindex, str1)
         if ggg > 0 then do
            str2 = delstr(str1, ggg, 1)
            local_str = str2 
       /*     gag = pos(index, local_str)
            if gag = 0 then do
               more_index = FALSE
               leave 
            end   */
/*            return str2 */
         end
      end
   end
/*end */
   return local_str
return 

/*******************************************************************/
strip_see : parse arg in_str
                                /*   strip  "see file menu.." lines*/
   g = pos('See', in_str)
   if g = 0 then do
      return in_str
   end
   if g > 0 then do
      return ''
   end
return 

/***********************************************************************/
Prompt_for_Vars : procedure expose in_name out_name
say ' Input file ? (default is EDITOR.TMP) '
call SysCurPos 5, 38
/* pull in_name */
if in_name = "" then 
   in_name = 'EDITOR.TMP'
say ' Output Filename ? (default is EDITOR.001) '
call SysCurPos 6, 45
/* parse pull out_name */
if out_name = "" then 
   out_name = 'EDITOR.TMP'
return

/***********************************************************************/
syntax : procedure
call syscls
say ' '
say ' This cmd file will strip all formatting commands from EDITOR.CSH '
say ' and output a TXT file for KEd documentation '
say ' '
say ' Press any key to continue '
/* call SysGetKey('NOECHO') */
return
