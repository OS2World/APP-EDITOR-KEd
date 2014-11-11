@memcpy_even$qpvpxvui   proc    near
   ;    static void memcpy_even (void* dest, void const* source, unsigned count)
@9@51:
    push    ebp
    mov ebp,esp
    push    ebx
    mov ecx,dword ptr [ebp+16]
   ;               {    
   ;                     while (count--) {
    mov eax,ecx
    dec ecx
    test    eax,eax
    je  short @13
    mov edx,dword ptr [ebp+12]
    mov eax,dword ptr [ebp+8]
@12:
   ;                  *(byte*)dest= *(byte*)source;
    mov bl,byte ptr [edx]
    mov byte ptr [eax],bl
   ;                  dest= (byte*)dest + 2;
    add eax,2
   ;                  source= (byte*)source + 1;
    inc edx
    mov ebx,ecx
    dec ecx
    test    ebx,ebx
    jne short @12
@13:
@14:
@14@0:
    pop ebx
    pop ebp
    ret 12
@memcpy_even$qpvpxvui   endp
