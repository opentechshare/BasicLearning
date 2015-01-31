
.globl _start
_start:

/* 硬件相关的设置 */
	/* Peri port setup */
	ldr r0, =0x70000000
	orr r0, r0, #0x13
	mcr p15,0,r0,c15,c2,4       @ 256M(0x70000000-0x7fffffff)
    
/* 关看门狗 */
/* 往WTCON(0x7E004000)写0 */
	ldr r0, =0x7E004000
	mov r1, #0
	str r1, [r0]

/* 设置GPMCON让GPM3作为输出引脚 */
	ldr r1, =0x7F008820
	mov r0, #0x1000
	str r0, [r1]

/* 设置GPMDAT让GPM3输出0 */
	ldr r1, =0x7F008824
	mov r0, #0
	str r0, [r1]

halt:
	b halt
