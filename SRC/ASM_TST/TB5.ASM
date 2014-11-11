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
;Notes          this version uses Abrash branch-to partial in-line code
;
;-----------------------------------------------------------------------

Code_seg@

Func@   memcpy_even, _EXPFUNC, _RTLENTRY, <pointer dest>,<pointer src>,<int n>

   ;    static void memcpy_even3 (USHORT* dest, byte const* source, USHORT count)

   Link@   ebx,edi,esi, ecx, edx

   movzx ecx,word ptr  [ebp+16]      ; count
   mov   edi,dword ptr [ebp+12]      ; source
   mov   esi,dword ptr [ebp+8]       ; dest
   ;                        {   
   ;                         for(int n = 1; n <= count; n++) {
Memcpy_Even_Func:
     cld
     movzx  eax, cx
     and    eax, 3                ; # of reps mod 4
     mov    edx, eax              ; save it
     shl    eax, 3
;     add    eax,edx
     mov    edx, offset InlineMemcpyEnd
     sub    edx,eax
     shr    cx,1
     shr    cx,1
     inc    cx
     jmp    edx                  ; jmp to rep routine
InlineMemcpy:
    mov bl,byte ptr [edi]       ; *dest= *source;
    mov byte ptr [esi],bl
    add esi,2                   ; dest= dest + 2;
    inc edi                     ; source++;
Do3:
    mov bl,byte ptr [edi]       ; *dest= *source;
    mov byte ptr [esi],bl
    add esi,2                   ; dest= dest + 2;
    inc edi                     ; source++;
Do2:
    mov bl,byte ptr [edi]       ; *dest= *source;
    mov byte ptr [esi],bl
    add esi,2                   ; dest= dest + 2;
    inc edi                     ; source++;
Do1:
    mov bl,byte ptr [edi]       ; *dest= *source;
    mov byte ptr [esi],bl
    add esi,2                   ; dest= dest + 2;
    inc edi                     ; source++;
InlineMemcpyEnd:
    loop InlineMemcpy            ; do more

    Unlink@ ebx,edi,esi, ecx, edx
    Return@

EndFunc@ memcpy_even

Code_EndS@

        end
