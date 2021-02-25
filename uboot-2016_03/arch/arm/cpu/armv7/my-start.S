/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * Do important init only if we don't start from memory!
 * Setup memory and board specific bits prior to relocation.
 * Relocate armboot to ram. Setup stack.
 *
 *************************************************************************/

.text             //代码段  
.global _start    //全局变量  
_start:  


/* 1. 关看门狗 */  
  
/* 2. 设置时钟 */  
  
  
    /* 启动ICACHE */  
    cpsid i                     /* 关闭全局中断 */

    /* 关闭MMU 
     * 采取读-改-写的方式。
     */
    mrc     p15, 0, r0, c1, c0, 0     /* 读取CP15的C1寄存器到R0中                           */
    bic     r0,  r0, #0x1             /* 清除C1寄存器的bit0(M位)， 关闭MMU                    */
    mcr     p15, 0, r0, c1, c0, 0     /* 将r0寄存器中的值写入到CP15的C1寄存器中                  */

  
  
/* 3. 初始化SDRAM */  

1:  
  
/* 4. 重定位 : 把bootloader本身的代码从flash复制到它的链接地址去 */  
  
    /* 不管是NandFlash启动还是NorFlash启动，内核都存在NandFlash上，所以都得先初始化NandFlash */  
  
    /*汇编向C函数传递参数（参数1：r0、参数2：r1、参数3：r2）*/  
  
    /*拷贝完后，将未初始化的全局和初始化为零的全局内存空间清零*/      
      
/* 5. 执行main */  
    ldr lr, =halt  
    ldr pc, =main  
halt:  
    b halt  



.globl	reset

reset:
	/* Allow the board to save important registers */
	/*
	 * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
	 * except if in HYP mode already
	 */
	mrs	r0, cpsr			@ read cpsr register to r0 (r0=cpsr)
	and	r1, r0, #0x1f		@ mask mode bits
	teq	r1, #0x1a			@ test for HYP mode
	bicne	r0, r0, #0x1f	@ clear all mode bits   不等于 0-5位进行清零
	orrne	r0, r0, #0x13	@ set SVC mode			r0 低5位 进行或运算
	
	orr	r0, r0, #0xc0		@ disable FIQ and IRQ   
	msr	cpsr,r0				@ 设置 cpsr register FIQ IRQ

/*
 * Setup vector:
 * (OMAP4 spl TEXT_BASE is not 32 byte aligned.
 * Continue to use ROM code vector only in OMAP4 spl)
 */

	/* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
	mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTLR Register
	bic	r0, #CR_V		@ V = 0
	mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTLR Register

	/* Set vector address in CP15 VBAR register */
	ldr	r0, =_start
	mcr	p15, 0, r0, c12, c0, 0	@Set VBAR

	/* the mask ROM code should have PLL and others stable */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_cp15
	bl	cpu_init_crit
#endif

	bl	_main

/*------------------------------------------------------------------------------*/


/*
 * If I-cache is enabled invalidate it
 */
	mcr     p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr     p15, 0, r0, c7, c10, 4	@ DSB
	mcr     p15, 0, r0, c7, c5, 4	@ ISB

	bx	lr

/*************************************************************************
 *
 * void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3)
 *	__attribute__((weak));
 *
 * Stack pointer is not yet initialized at this moment
 * Don't save anything to stack even if compiled with -O0
 *
 *************************************************************************/
//ENTRY(save_boot_params)
//	b	save_boot_params_ret		@ back to my caller 
//ENDPROC(save_boot_params)
//	.weak	save_boot_params

/*************************************************************************
 *
 * cpu_init_cp15
 *
 * Setup CP15 registers (cache, MMU, TLBs). The I-cache is turned on unless
 * CONFIG_SYS_ICACHE_OFF is defined.
 *
 *************************************************************************/
ENTRY(cpu_init_cp15)
	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	mcr     p15, 0, r0, c7, c10, 4	@ DSB
	mcr     p15, 0, r0, c7, c5, 4	@ ISB

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB

	bic	r0, r0, #0x00001000	@ clear bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0 @ write control register and disenable MMU


	mov	r5, lr			@ Store my Caller
	mrc	p15, 0, r1, c0, c0, 0	@ r1 has Read Main ID Register (MIDR)
	mov	r3, r1, lsr #20		@ get variant field
	and	r3, r3, #0xf		@ r3 has CPU variant
	and	r4, r1, #0xf		@ r4 has CPU revision
	mov	r2, r3, lsl #4		@ shift variant field for combined value
	orr	r2, r4, r2		@ r2 has combined CPU variant + revision

	mov	pc, r5			@ back to my caller
ENDPROC(cpu_init_cp15)

/*************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************/

	/*
	 * Jump to board specific initialization...
	 * The Mask ROM will have already initialized
	 * basic memory. Go here to bump up clock rate and handle
	 * wake up conditions.
	 */
	b	lowlevel_init		@ go setup pll,mux,memory
	
