# 10th April 2026
Implemented exception handling

## Developments so far
When running `./run.sh`, we will get...
```bash
Hello World!
1 + 2 = 3, 1234abcd
PANIC: kernel.c:57: unexpected trap scause=00000002, stval=00000000, sepc=8020013c
```
This shows that...
- scause = 2 -> 2 = illegal instruction
which is the intended thing from the `unimp` instruction executed in `kernel.c` to test the exception handler.

## Examining / Debugging:
Seen in the output section above, you can see that `sepc=8020013c` which signifies the program counter's value when that exception occured.
From this, we can check where the hell is "exception'd" by doing...
```bash
$ llvm-addr2line -e kernel.elf 8020013c
/var/home/winsdominoes/Workspace/dionysios/kernel.c:160
```
Look at how wonderful this is! The program `llvm-addr2-line` takes in the address `8020013c` and spits out the line in that specific source file (`kernel.c`) where the exception occured. 
And if you check in `kernel.c` (as of this commit), you will see...
```c
155		// unimp is a pseudo instruction - the assembler turns it into
156		// csrrw x0, cycle, x0 
157		// reads and writes the cycle register into x0 
158		// cycle is a read-only register, therefore, you can write shit into it
159		// CPU sees it, it goes "INVALID!" and triggers an illegal instruction exception
160		__asm__ __volatile__("unimp");			// <---- SEE!?!?
```
Voila! See that? kernel.c:160 / line 160 is EXACTLY where the exception occured! How cool is that?
Very useful for debugging!

From: Win <winsdominoes@winscloud.net> 00:00 GMT +8 (Singapore)
