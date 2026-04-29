#include "kernel.h"
#include "common.h"

// in K&R C book, Section A8.9 - typedef is basically an alias for a type
typedef unsigned char uint8_t; 
typedef unsigned int uint32_t;
typedef uint32_t size_t; 

extern char __bss[], __bss_end[], __stack_top[];    // we want to get the "address" from the symbols!
extern char __free_ram[], __free_ram_end[];

extern char __binary_shell_bin_start[], __binary_shell_bin_size[];  // symbols for embedded raw bin

// Process creation
struct process procs[PROCS_MAX];             // All process control structures
extern char __kernel_base[];				 // Kernel pages span from __kernel_base -> __free_ram_end
											 // Makes sure kernel alwaus can access static allocated & dynamic allocated

/* 
	MEMORY ALLOCATOR: BUMP / LINEAR ALLOCATOR
	- Used in cases where mem dealloc not neccessary
	- Practically, we need to be able to deallocate memory (for future implementation)
*/

// Since __free_ram is plasced on a 4KB boundary due to ALIGN(4096), the function
// must return an addr aligned to 4KB. That is why you multiply w/ page size
paddr_t alloc_pages(uint32_t n)
{
	// Static because value is same between func calls, that is, is like ehhh global vars 
	// next_paddr points to the "start address" of the "next area to be allocated" (free area)
	// initially holds addr of __free_ram (bc we start alloc mem from __free_ram addr - check kernel.ld linker file)
	static paddr_t next_paddr = (paddr_t) __free_ram;	
													
	paddr_t paddr = next_paddr;
	next_paddr += n * PAGE_SIZE;					// Defined in common.h						
													
	if (next_paddr > (paddr_t) __free_ram_end) {	// If you align more than __free_ram_end -> no more memory
		PANIC("Out of memory");
	}

	memset((void *) paddr, 0, n * PAGE_SIZE);		// Sets the alloc'd mem area is always filled with zeros. 
													// prevent unitialized memory and hard-to-debug issues
	return paddr;
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

// Function for handling the exception
void handle_trap(struct trap_frame *f)
{
    // Reads why the exception has occured
    uint32_t scause = READ_CSR(scause);         // Type of exception 
    uint32_t stval = READ_CSR(stval);           // Additional information about the exception (e.g. mem addr that caused the exception)
    uint32_t user_pc = READ_CSR(sepc);          // Program counter (at the point) where the exception occured.

    // Triggers a kernel panic for debugging processes
    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}


// MAPPING PAGES
/*
	table1 -> first-level page table
	vaddr -> virtual address
	paddr -> physical addressd
	flags -> page table entry flags
	
	TODO: Read more on RISC-V Paging mechanism (Sv32)
*/
void map_page(uint32_t *table1, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
	if (!is_aligned(vaddr, PAGE_SIZE))
	{
		PANIC("Unaligned vaddr %x", vaddr);
	}

	if (!is_aligned(paddr, PAGE_SIZE))
	{
		PANIC("Unaligned paddr %x", paddr);
	}

	uint32_t vpn1 = (vaddr >> 22) & 0x3ff; 
	if ((table1[vpn1] & PAGE_V) == 0)		// This means... there is no 1st level page table yet.
	{
		// Create 1st level apge
		uint32_t pt_paddr = alloc_pages(1);
		table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10 | PAGE_V);
	}

	// Set the 2nd level page entry to map the physical page
	uint32_t vpn0 = (vaddr >> 12) & 0x3ff;
	uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);
	table0[vpn0] = ((paddr / PAGE_SIZE) << 10 | flags | PAGE_V);

	// > It divides paddr by PAGE_SIZE because the entry should contain the physical page number, not the physical address itself.
}

void user_entry(void)
{
    PANIC("not yet implemented");
}

struct process *create_process(uint32_t pc)
{
    // Find unused proc in control structure.
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++)
    {
        if (procs[i].state == PROC_UNUSED)
        {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
    {
        PANIC("No free process slots");
    }

    // Stack callee-saved registers. 
    uint32_t *sp = (uint32_t *) &proc->stack[sizeof(proc->stack)];
    *--sp = 0;                      // s11
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint32_t) user_entry;  // ra

	// add page table - mapping the kernel page
	uint32_t *page_table = (uint32_t *) alloc_pages(1);
	// from __kernel_base to __free_ram_end -> increment paddr by PAGE_SIZE
	// essentially, mapping kernel page
	for (paddr_t paddr = (paddr_t) __kernel_base; 
		 paddr < (paddr_t) __free_ram_end; paddr += PAGE_SIZE)
	{
		map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);
	}

	// map user pages
	for (uint32_t off = 0; off < image_size; off += PAGE_SIZE)
	{
	    paddr_t page = alloc_pages(1);
	    // TODO

	    // IF data coped is smaller than page size
	    size_t remaining = image_size - opf
	}

    // Initialize fields.
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;
    proc->page_table = page_table;
    return proc;
}

// CONTEXT SWITCHING
__attribute__((naked)) void switch_context(uint32_t *prev_sp,
                                           uint32_t *next_sp)
{
    __asm__ __volatile__
    (
        // Callee-saved registers: Registers that a called function must restore before returning.
        // Save callee-saved registers onto the current process's stack.
        "addi sp, sp, -13 * 4\n" // Allocate stack space for 13 4-byte registers
        "sw ra,  0  * 4(sp)\n"   // Save callee-saved registers only
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Switch the stack pointer.
        "sw sp, (a0)\n"         // *prev_sp = sp;
        "lw sp, (a1)\n"         // Switch stack pointer (sp) here

        // Restore callee-saved registers from the next process's stack.
        "lw ra,  0  * 4(sp)\n"  // Restore callee-saved registers only
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"  // We've popped 13 4-byte registers from the stack
        "ret\n"
    );
}

// Scheduler
struct process *current_proc;                   // Current process dat is running
struct process *idle_proc;                      // idle process - the process to run when there are no runnable processes

void yield(void)
{
    // Search for runnable process
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++)
    {
        struct process *proc = &procs[(current_proc->pid + i)];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0)
        {
            next = proc;
            break;
        }
    }

    // If no runnable proc, continue running this current proc
    if (next == current_proc)
        return;

	// sfence.vma -> ensure changes to page table are properly completed (mem fence)
	// 			  -> clear cache of page table entries (TLB)

	// switching page tables (because context switching -> switch process' page table)
    __asm__ __volatile__
    (
    	"sfence.vma\n"
    	"csrw satp, %[satp]\n"
    	"sfence.vma\n"
        "csrw sscratch, %[sscratch]\n"
        :
        : [satp] "r" (SATP_SV32 | ((uint32_t) next->page_table / PAGE_SIZE)),
          [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
    );

    // context switching
    struct process *prev = current_proc;
    current_proc = next; 
    switch_context(&prev->sp, &next->sp);
}


__attribute__((naked))                      // tells compiler -> no generating unnecessary code before and after function
__attribute__((aligned(4)))                 // aligns function starting addr to a 4-byte boundary
// Entry point of the exception handler
// This whole thing is basically saving the state of the registers and doing the exception handling
void kernel_entry(void)
{
    __asm__ __volatile__
    (
        "csrrw sp, sscratch, sp\n"          // register -> temp storage saving the STACK POINTER at the time of exception occurance
                                            // retrieve the kernel stack of the running process from sscratch
                                            // sp points to the kernel stack of the currently running proc
                                            // sscratch holds the original value of sp (user stack) at the time of except.
                                            
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

        // Restore the original value of sp from sscratch and save it to kernel stack
        "csrr a0, sscratch\n"                // sscratch (tmp storage) has the val of sp (earlier) -> store into a0
        "sw a0, 4 * 30(sp)\n"                

        // Reset the kernel stack
        "addi a0, sp, 4 * 31\n"
        "csrw sscratch, a0\n"

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
    );

    /* 
    The key point here is that each process has its own independent kernel stack. 
    By switching the contents of sscratch during context switching, 
    we can resume the execution of the process from the point where it was interrupted, 
    as if nothing had happened.
    */
}

// Test context switching
void delay(void)
{
    for (int i = 0; i < 30000000; i++)
    {
        __asm__ __volatile__("nop");        // do nothing
    }
}

struct process *proc_a;
struct process *proc_b;

// Process A
void proc_a_entry(void)
{
    printf("Starting process A\n");
    while (1)
    {
        putchar('A');
        switch_context(&proc_a->sp, &proc_b->sp);
        yield();
    }
}

// Proces B
void proc_b_entry(void)
{
    printf("Start process B\n");
    while (1)
    {
        putchar('B');
        switch_context(&proc_b->sp, &proc_a->sp);
        yield();
    }
}

// Main Kernel function
void kernel_main(void)
{
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);  // initializes / sets the bss section to zero

    printf("\n\n");
    
    WRITE_CSR(stvec, (uint32_t) kernel_entry);
    
    idle_proc = create_process((uint32_t) NULL);
    idle_proc->pid = 0;                     // Idle process id
    current_proc = idle_proc;

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);

    yield();
    PANIC("Switched to idle process");

	// paddr_t paddr0 = alloc_pages(2);		// Alloc two pages of memory (8KB)
	// paddr_t paddr1 = alloc_pages(1);		// Alloc one page of memory	(4KB)
	// printf("alloc_pages test: paddr0=%x\n", paddr0);
	// printf("alloc_pages test: paddr1=%x\n", paddr1);

    // printf("\n\nHello %s\n", "World!");
    // printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);

	// PANIC("unreachable here!");


    /* UNUSED 
    
    // unimp is a pseudo instruction - the assembler turns it into
    // csrrw x0, cycle, x0 
    // reads and writes the cycle register into x0 
    // cycle is a read-only register, therefore, you can write shit into it
    // CPU sees it, it goes "INVALID!" and triggers an illegal instruction exception
    __asm__ __volatile__("unimp");
    
    
    */

	// UNUSED

	// printf("This text must not be reached!\n");

	/* 
    for (;;)
    {
        __asm__ __volatile__("wfi");
    }
    */
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
// Process creation
struct process procs[PROCS_MAX];             // All process control structures
extern char __kernel_base[];				 // Kernel pages span from __kernel_base -> __free_ram_end
											 // Makes sure kernel alwaus can access static allocated & dynamic allocated
