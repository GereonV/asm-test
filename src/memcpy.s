global movs_memcpy
global rep_movs_memcpy

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
