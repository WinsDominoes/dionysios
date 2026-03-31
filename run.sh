#!/bin/bash
set -xue

# Vars
QEMU=qemu-system-riscv32
CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib"
# CFlags explained (left to right): Use C11, enable optimizations, 
# max amount of debug info, major warnings, additional warnings, 
# compile for 32-bit risc-v, no std library for host env, use LLVM linker,
# disable unnecessary stack protection, no std library

# build kernel
$CC $CFLAGS -Wl, -Tkernel.ld -Wl, -Map=kernel.map -o kernel.elf \ 
    kernel.c
# -Wl => passing options to linker (and not C compiler)
# -Wl, -Tkernel.ld: specify linker script
# -Wl, -Map=kernel.map: Output a map file (linker allocation result)

# QEMU start cmd
$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot \
    -kernel kernel.elf