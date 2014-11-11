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
;-----------------------------------------------------------------------

Code_seg@

Func@   memcpy_even, _EXPFUNC, _RTLENTRY, <pointer dest>,<pointer src>,<int n>

   ;    static void memcpy_even3 (USHORT* dest, byte const* source, USHORT count)

;   Link@   
   Link@   ebx
   ;                        {   
   ;                         for(int n = 1; n <= count; n++) {
    cld
    movzx ecx,word ptr [ebp+16]       ; count
;    mov cx,word ptr [ebp+16]       ; count
    mov edx,dword ptr [ebp+12]      ; source
    mov eax,dword ptr [ebp+8]       ; dest
@24:
    mov bl,byte ptr [edx]       ; *dest= *source;
    mov byte ptr [eax],bl
    add eax,2                   ; dest= dest + 2;
    inc edx                     ; source++;
    dec ecx                     ; increment count
;    dec cx                     ; increment count
    jnz short @24               ; do more

;    Unlink@ 
    Unlink@ ebx
    Return@

EndFunc@ memcpy_even

Code_EndS@

        end
