#include "user.h"

extern char __stack_top[];

__attribute__((noreturn)) void exit(void) 
{
	for (;;);				// Infinite loop, duh
}

void putchar(char ch)
{
	// TODO
}

// Execution of application starts here, the start function. 
// (check 'em kernel boot process - very similar innit)
__attribute__((section(".text.start")))
__attribute__((naked))
void start(void)
{
	__asm__ __volatile__(
		"mv sp, %[stack_top] \n"			// copy %[stack_top] -> sp (stack pointer)
		"call main			 \n"			// calls the app main function
		"call exit			 \n"			// prepare for terminating app
		:: [stack_top] "r" (__stack_top)
		
		// diff from kernel init process, we do not clear .bss section w/ zero
		// bc kernel makes sure of setting to zero w/ alloc_pages function.
	);
}
