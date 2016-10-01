
cmn = '''


common:
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rbp
	pushq %rdi
	pushq %rsi
	pushq %r9
	pushq %r8
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
'''

print cmn 
 

with open('src/entry.S', 'w') as otp:
	res = []
	res += ['\t.text\n', '\t.global entry_table\n', '\t.extern c_handler\n\n\n']
	for i in range(50):
		if i not in {8, 10, 11, 12, 13, 14, 17}:
			res += ['entry'+str(i)+':\n', '\tsubq $8, %rsp\n', '\tpushq $'+str(i)+'\n', \
			'\tjmp common\n']
		else:
			res += ['entry'+str(i)+':\n', '\tpushq $'+str(i)+'\n', \
			'\tjmp common\n']
	res += [cmn, '\nentry_table:\n']
	for i in range(50):
		res += ['\t.quad entry'+str(i)+'\n']
	res += ['\n']

	otp.write(''.join(res))

