global cmov_clamp
global  jmp_clamp
global  opt_clamp

section .text

; clamp(uint32_t * data, uint64_t count)
;                 rdi             rsi

cmov_clamp:
	test	rsi, rsi
	jz	.RET
	mov	edx, 255
.LOOP:
	mov	eax, DWORD [rdi + 4 * rsi - 4]
	cmp	eax, edx
	cmova	eax, edx
	mov	DWORD [rdi + 4 * rsi - 4], eax
	sub	rsi, 1
	jne	.LOOP
.RET:
	ret

jmp_clamp:
	test	rsi, rsi
	jz	.RET
	mov	edx, 255
.LOOP:
	mov	eax, DWORD [rdi + 4 * rsi - 4]
	cmp	eax, edx
	jbe	.NEXT
	mov	DWORD [rdi + 4 * rsi - 4], edx
.NEXT:
	sub	rsi, 1
	jne	.LOOP
.RET:
	ret

opt_clamp:
	test	rsi, rsi
	jz	.RET
	xor 	ecx, ecx
	mov 	edx, 255
.LOOP:
	mov	eax, DWORD [rdi + 4 * rcx]
	cmp	eax, edx
	jbe	.NEXT
	mov	DWORD [rdi + 4 * rcx], edx
.NEXT:
	add	rcx, 1
	cmp	rcx, rsi
	jne	.LOOP
.RET:
	ret
