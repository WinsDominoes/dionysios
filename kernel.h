#pragma once

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
