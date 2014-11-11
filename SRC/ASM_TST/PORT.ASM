  ; *************************************************************
  ; * Assembly Language module that declares a 16 bit segment.  *
  ; * It is required that a 16 bit segment is used to create an *
  ; * IOPL segment.                                             *
  ; *************************************************************

  P386
  ; Procedure in the IOPL segment must be pascal to
  ; clean up their stack.
  MODEL     LARGE, PASCAL

  IOPLCODE  SEGMENT   WORD PUBLIC USE16 'CODE'
            ASSUME    CS:IOPLCODE

            PUBLIC    WPORTW ; Write a word to a port
            PUBLIC    WPORTB ; Write a byte to a port
            PUBLIC    RPORTW ; Read a word from a port
            PUBLIC    RPORTB ; Read a byte from a port
  ; **************************************************************
  ; * Write port functions WPORTW and WPORTB are called with EDX *
  ; * holding both the port address and the value to write.      *
  ; * EDX low word contains the address of the port, and the EDX *
  ; * high word contains the value to write.                     *
  ; **************************************************************
  WPORTW    PROC      FAR
            push      edx

            xor       eax, eax            ; clear eax
            shld      eax, edx, 16        ; get the value to write

            out       dx,ax

            pop       edx
            ret
            ENDP

  WPORTB    PROC      FAR
            push      edx

            xor       eax, eax            ; clear the eax
            shld      eax, edx, 16        ; get the value to write

            out       dx,al

            pop       edx
            ret
            ENDP

  ;**************************************************************
  ;* The Read port functions should contain the port address to *
  ;* read from  the DX register. The CX register will contain   *
  ;* the value read from the port in either CL or CX for byte   *
  ;* or word port reads.                                        *
  ;**************************************************************
  RPORTW    PROC       FAR

            in         ax,dx
            mov        ecx, eax

            ret
            ENDP

  RPORTB    PROC       FAR

            in         al,dx
            mov        ecx, eax

            ret
            ENDP

  IOPLCODE  ENDS
            END
































