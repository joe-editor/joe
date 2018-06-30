PUBLIC coro_transfer

.code

coro_transfer proc
	mov rax, rsp
	sub rsp, 176			; two extra qwords for safe realignment
	sub rax, 160			; movaps will AV if unaligned
	and rax, -16			; align RAX
	movaps [rax], xmm6
	movaps [rax+16], xmm7
	movaps [rax+32], xmm8
	movaps [rax+48], xmm9
	movaps [rax+64], xmm10
	movaps [rax+80], xmm11
	movaps [rax+96], xmm12
	movaps [rax+112], xmm13
	movaps [rax+128], xmm14
	movaps [rax+144], xmm15
	push rsi
	push rdi
	push rbp
	push rbx
	push r12
	push r13
	push r14
	push r15
	push QWORD PTR gs:[0h]
	push QWORD PTR gs:[8h]
	push QWORD PTR gs:[10h]
	mov QWORD PTR [rcx], rsp
	mov rsp, QWORD PTR [rdx]
	pop QWORD PTR gs:[10h]
	pop QWORD PTR gs:[8h]
	pop QWORD PTR gs:[0h]
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	pop rdi
	pop rsi
	add rsp, 176
	mov rax, rsp
	sub rax, 160			; movaps will AV if unaligned
	and rax, -16			; align RAX
	movaps xmm6, [rax]
	movaps xmm7, [rax+16]
	movaps xmm8, [rax+32]
	movaps xmm9, [rax+48]
	movaps xmm10, [rax+64]
	movaps xmm11, [rax+80]
	movaps xmm12, [rax+96]
	movaps xmm13, [rax+112]
	movaps xmm14, [rax+128]
	movaps xmm15, [rax+144]
	pop rcx
	jmp rcx

coro_transfer	endp

end