global simple_clamp
global    opt_clamp

default rel
; %define nop2	db	66h, 90h,
; %define nop3	db	0Fh, 1Fh, 00h,
; %define nop4	db	0Fh, 1Fh, 40h, 00h,
; %define nop5	db	0Fh, 1Fh, 44h, 00h, 00h,
; %define nop6	db	66h, 0Fh, 1Fh, 44h, 00h, 00h,
; %define nop7	db	0Fh, 1Fh, 80h, 00h, 00h, 00h, 00h,
; %define nop8	db	0Fh, 1Fh, 84h, 00h, 00h, 00h, 00h, 00h,
; %define nop9	db	66h, 0Fh, 1Fh, 84h, 00h, 00h, 00h, 00h, 00h,

section .text

; clamp(uint32_t * data, uint64_t count)
;                   rdi             rsi

simple_clamp:
	test	rsi, rsi
	jz	.RET
	lea	rax, [rdi + 4 * rsi]
	align	16
.LOOP:
	and	DWORD [rdi], 255
	add	rdi, 4
	cmp	rdi, rax
	jne	.LOOP
.RET:
	ret

	align	16
opt_clamp:
	; jump if too small
	cmp	rsi, 8
	jb	simple_clamp

	; setup constants
	mov	ecx, 8
	mov	edx, 32
	vmovdqa	ymm1, [and_mask]

	; unaligned first packed clamp
	vmovdqu	ymm0, [rdi]
	vpand	ymm0, ymm0, ymm1
	vmovdqu	[rdi], ymm0

	; align rdi to next 32 bytes
	; subtract from count in rsi accordingly
	; rsi -= (32-rdi%32)/4
	; rdi += 32-(rdi%32)
	mov	rax, rdi
	and	rdi, ~31
	add	rdi, rdx
	sub	rax, rdi
	sar	rax, 2
	add	rsi, rax

	; no aligned moves possible
	cmp	rsi, rcx
	jb	.LAST

	align	16
.LOOP:
	; aligned packed clamp
	vmovdqa	ymm0, [rdi]
	vpand	ymm0, ymm0, ymm1
	vmovdqa	[rdi], ymm0

	sub	rsi, rcx
	add	rdi, rdx
	cmp	rsi, rcx
	jae	.LOOP
.LAST:
	; unaligned packed clamp
	sub	rsi, rcx
	vmovdqu	ymm0, [rdi + 4 * rsi]
	vpand	ymm0, ymm0, ymm1
	vmovdqu	[rdi + 4 * rsi], ymm0
	vzeroupper
	ret


section .data

	align	32
and_mask:
	dd	255, 255, 255, 255, 255, 255, 255, 255
