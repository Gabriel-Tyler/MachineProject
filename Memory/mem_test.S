.section .text
.global _start
_start:
	ld	t0, 0(zero)
	sb	zero, 0(zero)
	ld	t0, 0(zero)
	ld	t0, 4(zero)
	ld	t0, 8(zero)
	sw	zero, 4(zero)
	ld	t0, 4(zero)
	sh	zero, 8(zero)
	ld	t0, 8(zero)
	lb	t0, 8(zero)
	lh	t0, 8(zero)
	lw	t0, 8(zero)
	lbu	t0, 12(zero)
	lhu	t0, 16(zero)
	lwu	t0, 24(zero)

