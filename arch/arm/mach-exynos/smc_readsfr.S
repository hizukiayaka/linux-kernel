#include <mach/smc.h>

	.data
	.align
	.global exynos_smc_readsfr

exynos_smc_readsfr:
	stmfd   sp!, {lr,r4}
	mov	r4, r0
	mov     r3, #0
	mov     r2, #0
	ldr     r0, =SMC_CMD_REG
	smc     0
	str	r2, [r4]
	ldmfd   sp!, {pc,r4}
