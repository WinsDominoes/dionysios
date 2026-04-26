#!/bin/bash
set -xue

# Vars
QEMU=qemu-system-riscv32
## FOR LINUX USE THIS 
CC=clang
## FOR MAC (my iMac use this)
#CC="/usr/local/opt/llvm/bin/clang"
OBJCOPY=llvm-objcopy

CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib"
# CFlags explained (left to right): Use C11, enable optimizations, 
# max amount of debug info, major warnings, additional warnings, 
# compile for 32-bit risc-v, no std library for host env, use LLVM linker,
# disable unnecessary stack protection, no std library

# build the shell a.k.a the app
# compile C files & link w/ user.ld linker script
$CC $CFLAGS -Wl,-Tuser.ld -Wl,-Map=shell.map -o shell.elf shell.c user.c common.c
# converts the ELF to raw binary format. 
# Raw binary format is the ACTUAL content that will be expanded in memory
# from base addr (0x1000000). OS copies content of raw bin for preparation.
$OBJCOPY --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin
# Converts raw binary -> to format can be embed in C
$OBJCOPY -Ibinary -Oelf32-littleriscv shell.bin shell.bin.o

# build kernel
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
    kernel.c common.c shell.bin.o
# -Wl => passing options to linker (and not C compiler)
# -Wl, -Tkernel.ld: specify linker script
# -Wl, -Map=kernel.map: Output a map file (linker allocation result)

# QEMU start cmd
$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel kernel.elf
