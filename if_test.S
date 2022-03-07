.section .text
.global _start
.option norvc
_start:
	lwu a0, -32(sp)
	addi sp, sp, -8
	add a0, a1, a2
	sub a2, a3, a4
	sll t1, s4, tp
	li a2, -8
	li gp, 2
	sra t0, a2, gp
	srl t1, a2, gp
	li t1, -8
	li t2, 8
	slt a6, t1, t2
	sltu a5, t1, t2
	addi sp, sp, 8
	li a7, 0
	li a0, 255
	ecall
