global movs_memcpy
global rep_movs_memcpy
global avx_memcpy
global stream_memcpy
global opt_memcpy

%use smartalign
alignmode p6
default rel

section .text

; memcpy(void * dst, void const * src, uint64_t bytes)
;               rdi         rsi                 rdx

	align	32
movs_memcpy:
	mov	rax, rdx
	and	edx, 0b111
	shr	rax, 3
	je	.FEW
.LOOP:
	movsq
	sub	rax, 1
	jnz	.LOOP
	mov	rax, [rsi + rdx - 8]
	mov	[rdi + rdx - 8], rax
	ret
.FEW:
	mov	ecx, edx
	rep movsb
	ret

	align	32
rep_movs_memcpy:
	mov	rcx, rdx
	rep movsb
	ret

	align	32
avx_memcpy:
	mov	rcx, rdx
	and	rdx, ~(128-1)
	jz	.FEW
	xor	eax, eax
	and	rcx, (128-1)
.LOOP:
	vmovdqu	ymm0, [rsi + rax]
	vmovdqu	ymm1, [rsi + rax + 32]
	vmovdqu	ymm2, [rsi + rax + 64]
	vmovdqu	ymm3, [rsi + rax + 96]
	vmovdqu	[rdi + rax], ymm0
	vmovdqu	[rdi + rax + 32], ymm1
	vmovdqu	[rdi + rax + 64], ymm2
	vmovdqu	[rdi + rax + 96], ymm3
	add	rax, 128
	cmp	rax, rdx
	jne	.LOOP
	lea	rax, [rax + rcx - 128]
	vmovdqu	ymm0, [rsi + rax]
	vmovdqu	ymm1, [rsi + rax + 32]
	vmovdqu	ymm2, [rsi + rax + 64]
	vmovdqu	ymm3, [rsi + rax + 96]
	vmovdqu	[rdi + rax], ymm0
	vmovdqu	[rdi + rax + 32], ymm1
	vmovdqu	[rdi + rax + 64], ymm2
	vmovdqu	[rdi + rax + 96], ymm3
	vzeroupper
	ret
.FEW:
	rep movsb
	ret

	align	32
stream_memcpy:
	cmp	rdx, 32
	jb	.FEW
	vmovdqu	ymm0, [rsi]
	vmovdqu	[rdi], ymm0
	je	.RET
	mov	eax, edi
	and	eax, 32 - 1
	lea	rdx, [rdx + rax - 32]
	neg	rax
	lea	rdi, [rdi + rax + 32]
	lea	rsi, [rsi + rax + 32]
	sub	rdx, 32
	jc	.LEFT
	xor	eax, eax
.LOOP:
	vmovdqu	ymm0, [rsi + rax]
	vmovntdq	[rdi + rax], ymm0
	add	rax, 32
	sub	rdx, 32
	jnc	.LOOP
	add	rdi, rax
	add	rsi, rax
.LEFT:
	vmovdqu	ymm0, [rsi + rdx]
	vmovdqu	[rdi + rdx], ymm0
	sfence
	vzeroupper
.RET:
	ret
.FEW:
	mov	ecx, edx
	rep movsb
	ret

	align	32
opt_memcpy:
	cmp	rdx, 96
	jb	.FEW
	cmp	rdx, (1 << 20) + (1 << 10)
	jae	.MANY
	xor	eax, eax
	sub	rdx, 96
	align	32
.LOOP:
	vmovdqu	ymm0, [rsi + rax]
	vmovdqu	ymm1, [rsi + rax + 32]
	vmovdqu	ymm2, [rsi + rax + 64]
	vmovdqu	[rdi + rax], ymm0
	vmovdqu	[rdi + rax + 32], ymm1
	vmovdqu	[rdi + rax + 64], ymm2
	add	rax, 96
	sub	rdx, 96
	jnc	.LOOP
	add	rax, rdx
	vmovdqu	ymm0, [rsi + rax]
	vmovdqu	ymm1, [rsi + rax + 32]
	vmovdqu	ymm2, [rsi + rax + 64]
	vmovdqu	[rdi + rax], ymm0
	vmovdqu	[rdi + rax + 32], ymm1
	vmovdqu	[rdi + rax + 64], ymm2
	vzeroupper
	ret
.FEW:
	mov	ecx, edx
	rep movsb
	ret
.MANY:
	vmovdqu	ymm0, [rsi]
	vmovdqu	[rdi], ymm0
	mov	eax, edi
	and	eax, 32 - 1
	lea	rdx, [rdx + rax - 32 - 128]
	neg	rax
	lea	rdi, [rdi + rax + 32]
	lea	rsi, [rsi + rax + 32]
	xor	eax, eax
	mov	ecx, 128
.STREAM:
	prefetcht0	[rsi + rax + 128]
	vmovdqu	ymm0, [rsi + rax]
	vmovdqu	ymm1, [rsi + rax + 32]
	vmovntdq	[rdi + rax], ymm0
	vmovdqu	ymm2, [rsi + rax + 64]
	vmovntdq	[rdi + rax + 32], ymm1
	prefetcht0	[rsi + rax + 192]
	vmovdqu	ymm3, [rsi + rax + 96]
	vmovntdq	[rdi + rax + 64], ymm2
	vmovntdq	[rdi + rax + 96], ymm3
	add	rax, 128
	cmp	rax, rdx
	jb	.STREAM
	sfence
	vmovdqu	ymm0, [rsi + rdx]
	vmovdqu	ymm1, [rsi + rdx + 32]
	vmovdqu	ymm2, [rsi + rdx + 64]
	vmovdqu	ymm3, [rsi + rdx + 96]
	vmovdqu	[rdi + rdx], ymm0
	vmovdqu	[rdi + rdx + 32], ymm1
	vmovdqu	[rdi + rdx + 64], ymm2
	vmovdqu	[rdi + rdx + 96], ymm3
	vzeroupper
	ret
