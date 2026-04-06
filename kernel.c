#include "kernel.h"
#include "common.h"

// in K&R C book, Section A8.9 - typedef is basically an alias for a type
typedef unsigned char uint8_t; 
typedef unsigned int uint32_t;
typedef uint32_t size_t; 

extern char __bss[], __bss_end[], __stack_top[];    // we want to get the "address" from the symbols!


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

	PANIC("booted!");
	printf("This text must not be reached!\n");

	/* 
    for (;;)
    {
        __asm__ __volatile__("wfi");
    }
    */
}

__attribute__((naked))                      // tells compiler -> no generating unnecessary code before and after function
__attribute__((aligned(4)))
// Entry point of the exception handler
// 
void kernel_entry(void)
{
    __asm__ __volatile__
    (
        "csrw sscratch, sp\n"               // register -> temp storage saving the STACK POINTER at the time of exception occurance
        "addi sp, sp, -4 * 31\n"            // offsetting the stack pointer...?
        "sw ra, 4 * 0(sp)\n"                  // sw = store word -> return address register
        "sw gp, 4 * 1(sp)\n"

        "csrw sscratch, sp\n"               // sscratch (register) -> temp storage -> saving the STACK POINTER at the time of exception occurance
        "addi sp, sp, -4 * 31\n"            // offsetting the stack pointer...  
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"               // SAVING 
        "sw t5,  4 * 8(sp)\n"               // REGISTER
        "sw t6,  4 * 9(sp)\n"               // STATE
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"              // Each register is 32 bits wide
        "sw a5,  4 * 15(sp)\n"              // Storing a WORD (a WORD is ALWAYS 32 BITS)
        "sw a6,  4 * 16(sp)\n"              // Size of 16(sp) is a WORD
        "sw a7,  4 * 17(sp)\n"              // 4 * (WORD) -> 16-bytes 
        "sw s0,  4 * 18(sp)\n"              // ISA: the stack grows downward 
        "sw s1,  4 * 19(sp)\n"              // stack pointer is ALWAYS kept
        "sw s2,  4 * 20(sp)\n"              // 16-byte aligned
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"
        
        "csrr a0, sscratch\n"                
        "sw a0, 4 * 30(sp)\n"               

        "mv a0, sp\n"                        // stack pointer stored in a0: sp points tp register values stored as the trap_frame structure
        "call handle_trap\n"                // call the handle_trap function
        
        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n"
    )    
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
