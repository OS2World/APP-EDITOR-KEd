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
#BUGSW= -v -vi -N -xp
BUGSW= -v -vi- -N -xp
ASM_BUGSW= /zi
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
#.PATH.cpp= SRC     #tmp change
#.PATH.asm= SRC

OBJS= \
   blitt.obj    \
   getkey.obj   \
#   wasm.obj     \
   wcore.obj    \
   wdisp.obj    \
   simpwin.obj   \
   wline.obj    \
   wmove.obj    \
   wsetup.obj   \
   fld_edit.obj \
   misc.obj \
   tow_os2.obj \
#   os2_util.obj \
   basewin.obj \
   tty.obj \
   menu.obj


.ASM.OBJ:
#  masm /MX $<,$(.PATH.obj)\$&;
#   tasm /zi /MX /jmasm51 /jquirks $<, $(.PATH.obj)\$&
   tasm /zi /MX $<, $(.PATH.obj)\$&

.cpp.obj:
!if $d(__OS2__)
   $(CC) $(BUGSW) $(OPTSW) -c -Ii $(HFLAG) -n$(.PATH.obj) {$< }
!else
   $(CC) $(BUGSW) -c -Ii -m$(MODEL) $(HFLAG) -n$(.PATH.obj) {$< }
!endif

TOW_OS2.LIB: $(OBJS)
   del $<
   tlib /C $< @&&!
   +$(OBJ)\blitt.obj    &
   +$(OBJ)\getkey.obj   &
#   +$(OBJ)\wasm.obj     &
   +$(OBJ)\wcore.obj    &
   +$(OBJ)\wdisp.obj    &
   +$(OBJ)\simpwin.obj   &
   +$(OBJ)\wline.obj    &
   +$(OBJ)\wmove.obj    &
   +$(OBJ)\wsetup.obj   &
   +$(OBJ)\fld_edit.obj &
   +$(OBJ)\misc.obj     &
   +$(OBJ)\tow_os2.obj  &
   +$(OBJ)\basewin.obj &
   +$(OBJ)\tty.obj      &
   +$(OBJ)\menu.obj
!
