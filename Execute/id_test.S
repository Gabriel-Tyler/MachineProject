.section .text
.global _start
_start:
lb t0, -8(sp)
lw t1, 16(a0)
sh t1, -8(sp)
sd t0, 16(a0)
bne t0, t1, there
add s0, s1, s2
there:
call there2
addi s0, ra, -16
there2:
lui  s0, 0xbeef
mulw a0, a1, a2
addiw a0, a1, 128
ecall
