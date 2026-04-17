# Memory Allocation Implemented
Implemented a simple linear allocator -> but currently has no deallocation, will implement later.

When allocating memory with this allocator, we allocate a "page" of memory.
In this case, one page of memory = 4096 bytes or 4KB.
Therefore, the function `alloc_page` in kernel.c takes in `n` (number of pages) and multiply with PAGE_SIZE or 4096 Bytes; essentially making sure that the memory address is aligned to 4KB (as defined in `kernel.ld`)

**NOTE**: 4096 bytes or 4 KB = 0x1000 in hexadecimal
```bash
alloc_pages test: paddr0=80221000	# Here, we first allocated ONE page of memory, the memory starts from __free_end that is why it has that number
alloc_pages test: paddr1=80223000	# When we allocate 8 KB which in hex is 0x2000 -> 0x80221000 jumps to 80223000 -> difference of 0x2000 :D
PANIC: kernel.c:88: booted!
```

Check with llvm-nm
```bash
winsdominoes@fedora:~/Workspace/dionysios$ llvm-nm kernel.elf | grep __free_ram
80221000 B __free_ram_end			# Start of the free RAM section
84221000 B __free_ram_end			# End of the free RAM section
```
If you do your hexarithmetic, `84221000 - 80221000 = 4000000` and `4000000` corresponds to 64 MB. That is the size of the free RAM area as defined in `kernel.ld`
And there it comes together!
