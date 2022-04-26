0000000000000000 <_start>:
   00: 00    02c000ef                jal     ra,2c #44 = <main>
   04: 04    00000893                addi    a7,zero,0
   08: 08    00000073                ecall

000000000000000c <print>:
   0c: 12    00050293                addi    t0,a0,0
   10: 16    00200893                addi    a7,zero,2
   14: 20    0002c503                lbu     a0,0(t0)
   18: 24    00050863                beq     a0,zero,28 #40 = <print+0x1c>
   1c: 28    00000073                ecall
   20: 32    00128293                addi    t0,t0,1
   24: 36    ff1ff06f                jal     zero,14 #20 = <print+0x8>
   28: 40    00008067                jalr    zero,0(ra)

000000000000002c <main>:
   2c: 44    f0010113                addi    sp,sp,-256
   30: 48    00113023                sd      ra,0(sp)
   34: 52    00000517                auipc   a0,0x0
   38: 56    01850513                addi    a0,a0,24 #76 = 4c <output>
   3c: 60    fd1ff0ef                jal     ra,0c #12 = <print>
   40: 64    00013083                ld      ra,0(sp)
   44: 68    10010113                addi    sp,sp,256
   48: 72    00008067                jalr    zero,0(ra)

000000000000004c <output>:
   4c: 76    6c6c6548                .word   0x6c6c6548
   50: 80    6f57206f                .word   0x6f57206f
   54: 84    00646c72                .word   0x00646c72