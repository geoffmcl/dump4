
// D4Bmp.c

/* ======================================

Proc_BMP:
	SUB	CX,CX
	SUB	DX,DX
	MOV	AL,1		; GET CURRENT POSITION
	DOS_MOVEPTR
	JNC	@F
	JMP	Err_BMP
@@:	Push	Ax
	Push	Dx		; Keep CURRENT position
	SUB	CX,CX
	SUB	DX,DX
	MOV	AL,2		; GET EOF POSITION
	DOS_MOVEPTR
	Mov	FSIZE1,AX
	Mov	FSIZE2,DX
	Pop	Dx
	Pop	Cx
	Jnc	@F
	JMP	Err_BMP
@@:	Mov	Al,0		; Begin PLUS offset
	DOS_MOVEPTR
	Jnc	@F
	JMP	Err_BMP
@@:
	Cmp	FSIZE2,0
	Jne	@F
	Mov	Cx,FSIZE1
	CMP	CX,(SIZE BITMAPFILEHEADER + SIZE BITMAPINFOHEADER)
	JBE	Nul_BMP
@@:
;BITMAPFILEHEADER struc
;    bfType          dw ?
;    bfSize          dd ?
;    bfReserved1     dw ?
;    bfReserved2     dw ?
;    bfOffBits       dd ?
;BITMAPFILEHEADER ends
;	Just READ a BLOCK at a TIME
	Mov	Cx,SIZE BITMAPFILEHEADER
	Call	Read_BMP_File
	Jc	Err_BMP
	Cmp	Ax,Cx
	Jne	Err_BMP
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
;	Set BITMAPFILEHEADER Pointer
	Mov	[BBSEG],ES
	Mov	[BBOFF],DI
	MOV	Ax,es:[Di]	; Get FIRST WORD
	POP	ES
	CMP	Ax,'MB'		; Is it a BIT MAP FILE
	Je	Got_BMP		; NOT A BITMAP FILE
Nul_BMP:
	dpDmsg	<'ERROR: File length less than BITMAPFILEHEADER + BITMAPINFOHEADER!'>
	MOV	[EXIT_VAL],1
	JMP	End_BMP
Err_BMP:
	dpDmsg	<'ERROR: File Handling Failed ...'>
	MOV	[EXIT_VAL],1
	JMP	End_BMP
End_BMP:
	JMP	GET_FIN_TIME	; ALL OVER
Got_BMP:
	Mov	Dx,Offset BM_Msg0
	Mov	Cx,LBM_Msg0
	Call	Out_Msg
	Mov	Dx,Offset FN_LOC
	Call	Out_2_Nul
	Mov	Dx,Offset BM_Msg00
	Mov	Cx,LBM_Msg00
	Call	Out_Msg
	Mov	Ax,FSIZE1
	Mov	Dx,FSIZE2
	Call	Out_Bytes
	Mov	Dx,Offset BM_Msg1
	Mov	Cx,LBM_Msg1
	Call	Out_Msg
	Mov	Ax,SIZE BITMAPFILEHEADER
	Mov	Dx,0
	Call	Out_Bytes
	Mov	Dx,Offset BM_Msg1a
	Mov	Cx,LBM_Msg1a
	Call	Out_Msg

	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,es:[Di].BITMAPFILEHEADER.bfType	; Get FIRST WORD
	POP	ES
	Call	Out_Ax_Hex
	Mov	Dx,Offset BM_Msg2
	Mov	Cx,LBM_Msg2
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,Word ptr es:[Di+2].BITMAPFILEHEADER.bfSize	; Get 2nd WORD
	Mov	Dx,Ax
	MOV	Ax,Word ptr es:[Di+0].BITMAPFILEHEADER.bfSize	; Get 2nd WORD
	POP	ES
	Call	Out_AXDX_Dec
	Mov	Dx,Offset BM_Msg3
	Mov	Cx,LBM_Msg3
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,Word ptr es:[Di+2].BITMAPFILEHEADER.bfOffBits ; Get 2nd WORD
	POP	ES
	Call	Out_Ax_Hex
	Mov	Al,':'
	Call	Out_Al
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,Word ptr es:[Di+0].BITMAPFILEHEADER.bfOffBits ; Get 2nd WORD
	POP	ES
	Call	Out_Ax_Hex
	Call	Out_CrLf

;	Move on to BITMAPINFOHEADER
	PUSH	ES
	LES	DI,[B_SEG3]
	ADD	DI,SIZE BITMAPFILEHEADER
	JNC	@F
	MOV	AX,ES
	ADD	AX,1000h
	MOV	ES,AX
@@:	MOV	WORD PTR [B_SEG3+0],DI
	MOV	WORD PTR [B_SEG3+2],ES
	MOV	[BMIOFF],DI
	MOV	[BMISEG],ES
	POP	ES

	MOV	CX,SIZE BITMAPINFOHEADER
	Call	Read_BMP_File	; Read NEXT Block
	Jnc	@F
	Jmp	Err_BMP
@@:	Cmp	Ax,Cx
	Je	@F
	Jmp	Err_BMP
@@:
;BITMAPINFOHEADER struc
;        biSize           dd ?
;        biWidth          dd ?
;        biHeight         dd ?
;        biPlanes         dw ?
;        biBitCount       dw ?
;        biCompression    dd ?
;        biSizeImage      dd ?
;        biXPelsPerMeter  dd ?
;        biYPelsPerMeter  dd ?
;        biClrUsed        dd ?
;        biClrImportant   dd ?
;BITMAPINFOHEADER ends

	Mov	Dx,Offset BM_IM1
	Mov	Cx,LBM_IM1
	Call	Out_Msg

	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biSize	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biSize	; Get FIRST WORD
	POP	ES

	push	ax
	push	dx
	;;Call	Out_AXDX_Dec
	Call	Out_Bytes
	pop	dx
	pop	ax
	or	dx,dx
	jnz	Not_WBMP
	cmp	ax,SIZE BITMAPINFOHEADER
	Je	@F
Not_WBMP:
	mov	dx,offset BM_NWB
	mov	cx,LBM_NWB
	call	Out_Msg
	Jmp	Do_Dump
@@:	
	Mov	Dx,Offset BM_IM2
	Mov	Cx,LBM_IM2
	Call	Out_Msg

	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biWidth	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biWidth	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec

	Mov	Dx,Offset BM_IM3
	Mov	Cx,LBM_IM3
	Call	Out_Msg
		
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biHeight	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biHeight	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec

	Mov	Dx,Offset BM_IM4
	Mov	Cx,LBM_IM4
	Call	Out_Msg
		
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,es:[Di].BITMAPINFOHEADER.biPlanes	; Get FIRST WORD
	Mov	Dx,0
	POP	ES
	Call	Out_AXDX_Dec

	Mov	Dx,Offset BM_IM5
	Mov	Cx,LBM_IM5
	Call	Out_Msg
		
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,es:[Di].BITMAPINFOHEADER.biBitCount	; Get FIRST WORD
	Mov	Dx,0
	POP	ES
	Call	Out_AXDX_Dec

	Mov	Dx,Offset BM_IM5_MONO
	Mov	[ColorCnt],2
	Cmp	Ax,1
	Je	@F
	Mov	Dx,Offset BM_IM5_16
	Mov	[ColorCnt],16
	Cmp	Ax,4
	Je	@F
	Mov	Dx,Offset BM_IM5_256
	Mov	[ColorCnt],256
	Cmp	Ax,8
	Je	@F
	Mov	Dx,Offset BM_IM5_24
	Mov	[ColorCnt],0
	Cmp	Ax,24
	Je	@F
	Mov	Dx,Offset BM_IM6_UNK
	Mov	[ColorCnt],-1
@@:	Call	Out_2_Nul

	Mov	Dx,Offset BM_IM6
	Mov	Cx,LBM_IM6
	Call	Out_Msg
		
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biCompression	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biCompression	; Get FIRST WORD
	POP	ES
	Push	Ax
	Push	Dx
	Call	Out_AXDX_Dec
	Pop	Dx
	Pop	Ax
	Mov	[BmComp],-1		; Set an UNKNOWN value
	Or	Dx,Dx
	Mov	Dx,Offset BM_IM6_UNK	; LOAD "UNKNOWN" string
	Jnz	Shw_Comp		; Bad UNKNOWN value
	Mov	[BmComp],Ax		; SAVE the COMPRESSION TYPE
	Mov	Di,Offset BM_IM6_0
	Cmp	Ax,[Di]
	Je	Set_Comp
	Mov	Di,Offset BM_IM6_1
	Cmp	Ax,[Di]
	Je	Set_Comp
	Mov	Di,Offset BM_IM6_2
	Cmp	Ax,[Di]
	Je	Set_Comp
	Mov	Di,Offset BM_IM6_3
	Cmp	Ax,[Di]
	Jne	Shw_Comp
Set_Comp:
	Mov	Dx,Di
	Add	Dx,2
Shw_Comp:
	Call	Out_2_Nul

	Mov	Dx,Offset BM_IM7
	Mov	Cx,LBM_IM7
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biSizeImage	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biSizeImage	; Get FIRST WORD
	POP	ES
	Mov	[BmSiz1],Ax	; Keep this IMAGE SIZE
	Mov	[BmSiz2],Dx	; (IF ANY GIVEN)
	Call	Out_AXDX_Dec
		
	Mov	Dx,Offset BM_IM8
	Mov	Cx,LBM_IM8
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biXPelsPerMeter	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biXPelsPerMeter	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec
		
	Mov	Dx,Offset BM_IM9
	Mov	Cx,LBM_IM9
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biYPelsPerMeter	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biYPelsPerMeter	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec
		
		
	Mov	Dx,Offset BM_IM10
	Mov	Cx,LBM_IM10
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biClrUsed	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biClrUsed	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec
		
		
	Mov	Dx,Offset BM_IM11
	Mov	Cx,LBM_IM11
	Call	Out_Msg
	PUSH	ES
	LES	DI,[B_SEG3]	; Get data point - File i/o
	MOV	Ax,word ptr es:[Di+2].BITMAPINFOHEADER.biClrImportant	; Get FIRST WORD
	Mov	Dx,Ax
	MOV	Ax,word ptr es:[Di+0].BITMAPINFOHEADER.biClrImportant	; Get FIRST WORD
	POP	ES
	Call	Out_AXDX_Dec

;	Move on to COLOUR MAP
	PUSH	ES
	LES	DI,[B_SEG3]
	ADD	DI,SIZE BITMAPINFOHEADER
	JNC	@F
	MOV	AX,ES
	ADD	AX,1000h
	MOV	ES,AX
@@:	MOV	WORD PTR [B_SEG3+0],DI
	MOV	WORD PTR [B_SEG3+2],ES
	MOV	[BMCOFF],DI
	MOV	[BMCSEG],ES
	POP	ES

	Mov	Dx,Offset BM_CM1
	MOV	Cx,LBM_CM1
	Call	Out_Msg

	Mov	Ax,[ColorCnt]	; Get Colour count
	Mov	Dx,0
	Call	Out_AXDX_Dec

	Cmp	Ax,-1		; IF ERRANT COLOUR COUNT!!!
	Jne	Col_BMP		; GO do the COLOURS
;	Errant BMP
Err_Colr:
	Mov	Dx,Offset BM_CM2
	MOV	Cx,LBM_CM2
	Call	Out_Msg
Do_Dump:
	Mov	[CUR_COL],0
	Push	ES
	LES	DI,[BBSEG3]	; Get original pointer
	MOV	[B_S3OFF],DI
	MOV	[B_S3SEG],ES
	Pop	ES
	JMP	DO_DISP		; Back to MAIN LINE
Col_BMP:
	MOV	Ax,[ColorCnt]
	MOV	Cx,4
	Sub	Dx,Dx
	Mul	Cx
	Mov	[COLRCNT],Ax
	Or	Dx,Dx
	Jz	@F
	Jmp	Err_Colr
@@:
;;	Mov	Dx,Offset BM_CM1a
;;	MOV	Cx,LBM_CM1a
;;	Call	Out_Msg
	Mov	Ax,[COLRCNT]
	Mov	Dx,0
	Call	Out_Bytes
;;	Call	Out_AXDX_Dec
;;	Mov	Al,')'
;;	Call	Out_Al

;	How to display the colours
	MOV	Ax,[ColorCnt]
	Or	Ax,Ax
	Jnz	Not_C24
;	No COLOR MAP
	Mov	Dx,Offset BM_CM3
	Mov	Cx,LBM_CM3
	Call	Out_Msg
	Jmp	Dn_BCOL
Not_C24:
	Call	Out_CrLf		
	MOV	Ax,[ColorCnt]
	Mov	Cx,2
	Cmp	Ax,2
	Je	@F
	Mov	Cx,4
@@:
Put_CHdr:
	Push	Cx
	Mov	Dx,Offset BM_CM4
	Mov	Cx,LBM_CM4
	Call	Out_Msg
	Pop	Cx
	Loop	Put_CHdr
	Call	Out_CrLf

	Mov	Cx,[COLRCNT]
	Call	Read_BMP_File	; Read NEXT Block - The COLOUR TABLE
	Jnc	@F
	Jmp	Err_BMP
@@:	Cmp	Ax,Cx
	Je	@F
	Jmp	Err_BMP
@@:
;;;111(000,000,000) 111(000,000,000) 111(000,000,000) 111(000,000,000)
	Push	ES
	Les	Di,[B_SEG3]	; Set DI pointing to COLOURS
	Pop	Es
	MOV	Si,[ColorCnt]	; And COUNT of RGBQUAD
	Sub	Dx,Dx
	Sub	Bx,Bx

Colr_Loop:

	Push	Si

	Mov	Ax,Bx
	Call	Out_Dec3
	Mov	Al,'('
	Call	Out_Al
	Mov	Cx,3
One_Colr:
	Push	Es
	Les	Si,[B_SEG3]
	Mov	Al,Es:[Di]
	Pop	Es
	Sub	Ah,Ah
	Inc	Di
	Call	Out_Dec3
	Cmp	Cx,1
	je	@F
	Mov	Al,','
	Call	Out_Al
@@:
	Loop	One_Colr
	inc	Di		; And final bump
	inc	Bx		; Bump INDEX
	inc	Dx		; Bump SET of 4
	Mov	Al,')'
	Call	Out_Al
	Mov	Al,' '
	Call	Out_Al

	Pop	Si

	Dec	Si
	Jz	End_Colr
	Cmp	Dx,4
	Jb	@F
	Call	Out_CrLf
	Sub	Dx,Dx
@@:	Jmp	Colr_Loop
End_Colr:
	Or	Dx,Dx
	Jz	Dn_BCOL
	Call	Out_CrLf
Dn_BCOL:
;	Move on to BITMAP DATA
	Mov	Ax,[COLRCNT]
	PUSH	ES
	LES	DI,[B_SEG3]
	ADD	DI,Ax
	JNC	@F
	MOV	AX,ES
	ADD	AX,1000h
	MOV	ES,AX
@@:
	MOV	WORD PTR [B_SEG3+0],DI
	MOV	WORD PTR [B_SEG3+2],ES
	MOV	[BMDOFF],DI
	MOV	[BMDSEG],ES
	POP	ES

	Mov	Dx,Offset BM_DM1
	Mov	Cx,LBM_DM1
	Call	Out_Msg

	Mov	[BmOdd],0	; Flag NOT odd size

	Mov	Ax,[ColorCnt]	; This is 2, 16, 256 or ZERO
	Cmp	Ax,2
	Je	Do_MONO
	Jmp	Not_Mono
Do_MONO:
;	A MONOCHROME is where each BIT of each BYTE is a COLOR Pixel
;	0 = First colour 1 = Second Colour
;	So
;	1. Get BYTES needed to contain PIXEL Width
;		Round UP as required
;	==========================================
	Call	Get_Width
	Mov	Bx,0
	Mov	Cx,8		; Remainder from 8
	Push	Ax		; Save WIDTH
	Push	Dx

	push	bx
	push	cx
	push	dx
	push	ax
	mov	dx,0
	Call	G_Rem		; Get remainder after DIV by 4 (32-bit)
	Or	Ax,Ax		; Falg an BYTE remainder

	Pop	Dx		; Recover Widht
	Pop	Ax

	Jz	@F		; NO, Keep current WIDTH

	Mov	Bx,0		; Else
	Mov	Cx,8		; Divide by 8
	Push	bx
	push	cx
	push	dx
	push	ax
	Call	G_Div

	Add	ax,1		; Add ONE
	Adc	dx,0

	Mov	Bx,0		; and
	Mov	Cx,8		; Multiply by 8
	Push	bx
	push	cx
	push	dx
	push	ax
	Call	G_Mul		; To get ADJUST WIDTH

@@:
	Mov	Bx,0		; NOW
	Mov	Cx,8		; Divide by 8
	Push	bx
	push	cx
	push	dx
	push	ax
	Call	G_Div		; We now have the BYTE WIDTH

	Mov	[SigCol1],Ax
	Mov	[SigCol2],Dx	; Significant BYTES

;	2. This MUST be on a 32-Bit BOUNDARY, so
;	========================================
	Call	Get_Round32
	Mov	[AdjLine1],Ax
	Mov	[AdjLine2],Dx

	Call	Out_AXDX_Dec	; BYTE Width of MONCHROME
	Mov	Al,'x'
	Call	Out_SpxSp
	
	Call	Get_Height
	Call	Out_AXDX_Dec	; And HEIGHT
	Mov	Al,'='
	Call	Out_SpxSp

	Mov	Ax,[AdjLine1]
	Mov	Dx,[AdjLine2]
	Push	Dx
	Push	Ax
	Call	Get_Height
	Push	Dx
	Push	Ax
	Call	G_Mul
	Mov	[DATACNT1],Ax
	Mov	[DATACNT2],Dx
	;;Call	Out_AXDX_Dec
	Call	Out_Bytes

;;;	Call	Out_Total

OUT_IMAGE:
;	OUTPUT THE DATA
	Cmp	[Verb_Val],5
	Jae	@F
	Jmp	Data_Dump
@@:
;	Could TRY for an IMAGE TYPE output
;	======= BUT THAT CAN WAIT =======
Data_Dump:
	Call	Get_Height

Next_Row:
	Push	Ax
	Push	Dx	; Keep HEIGHT
;	=============================
	Call	Show_Row
;	=============================
	Pop	Dx
	Pop	Ax
;	Are we FINISHED?
;	================
	Or	Ax,Ax
	Jz	@F		; Possibly, if DX also ZERO
	Dec	Ax		; else reduce by 1
	Jnz	Next_Row	; Inner cycle on AX value
	or	Dx,Dx		; Ax went ZERO - Any Dx?
	Jz	Dn_All_Rows	; NO, then ALL OVER
	Jmp	Next_Row	; There are MANY more to DO
@@:	or	Dx,Dx
	jz	Dn_All_Rows
	Dec	Dx		; Reduce 64K counter
	Dec	Ax		; and set Ax = 0xffff
	Jmp	Next_Row
Dn_All_Rows:
	Mov	Dx,Offset BM_DM3
	Mov	Cx,LBM_DM3
	Call	Out_Msg
	Call	Out_Total

	Call	Out_CrLf		
;;;;	JMP	End_BMP
	JMP	Do_Dump		; JUST DUMP ANY REMAINDER IN FILE
;================================================================

		======================================== */


// eof = D4Bmp.c
