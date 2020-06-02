.section ".text"

.global memzero
.globl memcpy

memzero:
	str     xzr,    [x0],   #8
	subs    x1,     x1,     #8
	b.gt    memzero
	ret

memcpy:
	ldr     x3,     [x1],   #8
	str     x3,     [x0],   #8
	subs    x2,     x2,     #8
	b.gt    memcpy
	ret
