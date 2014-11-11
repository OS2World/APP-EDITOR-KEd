MAKE Version 3.7  Copyright (c) 1987, 1994 Borland International
	g:\bcos2\bin\bcc -a1 -P -xp- -xf- -xd- -sm -c -RT- -w -w-sig -w-ucp -Ig:\bcos2\include  -v -vi- -Od  -nos2_objs advfile.cpp ascii.cpp askfile.cpp assert.cpp block.cpp bwind.cpp dirlist.cpp dirname.cpp dirsort.cpp dmm.cpp edit.cpp ed_cmd.cpp 
Borland C++ Version 4.1 Copyright (c) 1993, 1994 Borland International
advfile.cpp:
ascii.cpp:
askfile.cpp:
Warning askfile.cpp 39: Initializing dirlist_flags with int in function get_load_name(char *,char *,const char *,const char *)
assert.cpp:
block.cpp:
bwind.cpp:
dirlist.cpp:
dirname.cpp:
dirsort.cpp:
dmm.cpp:
edit.cpp:
ed_cmd.cpp:
	g:\bcos2\bin\bcc -a1 -P -xp- -xf- -xd- -sm -c -RT- -w -w-sig -w-ucp -Ig:\bcos2\include  -v -vi- -Od  -nos2_objs edset.cpp edx.cpp err_hand.cpp getkey.cpp hyprhelp.cpp keydef.cpp keymac.cpp lned.cpp mark.cpp menu.cpp misc.cpp mylib.cpp picklist.cpp 
Borland C++ Version 4.1 Copyright (c) 1993, 1994 Borland International
edset.cpp:
edx.cpp:
err_hand.cpp:
Warning err_hand.cpp 126: Parameter 'pReserved' is never used in function __syscall MyExceptionHandler(_EXCEPTIONREPORTRECORD *,_EXCEPTIONREGISTRATIONRECORD *,_CONTEXT *,void *)
getkey.cpp:
Warning getkey.cpp 369: Condition is always true in function do_command(key_rec &)
hyprhelp.cpp:
keydef.cpp:
keymac.cpp:
lned.cpp:
mark.cpp:
menu.cpp:
misc.cpp:
mylib.cpp:
picklist.cpp:
	g:\bcos2\bin\bcc -a1 -P -xp- -xf- -xd- -sm -c -RT- -w -w-sig -w-ucp -Ig:\bcos2\include  -v -vi- -Od  -nos2_objs redbox.cpp redraw.cpp search.cpp setkey.cpp setting.cpp sheet.cpp shell.cpp status.cpp version.cpp viewer.cpp view_com.cpp view_drw.cpp 
Borland C++ Version 4.1 Copyright (c) 1993, 1994 Borland International
redbox.cpp:
redraw.cpp:
Warning redraw.cpp 187: Possible use of 'cursheetline' before definition in function view_redraw(unsigned long,long,short,short,short)
search.cpp:
setkey.cpp:
setting.cpp:
Warning setting.cpp 176: 'ok' is assigned a value that is never used in function setting_load(char *)
sheet.cpp:
shell.cpp:
status.cpp:
version.cpp:
viewer.cpp:
view_com.cpp:
view_drw.cpp:
	g:\bcos2\bin\bcc -a1 -P -xp- -xf- -xd- -sm -c -RT- -w -w-sig -w-ucp -Ig:\bcos2\include  -v -vi- -Od  -nos2_objs view_ext.cpp view_fil.cpp view_pic.cpp view_sch.cpp view_win.cpp vmem.cpp wdisp.cpp mymenu.cpp xfile.cpp tbasewin.cpp twcore.cpp 
Borland C++ Version 4.1 Copyright (c) 1993, 1994 Borland International
view_ext.cpp:
view_fil.cpp:
Warning view_fil.cpp 306: Function should return a value in function view_save_file_as(const char *)
Warning view_fil.cpp 306: Parameter 'default_dir' is never used in function view_save_file_as(const char *)
view_pic.cpp:
view_sch.cpp:
view_win.cpp:
Warning view_win.cpp 98: Initializing tudlr with udlr in function view_move_window(udlr,short)
vmem.cpp:
wdisp.cpp:
mymenu.cpp:
xfile.cpp:
Warning xfile.cpp 430: 'printing' is assigned a value that is never used in function sheet_save(unsigned long,const char *)
tbasewin.cpp:
twcore.cpp:
	g:\bcos2\bin\bcc -a1 -P -xp- -xf- -xd- -sm -c -RT- -w -w-sig -w-ucp -Ig:\bcos2\include  -v -vi- -Od  -nos2_objs twsetup.cpp twdisp.cpp twline.cpp twmove.cpp tmisc.cpp tow_os2.cpp tblitt.cpp 
Borland C++ Version 4.1 Copyright (c) 1993, 1994 Borland International
twsetup.cpp:
twdisp.cpp:
twline.cpp:
twmove.cpp:
tmisc.cpp:
tow_os2.cpp:
tblitt.cpp:
	g:\bcos2\bin\tlink /v /Toe /s /m /ap /c /Oc /A:5 /B:0x10000 /w! /S:F000 @MAKE0000.@@@
Turbo Link  Version 1.5 for OS/2 Copyright (c) 1994 Borland International
	rc ked.res ked.exe 
Operating System/2  Resource Compiler
Version 2.01.000 Feb 09 1993
(C) Copyright IBM Corporation 1988-1993
(C) Copyright Microsoft Corp. 1985-1993
All rights reserved.


Reading binary resource file ked.res

..
Writing resources to OS/2 v2.0 Linear .EXE file
Writing 1 DEMAND resource object(s)
  Writing:  996 bytes in 1 page(s)
    1.1 (996 bytes)
Writing Extended Attributes:  Default Icon
Writing Extended Attributes:  Checksum

