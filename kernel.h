#pragma once
#include "common.h"

/* Macro for Kernel panics

Win Pattanaphol (commentary) - 6th of April 2026

	Okay but what are kernel panics? A kernel panic happens when the kernel
gets into an unrecoverable error. It's like the BSOD, y'know...

- printf(): Prints where it panicked (lolz)
- while(1) {}: an infinite loop to halt processing
- while(0): means loop is executed once

Reason why it is a macro: "correctly display the 
- "source file name (__FILE__)" - where the __FILE__ is called (so we know what happened)
- "line number (__LINE__)" - where the __LINE__ is called
if defined as function, __FILE__ and __LINE__ would show filename and line num where PANIC is defined not where it was called

The printf would output something like PANIC: kernel.c:46: booted!
meaning the PANIC HAPPENED FROM line 46 IN kernel.c

REMEMBER: 
- Macros -> Pre-processed 
- Function -> Compiled

##__VA_ARGS -> compiler extension for accepting a variable number of args - removes
the preceding comma when the var args are empty!
*/

/* 
	What is trap_frame for?
	- It is a struct for the  "program state" that is saved in "kernel_entry"
*/	
struct trap_frame
{
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

/* 
	What are Control/Status Register
	- Control and Status Register (CSR) are auxiliary registers in many CPUs 
	and many microcontrollers that are used for reading status and changing configuration...

	- Both CPUs and I/O devices have CSRs. Typical examples include RISC-V CPU 
	which has a set of registers to handle interrupts

	Check: https://en.wikipedia.org/wiki/Control/Status_Register
*/

// For reading Control/Status Registers (CSRs)
#define READ_CSR(reg)															\
	({																			\
		unsigned long __tmp;													\
		__asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));					\
		__tmp;																	\
	})

// For WRITING Control/Status Registers (CSRs)
#define WRITE_CSR(reg, value)                                                   \
	do {																		\
		uint32_t __tmp = (value);												\
		__asm __volatile__("csrw " #reg ", %0" ::"r"(__tmp));					\
	} while (0)

#define PANIC(fmt, ...)															\
	do {																		\
		printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); 	\
		while (1) {} 															\
	} while (0) 
	
/* */

struct sbiret
{
    long error;
    long value;
};
