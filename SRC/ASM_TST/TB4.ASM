;[]-----------------------------------------------------------------[]
;|   MEMCPY_EVEN.ASM --  copy a block of n bytes from src to USHORT dst          |
;[]-----------------------------------------------------------------[]
;       KED v2.10 
; 
include g:\bcos2\source\rtl\rtlinc\RULES.ASI

;       Segments Definitions

Header@

;-----------------------------------------------------------------------
;
;Name           memcpy_even - copy a block of n bytes from src to USHORT dst
;
;Usage          void memcpy_even(void *dest, const void *src, size_t n);
;
;Description    memcpy_even copies  a block of n  bytes from src to dest.
;               skipping every other byte in dest     
;
;Return value   none
;
;Notes:         this version uses a loop instead of a jmp
;
;-----------------------------------------------------------------------

Code_seg@

Func@   memcpy_even, _EXPFUNC, _RTLENTRY, <pointer dest>,<pointer src>,<int n>

;   Link@   ebx,edi
   Link@   ebx
   ;    static void memcpy_even5 (USHORT* dest, byte const* source, USHORT count)
   ;    
;@27@54:
   ;                              for(int n = count; n > 0; n--) {
    movzx   ecx,word ptr [ebp+16]
    test    ecx,ecx
    jle short @31
    mov edx,dword ptr [ebp+12]     ; source
    mov eax,dword ptr [ebp+8]      ; dest
@30:
   ;                               *dest= *source;
;    movzx   bx,byte ptr [edx]
;    mov word ptr [eax],bx
    mov   bl,byte ptr [edx]
    mov  byte ptr [eax],bl
   ;                               dest++;
    add eax,2
   ;                               source++;
    inc edx
;    sub ecx,1
;    jg  short @30
   loop @30
@31:

;    Unlink@ ebx,edi
    Unlink@ ebx
    Return@

EndFunc@ memcpy_even

Code_EndS@

        end
