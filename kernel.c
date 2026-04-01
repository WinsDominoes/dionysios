#include "kernel.h"
#include "common.h"

// in K&R C book, Section A8.9 - typedef is basically an alias for a type
typedef unsigned char uint8_t; 
typedef unsigned int uint32_t;
typedef uint32_t size_t; 

extern char __bss[], __bss_end[], __stack_top[];    // we want to get the "address" from the symbols!

// memset: writes c into buf with the size n)
// n decreases because n decrease -> stack grows
// as the stack grows, we write c into that buffer
// which is referenced by *p pointer.
void *memset(void *buf, char c, size_t n)
{
    uint8_t *p = (uint8_t *) buf;
    while (n--)
    {
        *p++ = c;
    } 
    return buf;
}

/*  
    sbi_call: Function that calls the Supervisor Binary Interface to use its functions
    SBI: Supervisor Binary Interface
    Terms I should know:
    - FID: SBI function ID
    - EID: SBI extension ID

    a0-a7 -> function arguments : corresponds to x10-x17 registers of RISC-V
*/
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid)
{
    // "register" asks the compiler to place values in specified registers
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch)
{
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1); // console putchar
}

void kernel_main(void)
{
    // memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);  // initializes / sets the bss section to zero

    printf("\n\nHello %s\n", "World!");
    printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);

    for (;;)
    {
        __asm__ __volatile__("wfi");
    }
}

__attribute__((section(".text.boot")))      // sets the function at the .text.boot section, which is where OpenSBI jumps to (0x80200000)
__attribute__((naked))                      // tells compiler -> no generating unnecessary code before and after function
void boot(void)
{

    // NOTE: Stack GROWS towards zero (0)
    //       when we use the stack, it *decrements*
    //       so the end address must be set (the begin point when we use the stack)

    __asm__ __volatile(
        "mv sp, %[stack_top]\n"             // Set the stack pointer (sp) to end address
        "j kernel_main \n"                  // Jump to kernel_main function
        :
        : [stack_top] "r" (__stack_top)
    );
}