;
;   VIO_SAVE.ASM   for KEd v 2.15 1/7/95
;
;   save and restore an OS/2 VIO textmode screen.
;   based on viocall2.asm in  x:\bcos2\examples\tasm\thunk
;
;include g:\bcos2\source\rtl\rtlinc\RULES.ASI

ideal
p386
%LIST
                   ;  To get the proper jmp to the  global thunk_16:proc
include "viostruc.inc"
include "thunk.mac"
;include "kbdstruc.inc"

model flat,pascal
;stack 5000h

dataseg
blankchar db " ",1
screen_save db 4000 dup  (0)     ; buffer for screen save, hard code size
; screen_save_len =  $-screen_save
screen_save_len dw  4000
entermessage  db 13,10,'Press any key:'
entermessagelen = $-entermessage
;charin  kbdkeyinfo <>
vrows dd  0
vcols dd  0
modedata viomodeinfo {}

codeseg
;extrn DOSSLEEP:near
;start:                                 ; Clear the screen.
proc _vio_save                                     ; Clear the screen.

                                       ; Get the size of the screen
        call    Vio32GetMode pascal,offset modedata,0

                     ; Screen rows and columns start at 0,0 to 24,79
        xor     eax,eax
        mov     ax,[modedata.textrows]
        dec     eax
        xor     ebx,ebx
        mov     bx,[modedata.textcols]
        dec     ebx

        mov     [vrows], eax
        mov     [vcols], ebx

                                          ; NEW save screen
        call    Vio32ReadCellStr pascal, offset screen_save, offset screen_save_len, 0,0,0

        mov     [byte ptr blankchar],"*"
        mov     [byte ptr blankchar+1], 29
        call    Vio32ScrollUp pascal,0,0,[vrows],[vcols],[vrows],offset blankchar,0

        call    Vio32WrtTTY pascal,offset entermessage,entermessagelen,0
 
;        call    Kbd32FlushBuffer pascal,0
;        call    GetAnyKey

        call    Vio32WrtCellStr pascal, offset screen_save, offset screen_save_len, 0,0,0
        ret 


;proc    GetAnyKey
;        call    Kbd32CharIn,offset charin,0,0
;        call    Vio32WrtTTY pascal,offset charin.asciicode,1,0
;        ret
;endp

include "viocalls.inc"
;include "kbdcalls.inc"

;end start
ENDP

END























































