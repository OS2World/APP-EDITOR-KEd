a	b	c	d
 a	b	c	d
  a	b	c	d
   a	b	c	d
			a
			.386p
	ifndef	??version
?debug	macro
;			 //  break;				 // unreachable code warn
 ;			 //  break;				 // unreachable code warn
  ;			 //  break;				 // unreachable code warn
   ;			 //  break;				 // unreachable code warn

	endm
	endif
	?debug	S "tow_os2.cpp"
	?debug	T "tow_os2.cpp"
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
_DATA	segment dword public use32 'DATA'
_DATA	ends
_BSS	segment dword public use32 'BSS'
_BSS	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_TEXT	segment dword public use32 'CODE'
c@	label	byte
	assume	cs:_TEXT
_video_int	proc	near
   ;    
   ;	extern "C" USHORT video_int (USHORT AX,USHORT BX,USHORT CX,USHORT DX)
   ;    
@5@40:
	enter	44,0
	push	edi
	mov di,word ptr [ebp+20]
   ;    
   ;	{
   ;	  int Service = AX >> 8;
   ;    
	movzx	eax,word ptr [ebp+8]
	sar eax,8
   ;    
   ;	  VIOMODEINFO	viomode;
   ;	  VIOCURSORINFO cursordata;
   ;	  USHORT usRow, usCol;
   ;    
   ;	  switch(Service)
   ;    
	sub eax,2
	je	short @14
	dec eax
	je	short @15
	sub eax,12
	jne near ptr @16
@10:
   ;    
   ;	  { 
   ;		 case 15:
   ;				 viomode.cb = sizeof(viomode);		// get vid mode
   ;    
	mov word ptr [ebp-36],34
   ;    
   ;				 rc = VioGetMode(&viomode, 0);
   ;    
	lea eax,byte ptr [ebp-36]
	ror eax,16
	shl ax,3
	or	al,7
	rol eax,16
	push	eax
	push	word ptr 0
	call	near ptr VIO16GETMODE$32
	movzx	eax,ax
	mov large dword ptr _rc,eax
   ;    
   ;				 if(viomode.fbType & VGMT_OTHER)
   ;    
	test	byte ptr [ebp-34],1
	je	short @13
   ;    
   ;					return	0x0700;
   ;    
	mov ax,1792
	jmp short @19
	jmp short @14
@13:
   ;    
   ;				 else
   ;					return	0x0300;
   ;    
	mov ax,768
	jmp short @19
@14:
   ;    
   ;			 //  break;				 // unreachable code warn
   ;		 case 2:
   ;				 cursordata.attr   = -1;		 // hide cursor
   ;    
	mov word ptr [ebp-38],-1
   ;    
   ;				 cursordata.yStart =  0;		 // top
   ;    

