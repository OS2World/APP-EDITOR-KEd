;[]-----------------------------------------------------------------[]
;|   FILLWORDS.ASM --  sets memory to value                             |
;[]-----------------------------------------------------------------[]
;
;       KED v 2.10  August 1, 1994
; 
include g:\bcos2\source\rtl\rtlinc\RULES.ASI

;       Segments Definitions

Header@

;-----------------------------------------------------------------------
;
;Name            fillwords - sets memory to value
;
;Usage           void *fillwords(void *src, int c, size_t n);
;
;Prototype in    mem.h
;
;Description     sets the n 16 bit ints of the block pointed to by src to
;                c.
;
;Return value    void
;
;-----------------------------------------------------------------------

Code_seg@

;Func@   memset, _EXPFUNC, _RTLENTRYF, <pointer src>,<int c>,<int n>
Func@   fillwords, _EXPFUNC, _RTLENTRYF, <pointer src>,<int c>,<int n>

        Link@   edi
        mov     edi,src         ; get src
;        mov     al,c            ; get byte to store
        mov     ax,c            ; get ushort to store
;        mov     ah,al           ; copy to ah
        mov     dx,ax
        shl     eax,16          ; mov to upper word
        mov     ax,dx           ; get back lower word
        mov     ecx,n
        mov     edx,ecx         ; save count
        shr     ecx,1           ; calculate longword count
;        shr     ecx,1
        cld
        rep     stosd           ; store longwords
        mov     ecx,edx         ; get back count
;        and     ecx,3           ; calculate remainder byte count (0-3)
;        rep     stosb           ; store remaining bytes
        and     ecx,1           ; calculate remainder byte count (0-3)
        rep     stosw           ; store remaining bytes
        mov     eax,src         ; return src
        Unlink@ edi
        Return@

;EndFunc@ memset
EndFunc@ fillwords

Code_EndS@

        end
