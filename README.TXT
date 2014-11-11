
                            KEd   v2.07
                        OS/2 character mode editor

                     (C) Copyright 1993,4 Kudzu Software

                          Don Radick
                          Kudzu Software
                          1452 North Ave. NE
                          Atlanta, GA 30307
							
						  dradick <at> comcast <dot> net


UPDATE: 2014
KEd is now released with source under a simplified BSD license.

If I remember correctly, the sources would compile no warnings using Borland C++
OS/2 compiler and the IBM C++ compiler.  

RE: Support?
It's been a long time, and I'm not sure my understanding would be any better
than any good C programmer!

Don


Welcome to the fifth release of KEd!  User support has been growing, and
I have many great enhancements coming for future releases.
First a few legal things (stay with me, it gets better):

KEd is a character-mode editor. Why, you might ask, yet another text editor
for OS/2?  I first became disenchanted with editors while using
the Microsoft C 6.0 PWB editor for OS/2 v 1.x several years ago, and no editor 
really suited me. I wanted an editor with the ease of use of Qedit or
the Norton Editor, but I wanted to be able to customize it.  By the way,
KEd is produced with Borland C++ for OS/2. 

I think I've come up with some fairly natural key assignments.  The next 
major version of KEd will allow keyboard remapping. 
For now, KEd loosely emulates SideKick.


What does KEd do?

KEd is a general purpose text editor; you won't find a GUI or fancy font 
support, but you will find a lot of speed.  If you need to "punch out" a 
lot of text or need a fast, inexpensive programmer's editor I think you'll 
like it. KEd was designed for OS/2, and uses such OS/2 features as
multiple threads and HPFS support. I am dissatisfied with the current
implemention of the window manager, and do not anticipate adding 
mouse support anytime soon.  KEd will run fine in an OS/2 windowed
command session, but it's quite a bit slower. 

So what can you do with KEd? 

Hypertext online help

You will find a wealth of information here. Instead of writing a lengthy
manual, you can find an answer to almost any question while on-line. 
As the status line shows, press F1 to get to help.  KED.HLP must be in 
your current directory, your PATH, or any directory in your HELP path.  
I suggest putting KED.EXE and KED.HLP in your \UTIL subdirectory if that 
is in the PATH. Some users prefer to put KED.HLP in their drive:\OS2\HELP
subdirectory, and it works fine from there.


Search and Replace

KEd has powerful search and replace facilities. You can toggle case 
sensitivity (IE: find "House" rather than "house"), search backwards,
find only whole words, and choose to confirm each replace if you wish.


Cut and Paste by lines or columns

If you just need to block copy and paste lines, press F5 to mark the
beginning of the block and press F6 to mark the end of the block.
Press F7 to get the block menu, and then "C" (no quotes) to Copy the
block to the buffer. Once you have some text in the buffer, you can
press F7, then "P" to Paste the block. Detailed help is available
on-line from the "Function Key" screens.
You can mark blocks of text by lines, stream, or column. Once you have
marked a block you can cut, view, copy, paste, or write it to a new file.


Load multiple files

You can load multiple files from the command line, up to 34, the limit
of the Borland startup code. (6 files are used by OS/2)


HPFS (long filenames) support

Since this is an OS/2 product, long filenames are supported. The OS/2 
standard is to enclose a name with spaces with quotes. IE:
KED "This is a long file"
will load the file.


Settings save

You can change the color sets that various elements have, including 5 separate
sets of colors for the edit window, the borders, dialog boxes, warning
boxes, help and menus.  You can change a number of options for the editor
including Insert/Overstrike, write End_Of_File, safe save, 2 different forms
of PageUP/PageDown, and initial Numlock setting.  The options will be saved 
in KED.INI, which must be in your current directory or PATH. 



Ascii Chart

Press ALT-A, and the current character under the cursor will be highlighted
in a table of all the ASCII characters. You can move to a non-printable
character or a PC ASCII graphic character, press Return, and that character 
will be inserted in your file.


Shell to a new session

Press Shift-F2, and you can get to a command prompt easily. Initially,
you will find a reminder to type <exit> to get back to the editor, but
this will scroll off the screen, and you will need to remember this.


Safe_Save

A primary goal was the safety of your files.  By default, Safe_Save is
active.  This works by saving your current file to a temporary name, and
once that is safe, renaming the temporary file to the correct file name.
The only problem is if you run out of disk space. In this case KEd will
give you an error dialog, and let you continue.  You can turn off 
Safe_Save in the "Options" "Editor" menu, and save your file again.


Block Quoting

New with version 2.07 is an option to insert user-configurable quote
characters at the beginning of the line when you paste a block.
This can be useful in replying to mail messages. 
IE:      
When will Yoyodyne fix their C++ compiler
and provide an OS/2 assembler?

becomes:
>>When will Yoyodyne fix their C++ compiler
>>and provide an OS/2 assembler?


Pasting into KEd in an OS/2 Window

KEd (new with v2.07) fully supports Cut and Paste to and from OS/2 Windows,
but the default setting for "auto_indent" can cause problems if any of the 
pasted text is indented from the left margin.  It is suggested that you
turn off "auto_indent" if you experience column spacing irregularities.
(Options - Editor - Auto_Indent menu choices)

A WORD ABOUT THE INI FILE

Currently, KEd.INI (which stores the user configuration) is not upwardly
compatible between versions. While there are many user options, it should
only take 5 minutes to recreate your settings. KEd will refuse to load an
INI file from a later or earlier version.



A WORD ABOUT OS/2 TEXT MODE

OS/2 seems to have a bug in the VIO code concerning the first time a program
saves and restores the screen.  Sometimes when you exit KEd, it will restore 
the screen and it will be full of random characters and colors. Some SVGA 
adapters (like Trident) have this problem, but others don't. I've verified 
the problem on several computers using a small, 100% IBM source code utility.
IF you see video corruption, just "cls" the screen. KEd should not have a 
problem when run again. 


So what does KEd cost?

<Of historical interest only - as of 2014 FREE >

Currently, KEd costs $20 for a 1 user registration.  Continued development 
is ONLY possible with your support. I pledge to be very responsive to any 
comments regarding feature set and bug fixes, and at this time I
will offer support to unregistered users.  Any comments are welcome - send
US "snail mail" to the Atlanta address, or send electronic mail to the
Compuserve ID. Registered users will retain their license through the next 
major version change.  IE: if you register v2.07, you will retain your 
license up to and including v3.0x

To register, print out the REGISTER.DOC, and send a check or money order.
PLEASE send your address! I've gotten a few registrations without.
Corporations can purchase a multiple copy license or site license
at very attractive rates. Contact Kudzu Software for details.

There are no "begware" messages or "crippleware" disabled features.
Everyone is treated as a responsible person.


What are the limits?

Since KEd was designed for speed, some compromises had to be made. All
these limits can be changed, just let me know what you'd like to have.

256,000 lines
32MB total files capacity
NEW! No limits on screen modes. KEd supports any legal OS/2 video mode.

I think it's a little silly to have a text editor that can handle 4 gigabyte
files, or use an interpreter for everything, and those editors are much 
slower as a result. 

I hope you like KEd, and look forward to talking to you soon.

Don Radick
Kudzu Software

