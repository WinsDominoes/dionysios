#pragma once

typedef int bool;
typedef unsigned char uint_8t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t;					// Physical memory addresses
typedef uint32_t vaddr_t;					// Virtual memory addresses (if in stdlib C, uintptr_t)

// __builtin__ prefix - provided by the compiler (clang)
// https://clang.llvm.org/docs/LanguageExtensions.html#variadic-function-builtins
#define va_list     __builtin_va_list
#define va_start    __builtin_va_start
#define va_end      __builtin_va_end
#define va_arg      __builtin_va_arg

void printf(const char *fmt, ...);
