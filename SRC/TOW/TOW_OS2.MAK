.AUTODEPEND

!if !$d(OBJ)
OBJ= os2_objs
!endif

# provide default memory model
!if !$d(MODEL)
MODEL=l
!endif

!if $d(__OS2__)
MODEL=
!endif

# allow overriding of the name of the compiler
!ifndef $d(__OS2__)
!if !$d(CC)
CC=tcc
!else
#!if $d(__OS2__)
CC=g:\bcos2\bin\bcc
!endif
!endif

# allow precompiled headers
!if $d(DASHH)
HFLAG=-H=$(DASHH)
!endif

# turn on debug info
!if $d(DEBUG)
BUGSW= -v -vi- -N -xp
#ASM_BUGSW= /zi
!endif

# optimize for release
!if $d(RELEASE)
#OPTSW= -Ox -O-a     #traps on win hide
#OPTSW= -Ob -Oc      #works fines
#OPTSW= -Ob -Oc -Oe  # traps on win hide
#OPTSW= -Ob -Oc -Ot  # traps on win hide
OPTSW= -Ob -Oc -Oz -a1
!else
OPTSW = -Od -a1 -w -w-sig
!endif


.PATH.obj= $(OBJ)

OBJS= \
   tblitt.obj    \
#  getkey.obj   \
   twcore.obj    \
   twdisp.obj    \
#  simpwin.obj   \
   twline.obj    \
   twmove.obj    \
   twsetup.obj   \
#  fld_edit.obj \
   tmisc.obj \
   tow_os2.obj \
#   os2_util.obj \
   tbasewin.obj 


#.ASM.OBJ:
#   tasm /zi /MX $<, $(.PATH.obj)\$&

.cpp.obj:
!if $d(__OS2__)
   $(CC) $(BUGSW) $(OPTSW) -c -Ii $(HFLAG) -n$(.PATH.obj) {$< }
!else
   $(CC) $(BUGSW) -c -Ii -m$(MODEL) $(HFLAG) -n$(.PATH.obj) {$< }
!endif

TOW_OS2.LIB: $(OBJS)
   del $<
   tlib /C $< @&&!
   +$(OBJ)\tblitt.obj    &
#   +$(OBJ)\getkey.obj   &
   +$(OBJ)\twcore.obj    &
   +$(OBJ)\twdisp.obj    &
#   +$(OBJ)\simpwin.obj   &
   +$(OBJ)\twline.obj    &
   +$(OBJ)\twmove.obj    &
   +$(OBJ)\twsetup.obj   &
#  +$(OBJ)\fld_edit.obj &
   +$(OBJ)\tmisc.obj     &
   +$(OBJ)\tow_os2.obj  &
   +$(OBJ)\tbasewin.obj 
#   +$(OBJ)\menu.obj
!
