@memcpy_even2$qpucpxucus    proc    near
   ;    static void memcpy_even2 (byte* dest, byte const* source, USHORT count)
@15@54:
    push    ebp
    mov ebp,esp
    push    ebx
    mov cx,word ptr [ebp+16]
   ;               {                   
   ;                   while (count--) {
    mov ax,cx
    dec cx
    test    ax,ax
    je  short @19
    mov edx,dword ptr [ebp+12]
    mov eax,dword ptr [ebp+8]
@18:
   ;                        *dest= *source;
    mov bl,byte ptr [edx]
    mov byte ptr [eax],bl
   ;                        dest= dest + 2;
    add eax,2
   ;                        source++;
    inc edx
    mov bx,cx
    dec cx
    test    bx,bx
    jne short @18
@19:
@20:
@20@0:
    pop ebx
    pop ebp
    ret 12
@memcpy_even2$qpucpxucus    endp
