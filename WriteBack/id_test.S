.section .text
.global _start
_start:
lb t0, -8(sp) # 0xff810283 0
lw t1, 16(a0) # 0x01052303 4
sh t1, -8(sp) # 0xfe611c23 8
sd t0, 16(a0) # 0x00553823 12
bne t0, t1, there # 0x00629463 16
add s0, s1, s2 # 0x01248433 20
there:
call there2 # 0x008000ef 24
addi s0, ra, -16 # 0xff008413 28
there2:
lui  s0, 0xbeef # 0x0beef437 32
mulw a0, a1, a2 # 0x02c5853b 36
addiw a0, a1, 128 # 0x0805851b 40
ecall # 0x00000073 44
