;--------------------------------------------------------
; File Created by SDCC : FreeWare ANSI-C Compiler
; Version 2.3.1 Thu Jul 21 00:26:09 2016

;--------------------------------------------------------
	.module lcc12040
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; special function bits 
;--------------------------------------------------------
;--------------------------------------------------------
; internal ram data
;--------------------------------------------------------
	.area _DATA
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
	.area _OVERLAY
;--------------------------------------------------------
; indirectly addressable internal ram data
;--------------------------------------------------------
	.area _ISEG
;--------------------------------------------------------
; bit data
;--------------------------------------------------------
	.area _BSEG
;--------------------------------------------------------
; external ram data
;--------------------------------------------------------
	.area _XSEG
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _CODE
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;	infinit_store.c 6
;	genLabel
;	genFunction
;	---------------------------------
; Function main
; ---------------------------------
___main_start:
_main:
	lda	sp,-2(sp)
;	infinit_store.c 9
;	genAssign
;	AOP_STK for _main_ptr_1_1
	lda	hl,0(sp)
	ld	(hl),#0x31
	inc	hl
	ld	(hl),#0x30
;	infinit_store.c 10
;	genAssign
	ld	bc,#0x7117
;	infinit_store.c 12
;	genLabel
00102$:
;	infinit_store.c 13
;	genAssign (pointer)
;	AOP_STK for _main_ptr_1_1
	lda	hl,0(sp)
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	a,#0x66
	ld	(de),a
;	infinit_store.c 14
;	genAssign (pointer)
	ld	a,#0x99
	ld	(bc),a
;	genGoto
	jp	00102$
;	genLabel
00104$:
;	genEndFunction
	lda	sp,2(sp)
	ret
___main_end:
	.area _CODE
