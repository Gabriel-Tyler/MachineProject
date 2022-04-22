.section .text
.option norvc
.global main
.global _start
_start:
	call	main
	li	a7, 0
	ecall



print:
	mv	t0, a0
	li	a7, 2
1:
	lbu	a0, (t0)
	beqz	a0, 1f
	ecall
	addi	t0, t0, 1
	j	1b
1:
	ret


main:
	addi	sp, sp, -256
	sd	ra, 0(sp)
	la	a0, output
	call	print
	ld	ra, 0(sp)
	addi	sp, sp, 256
	ret


output: .asciz "Hello World"
