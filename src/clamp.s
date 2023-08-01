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
	cmp	DWORD [rdi], 255
	jbe	.NEXT
	mov	DWORD [rdi], 255
.NEXT:
	add	rdi, 4
	cmp	rdi, rax
	jne	.LOOP
.RET:
	ret
