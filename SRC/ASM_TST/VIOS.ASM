;
;   VIO_SAVE.ASM   for KEd v 2.15 1/7/95
;
;   save and restore an OS/2 VIO textmode screen.
;   based on viocall2.asm in  x:\bcos2\examples\tasm\thunk
;
;    this file compiles, links and runs perfectly
;    tested that it will work fine if called from a CPP main() prog
;
;    LIMITATION: the save/restore buffer must be allocated in this module.
;
;
ideal
p386

extrn VIO16GETMODE:far
extrn VIO16READCELLSTR:far
extrn VIO16WRTCELLSTR:far

GLOBAL c screen_save_buf:BYTE   
GLOBAL c screen_save_buf_len:WORD


%MACS
;**********************************************************************
; VIOSTRUC.INC                 ; Structure for Vio16GetMode
struc viomodeinfo
length      dw  14
vtype       db  0
vnumcolors  db  0
textcols    dw  0
textrows    dw  0
pelcols     dw  0
pelrows     dw  0
reserved1   db  0
reserved2   db  0
ends

;************************************************************************
;include "thunk.mac"
;************************************************************************
; FILE: THUNK.MAC
;
; This file contains a macro that sets up thunks for calling 16 bit routines.
;        include "crma.inc"
;************************************************************************
; FILE:CRMA.INC

; This macro performs the CRMA transformation on an 32 bit offset operand
; to turn it into a 16 bit segment and offset value.

; Target32 is the item to be converted, a register such as EAX,EBX,ECX
; EDX destroyed by this macro.
macro   crma  target32
        local lower16bits,lower8bits
ifidni  <target32>,<EAX>
  lower16bits equ <AX>
  lower8bits  equ <AL>
elseifidni <target32>,<EBX>
  lower16bits equ <BX>
  lower8bits  equ <BL>
elseifidni <target32>,<ECX>
  lower16bits equ <CX>
  lower8bits  equ <CL>
elseifidni <target32>,<EDX>
  lower16bits equ <DX>
  lower8bits  equ <DL>
endif

        mov     dx,ss   ; Steal the PL bits from the stack
        and     dl,03h  ; Get PL bits into dl
        or      DL,04H  ; Force LDT bit on

        ror     target32,16 ; Move the highword into AX
        shl     lower16bits,3    ; Create tiled selector
        or      lower8bits,dl   ; Add the PL bits
        ROL     target32,16  ; Restore proper order
        endm


; This macro reverses the CRMA algorithm
macro   deCRMA  target32
        local lower16bits,lower8bits
ifidni  target32,<EAX>
  lower16bits equ <AX>
  lower8bits  equ <AL>
elseifidni target32,<EBX>
  lower16bits equ <BX>
  lower8bits  equ <BL>
elseifidni target32,<ECX>
  lower16bits equ <CX>
  lower8bits  equ <CL>
elseifidni target32,<EDX>
  lower16bits equ <DX>
  lower8bits  equ <DL>
endif

        ror     target32,16
        shr     lower16bits,3
        rol     target32,16

        endm

;************************************************************************

macro   GenerateThunk name,calling,paramsize,params
        local thunk_16,back_to_32,L0

ends

extrn   name:far

codeseg
proc    name&Thunk calling near  ; params; Start the 32 bit part of the thunk
        local   tempstore:dword
        push    ebx
        push    edi
        push    esi
        push    es             ; bump stack down to just below a 64K boundary
        mov     [tempstore],esp
        mov     ax,ss
        cmp     ax,_DATA
        je      L0
        mov     eax,esp
        cmp     ax,1000h       ; Thunk minimum stack
        jnb     L0
        xor     ax,ax
        xchg    eax,esp
L0:
        push    ebp                       ; save EBP, ESP, and SS
        mov     ebx,esp
        push    ss
        push    ebx

        ; This is the code to copy the parameter list to the new
        ; stack. This code has several versions. For a normal thunk
        ; with a non-struct return value:
        lea     esi,[ebp+8]
if paramsize
        mov     ecx,paramsize    ; Load ECX with the param list size.
else
        xor     ecx,ecx
endif
        sub     esp,ecx
        mov     edi,esp
        rep     movsb

        mov     eax,esp   ; convert SS:ESP to SS:SP for a 0:32->16:16 thunk.
        ror     eax,16
        shl     ax,3
        or      al,7
        rol     eax,16
        push    eax
        lss     sp,[esp]
                                   ; far (16:16) jump to func's 16 bit thunk
        jmp     far ptr thunk_16

back_to_32:
        shl     eax,10h            ; Convert return value from AX:DX to EAX
        shrd    eax,edx,16   

ifidni <calling>,<c>
if paramsize
        add     esp,paramsize
endif
endif
                  ; restore ESP and EBP after returning from the 16 bit part
        movzx   esp,sp
        lss     esp,[esp]
        pop     ebp

        mov     esp,[tempstore]                       ; restore ES
        pop     es

        pop     esi                      ; Finish the function exit block
        pop     edi
        pop     ebx
        ret
endp    name&Thunk

;*******************************************************************
segment code16 word public use16 'CODE'
        assume cs:code16

proc    thunk_16
        call small name
        jmp     far ptr back_to_32              ; Go back to 32 bits
endp    thunk_16

ends    code16

        assume cs:flat,ds:flat
        endm

;**********************************************************************
; Macros used for generating 16 bit stack frames.

macro   storeword   which   ; Macro for placing a word sized argument
        mov     eax,which
        mov     [edi],ax
        inc     edi
        inc     edi
endm
macro   storedword   which  ; Macro for placing a dword sized argument
        mov     eax,which
        mov     [edi],eax
        add     edi,4
endm

macro   storeaddress which
        mov     eax,which
        CRMA    eax
        mov     [edi],eax
        add     edi,4
endm

;************************************************************************

; Makes all the thunk routines for a 16 bit call.
macro   MakeThunk Name32,Name16,arg1,type1,arg2,type2,arg3,type3,arg4,type4,arg5,type5,arg6,type6,arg7,type7

arg16size = 0          ; Calc the size of the 16 bit stack frame

macro MakeThunk_addsize typeparam
ifidni <typeparam>,<word>
    arg16size = arg16size + 2
elseifidni <typeparam>,<dword>
    arg16size = arg16size + 4
elseifidni <typeparam>,<address>
    arg16size = arg16size + 4
endif
endm

MakeThunk_addsize type1
MakeThunk_addsize type2
MakeThunk_addsize type3
MakeThunk_addsize type4
MakeThunk_addsize type5
MakeThunk_addsize type6
MakeThunk_addsize type7


;**********************************************************************
;; Get a count of the arguments
ifb <arg1>
    argcount  = 0
proc    Name32  c
elseifb <arg2>
    argcount  = 1
proc    Name32 pascal arg1:dword

elseifb <arg3>
    argcount  = 2
proc    Name32 pascal arg1:dword,arg2:dword

elseifb <arg4>
    argcount  = 3
proc    Name32 pascal arg1:dword,arg2:dword,arg3:dword

elseifb <arg5>
    argcount  = 4
proc    Name32 pascal arg1:dword,arg2:dword,arg3:dword,arg4:dword

elseifb <arg6>
    argcount  = 5
proc    Name32 pascal arg1:dword,arg2:dword,arg3:dword,arg4:dword,arg5:dword

elseifb <arg7>
    argcount  = 6
proc    Name32 pascal arg1:dword,arg2:dword,arg3:dword,arg4:dword,arg5:dword,arg6:dword

else
    argcount  = 7
proc    Name32 pascal arg1:dword,arg2:dword,arg3:dword,arg4:dword,arg5:dword,arg6:dword,arg7:dword

endif

;************************************************************************
        ; This routine uses direct memory stores to setup the 16 bit stack

        sub     esp,arg16size      ; Setup room for the 16 bit stack setup
        mov     edi,esp            ; Get a pointer to the newly created room

if argcount gt 6
        store&type7  [arg7]
endif
if argcount gt 5
        store&type6  [arg6]
endif
if argcount gt 4
        store&type5  [arg5]
endif
if argcount gt 3
        store&type4  [arg4]
endif
if argcount gt 2
        store&type3  [arg3]
endif
if argcount gt 1
        store&type2  [arg2]
endif
if argcount gt 0
        store&type1  [arg1]
endif

        call     Name16&Thunk
        ret
endp    Name32

GenerateThunk Name16,pascal,arg16size

endm

;************************************************************************

model flat,pascal     ; needs pascal linkage on API also
;model flat,c
;stack 5000h

dataseg
blankchar db " ",1
screen_save_buf db 13200 dup  (0)     ; buffer for screen save, hard code size
screen_save_buf_len dw  13200
;
;
;     PUBLIC didn't compile...
;
;PUBLIC screen_save_buf db 13200 dup  (0)     ; buffer for screen save, hard code size
;PUBLIC screen_save_buf_len dw  13200
;
vrows dd  0
vcols dd  0
modedata viomodeinfo {}

codeseg

PUBLIC c screen_save
PUBLIC c screen_restore

;     NOTES:
;     restore works ok as "pascal", next try restore as "c"
;     restore OK as "c", next try restore called from vio_save.cpp
;     restore called from vio_Save.cpp OK
;     OLD - GOOD asm versions saved as vios_good#.asm
;
;     eliminated Kbd stuff, calling DosSleep from vio_Save.cpp
;
;     This version works OK. VioScrollUP only called from vio_save.cpp now
;
;
;

proc screen_save c
start:                                 ; Clear the screen.
        call    _my_get_mode 
        call    Vio32ReadCellStr pascal, offset screen_save_buf, offset screen_save_buf_len, 0,0,0
        ret 
endp

proc _my_get_mode 
        call    Vio32GetMode pascal,offset modedata,0

        xor     eax,eax
        mov     ax,[modedata.textrows]
        mov     [vrows], eax
        xor     ebx,ebx
        mov     bx,[modedata.textcols]
        mov     [vcols], ebx

        imul    eax, ebx
        imul    eax, 2
        mov     [screen_save_buf_len], ax
        dec     [vrows]                ; convert to zero based coords
        dec     [vcols]
        ret
endp

proc screen_restore c
        call    Vio32WrtCellStr pascal, offset screen_save_buf, offset screen_save_buf_len, 0,0,0
        ret 
endp

;************************************************************************
; VIOCALLS.INC

macro   MakeVioThunk  name,arg1,type1,arg2,type2,arg3,type3,arg4,type4,arg5,type5,arg6,type6,arg7,type7
        MakeThunk Vio32&Name,Vio16&Name,arg1,type1,arg2,type2,arg3,type3,arg4,type4,arg5,type5,arg6,type6,arg7,type7
endm

macro   Vio16modecall which
        MakeVioThunk  which  ModeStruc address,viohandle word
        endm

        Vio16modecall GetMode

        MakeVioThunk  ReadCellStr CellString address,slen address,row word,column word,hvio word
        MakeVioThunk  WrtCellStr pcellstr address,slen word,row word,column word,hvio word

;************************************************************************

;end start
end















