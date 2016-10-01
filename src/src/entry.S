	.text
	.global entry_table
	.extern c_handler


entry0:
	subq $8, %rsp
	pushq $0
	jmp common
entry1:
	subq $8, %rsp
	pushq $1
	jmp common
entry2:
	subq $8, %rsp
	pushq $2
	jmp common
entry3:
	subq $8, %rsp
	pushq $3
	jmp common
entry4:
	subq $8, %rsp
	pushq $4
	jmp common
entry5:
	subq $8, %rsp
	pushq $5
	jmp common
entry6:
	subq $8, %rsp
	pushq $6
	jmp common
entry7:
	subq $8, %rsp
	pushq $7
	jmp common
entry8:
	pushq $8
	jmp common
entry9:
	subq $8, %rsp
	pushq $9
	jmp common
entry10:
	pushq $10
	jmp common
entry11:
	pushq $11
	jmp common
entry12:
	pushq $12
	jmp common
entry13:
	pushq $13
	jmp common
entry14:
	pushq $14
	jmp common
entry15:
	subq $8, %rsp
	pushq $15
	jmp common
entry16:
	subq $8, %rsp
	pushq $16
	jmp common
entry17:
	pushq $17
	jmp common
entry18:
	subq $8, %rsp
	pushq $18
	jmp common
entry19:
	subq $8, %rsp
	pushq $19
	jmp common
entry20:
	subq $8, %rsp
	pushq $20
	jmp common
entry21:
	subq $8, %rsp
	pushq $21
	jmp common
entry22:
	subq $8, %rsp
	pushq $22
	jmp common
entry23:
	subq $8, %rsp
	pushq $23
	jmp common
entry24:
	subq $8, %rsp
	pushq $24
	jmp common
entry25:
	subq $8, %rsp
	pushq $25
	jmp common
entry26:
	subq $8, %rsp
	pushq $26
	jmp common
entry27:
	subq $8, %rsp
	pushq $27
	jmp common
entry28:
	subq $8, %rsp
	pushq $28
	jmp common
entry29:
	subq $8, %rsp
	pushq $29
	jmp common
entry30:
	subq $8, %rsp
	pushq $30
	jmp common
entry31:
	subq $8, %rsp
	pushq $31
	jmp common
entry32:
	subq $8, %rsp
	pushq $32
	jmp common
entry33:
	subq $8, %rsp
	pushq $33
	jmp common
entry34:
	subq $8, %rsp
	pushq $34
	jmp common
entry35:
	subq $8, %rsp
	pushq $35
	jmp common
entry36:
	subq $8, %rsp
	pushq $36
	jmp common
entry37:
	subq $8, %rsp
	pushq $37
	jmp common
entry38:
	subq $8, %rsp
	pushq $38
	jmp common
entry39:
	subq $8, %rsp
	pushq $39
	jmp common
entry40:
	subq $8, %rsp
	pushq $40
	jmp common
entry41:
	subq $8, %rsp
	pushq $41
	jmp common
entry42:
	subq $8, %rsp
	pushq $42
	jmp common
entry43:
	subq $8, %rsp
	pushq $43
	jmp common
entry44:
	subq $8, %rsp
	pushq $44
	jmp common
entry45:
	subq $8, %rsp
	pushq $45
	jmp common
entry46:
	subq $8, %rsp
	pushq $46
	jmp common
entry47:
	subq $8, %rsp
	pushq $47
	jmp common
entry48:
	subq $8, %rsp
	pushq $48
	jmp common
entry49:
	subq $8, %rsp
	pushq $49
	jmp common



common:
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rbp
	pushq %rdi
	pushq %rsi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	movq %rsp, %rdi
	cld
	call c_handler
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rsi
	popq %rdi
	popq %rbp
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax
	addq $16, %rsp
	iretq

entry_table:
	.quad entry0
	.quad entry1
	.quad entry2
	.quad entry3
	.quad entry4
	.quad entry5
	.quad entry6
	.quad entry7
	.quad entry8
	.quad entry9
	.quad entry10
	.quad entry11
	.quad entry12
	.quad entry13
	.quad entry14
	.quad entry15
	.quad entry16
	.quad entry17
	.quad entry18
	.quad entry19
	.quad entry20
	.quad entry21
	.quad entry22
	.quad entry23
	.quad entry24
	.quad entry25
	.quad entry26
	.quad entry27
	.quad entry28
	.quad entry29
	.quad entry30
	.quad entry31
	.quad entry32
	.quad entry33
	.quad entry34
	.quad entry35
	.quad entry36
	.quad entry37
	.quad entry38
	.quad entry39
	.quad entry40
	.quad entry41
	.quad entry42
	.quad entry43
	.quad entry44
	.quad entry45
	.quad entry46
	.quad entry47
	.quad entry48
	.quad entry49

