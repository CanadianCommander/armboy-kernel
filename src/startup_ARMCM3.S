/* File: startup_ARMCM3.S
 * Purpose: startup file for Cortex-M3 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V2.0
 * Date: 16 August 2013
 *
/* Copyright (c) 2011 - 2013 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
	.syntax	unified
	.arch	armv7-m

	.section .stack
	.align	3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0xc00
#endif
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, 0x20087FFF

	.section .heap
	.align	3
#ifdef __HEAP_SIZE
	.equ	Heap_Size, __HEAP_SIZE
#else
	.equ	Heap_Size, 0x17FFF
#endif
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.if	Heap_Size
	.space	Heap_Size
	.endif
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

	.section .isr_vector
	.align	2
	.globl	__isr_vector
__isr_vector:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	defaultVector           /* NMI Handler */
	.long	defaultVector     /* Hard Fault Handler */
	.long	defaultVector     /* MPU Fault Handler */
	.long	defaultVector      /* Bus Fault Handler */
	.long	defaultVector    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_IRQ           /* SVCall Handler */
	.long	defaultVector      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	defaultVector        /* PendSV Handler */
	.long	defaultVector       /* SysTick Handler */

	/* External interrupts */
  .long defaultVector /**<  0 SAM3X8E Supply Controller (SUPC) */
  .long defaultVector /**<  1 SAM3X8E Reset Controller (RSTC) */
  .long defaultVector /**<  2 SAM3X8E Real Time Clock (RTC) */
  .long defaultVector /**<  3 SAM3X8E Real Time Timer (RTT) */
  .long defaultVector /**<  4 SAM3X8E Watchdog Timer (WDT) */
  .long defaultVector /**<  5 SAM3X8E Power Management Controller (PMC) */
  .long defaultVector /**<  6 SAM3X8E Enhanced Flash Controller 0 (EFC0) */
  .long defaultVector /**<  7 SAM3X8E Enhanced Flash Controller 1 (EFC1) */
  .long UART_ISR /**<  8 SAM3X8E Universal Asynchronous Receiver Transceiver (UART) */
  .long defaultVector /**<  9 SAM3X8E Static Memory Controller (SMC) */
  .long defaultVector /**< 11 SAM3X8E Parallel I/O Controller A, (PIOA) */
  .long defaultVector /**< 12 SAM3X8E Parallel I/O Controller B (PIOB) */
  .long defaultVector /**< 13 SAM3X8E Parallel I/O Controller C (PIOC) */
  .long defaultVector /**< 14 SAM3X8E Parallel I/O Controller D (PIOD) */
  .long defaultVector /**< 17 SAM3X8E USART 0 (USART0) */
  .long defaultVector /**< 18 SAM3X8E USART 1 (USART1) */
  .long defaultVector /**< 19 SAM3X8E USART 2 (USART2) */
  .long defaultVector /**< 20 SAM3X8E USART 3 (USART3) */
  .long defaultVector /**< 21 SAM3X8E Multimedia Card Interface (HSMCI) */
  .long defaultVector /**< 22 SAM3X8E Two-Wire Interface 0 (TWI0) */
  .long defaultVector /**< 23 SAM3X8E Two-Wire Interface 1 (TWI1) */
  .long defaultVector /**< 24 SAM3X8E Serial Peripheral Interface (SPI0) */
  .long defaultVector /**< 26 SAM3X8E Synchronous Serial Controller (SSC) */
  .long defaultVector /**< 27 SAM3X8E Timer Counter 0 (TC0) */
  .long defaultVector /**< 28 SAM3X8E Timer Counter 1 (TC1) */
  .long defaultVector /**< 29 SAM3X8E Timer Counter 2 (TC2) */
  .long defaultVector /**< 30 SAM3X8E Timer Counter 3 (TC3) */
  .long defaultVector /**< 31 SAM3X8E Timer Counter 4 (TC4) */
  .long defaultVector /**< 32 SAM3X8E Timer Counter 5 (TC5) */
  .long defaultVector /**< 33 SAM3X8E Timer Counter 6 (TC6) */
  .long defaultVector /**< 34 SAM3X8E Timer Counter 7 (TC7) */
  .long defaultVector /**< 35 SAM3X8E Timer Counter 8 (TC8) */
  .long defaultVector /**< 36 SAM3X8E Pulse Width Modulation Controller (PWM) */
  .long defaultVector /**< 37 SAM3X8E ADC Controller (ADC) */
  .long defaultVector /**< 38 SAM3X8E DAC Controller (DACC) */
  .long defaultVector /**< 39 SAM3X8E DMA Controller (DMAC) */
  .long defaultVector /**< 40 SAM3X8E USB OTG High Speed (UOTGHS) */
  .long defaultVector /**< 41 SAM3X8E True Random Number Generator (TRNG) */
  .long defaultVector /**< 42 SAM3X8E Ethernet MAC (EMAC) */
  .long defaultVector /**< 43 SAM3X8E CAN Controller 0 (CAN0) */
  .long defaultVector /**< 44 SAM3X8E CAN Controller 1 (CAN1) */

	.size	__isr_vector, . - __isr_vector

	.text
	.thumb
	.thumb_func
	.align	2
	.globl	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r4, =__copy_table_start__
	ldr	r5, =__copy_table_end__

.L_loop0:
	cmp	r4, r5
	bge	.L_loop0_done
	ldr	r1, [r4]
	ldr	r2, [r4, #4]
	ldr	r3, [r4, #8]

.L_loop0_0:
	subs	r3, #4
	ittt	ge
	ldrge	r0, [r1, r3]
	strge	r0, [r2, r3]
	bge	.L_loop0_0

	adds	r4, #12
	b	.L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__etext
	ldr	r2, =__data_start__
	ldr	r3, =__data_end__

.L_loop1:
	cmp	r2, r3
	ittt	lt
	ldrlt	r0, [r1], #4
	strlt	r0, [r2], #4
	blt	.L_loop1
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
	ldr	r3, =__zero_table_start__
	ldr	r4, =__zero_table_end__

.L_loop2:
	cmp	r3, r4
	bge	.L_loop2_done
	ldr	r1, [r3]
	ldr	r2, [r3, #4]
	movs	r0, 0

.L_loop2_0:
	subs	r2, #4
	itt	ge
	strge	r0, [r1, r2]
	bge	.L_loop2_0

	adds	r3, #8
	b	.L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__bss_start__
	ldr	r2, =__bss_end__

	movs	r0, 0
.L_loop3:
	cmp	r1, r2
	itt	lt
	strlt	r0, [r1], #4
	blt	.L_loop3
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
	bl	SystemInit
#endif

#ifndef __START
#define __START _start
#endif
	bl	__START

	.pool
	.size	Reset_Handler, . - Reset_Handler

	.align	1
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size	Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_irq_handler	NMI_Handler
	def_irq_handler	HardFault_Handler
	def_irq_handler	MemManage_Handler
	def_irq_handler	BusFault_Handler
	def_irq_handler	UsageFault_Handler
	def_irq_handler	SVC_Handler
	def_irq_handler	DebugMon_Handler
	def_irq_handler	PendSV_Handler
	def_irq_handler	SysTick_Handler
	def_irq_handler	DEF_IRQHandler

	.end
