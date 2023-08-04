global simple_clamp
global   cmov_clamp
global    bit_clamp
global    opt_clamp

%use smartalign
alignmode p6
default rel

section .text

; clamp(uint32_t * data, uint64_t count)
;                   rdi             rsi

	align	32
simple_clamp:
	test	rsi, rsi
	jz	.RET
	lea	rax, [rdi + 4 * rsi]
.LOOP:	; no alignment sice loop is within 32-byte alignment
	cmp	DWORD [rdi], 255
	jbe	.NEXT
	mov	DWORD [rdi], 255
.NEXT:
	add	rdi, 4
	cmp	rdi, rax
	jne	.LOOP
.RET:
	ret

	align	32
cmov_clamp:
	test	rsi, rsi
	jz	.RET
	lea	rax, [rdi + 4 * rsi]
	mov	edx, 255
.LOOP:	; no alignment sice loop is within 32-byte alignment
	mov	ecx, [rdi]
	cmp	ecx, edx
	cmova	ecx, edx
	mov	[rdi], ecx
	add	rdi, 4
	cmp	rdi, rax
	jne	.LOOP
.RET:
	ret

	align	32
bit_clamp:
	test	rsi, rsi
	jz	.RET
	lea	rax, [rdi + 4 * rsi]
	mov	edx, 255
	align	32
.LOOP:
	mov	ecx, [rdi]
	sub	ecx, edx
	sbb	esi, esi	; esi = ecx < edx ? -1 : 0
	and	esi, ecx	; esi = ecx < edx ? ecx-edx : 0
	add	esi, edx	; esi = ecx < edx ? ecx : edx
	mov	[rdi], esi
	add	rdi, 4
	cmp	rdi, rax
	jne	.LOOP
.RET:
	ret

	align	32
opt_clamp:
	cmp	rsi, 8
	jb	simple_clamp
	vmovdqa	ymm0, [data_255]
	sub	rsi, 8
	align	32
.LOOP:
	vpminud	ymm1, ymm0, [rdi]
	vmovdqu	[rdi], ymm1
	add	rdi, 32
	sub	rsi, 8
	jnc	.LOOP
	vpminud	ymm1, ymm0, [rdi + 4 * rsi]
	vmovdqu	[rdi + 4 * rsi], ymm1
	vzeroupper
	ret

section .data

	align	32
data_255:
	times 8		dd	255
