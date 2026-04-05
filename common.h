#pragma once

typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t;					// Physical memory addresses
typedef uint32_t vaddr_t;					// Virtual memory addresses (if in stdlib C, uintptr_t)

// Macros
#define true 1
#define false 0
#define NULL ((void *) 0) // void * -> a pointer to something "not tightly specified"

/* macros for memory alignment */
// __builtin__ -> clang-specific functions
#define align_up(value, align)		__builtin_align_up(value, align)		// the thing that rounds up
#define is_aligned(value, align)	__buildin_is_aligned(valie, align)		// check if rounded up
#define offsetof(type, member)		__builtin_offsetof(type, member)		// returns offset of member within structure

// __builtin__ prefix - provided by the compiler (clang)
// https://clang.llvm.org/docs/LanguageExtensions.html#variadic-function-builtins
#define va_list     __builtin_va_list
#define va_start    __builtin_va_start
#define va_end      __builtin_va_end
#define va_arg      __builtin_va_arg

void *memset(void *buf, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
// const char *src: src (pointer) to the base address that the src string is stored in
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);
