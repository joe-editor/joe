PUBLIC coro_transfer

.586p
.model flat
.code
.safeseh coro_transfer ; a bald-faced lie.

alias <@coro_transfer@8> = <coro_transfer>

coro_transfer proc near
	push ebp
	push ebx
	push esi
	push edi

	; TIB
	assume fs:nothing
	push dword ptr fs:[0]
	push dword ptr fs:[4]
	push dword ptr fs:[8]

	mov dword ptr [ecx], esp
	mov esp, dword ptr [edx]

	; TIB
	pop dword ptr fs:[8]
	pop dword ptr fs:[4]
	pop dword ptr fs:[0]

	pop edi
	pop esi
	pop ebx
	pop ebp
	pop ecx
	jmp ecx
coro_transfer ENDP

END
