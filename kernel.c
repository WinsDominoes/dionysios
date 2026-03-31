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

void kernel_main(void)
{
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);  // initializes / sets the bss section to zero

    for (;;);
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