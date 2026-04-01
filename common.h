#pragma once

// __builtin__ prefix - provided by the compiler (clang)
// https://clang.llvm.org/docs/LanguageExtensions.html#variadic-function-builtins
#define va_list     __builtin_va_list
#define va_start    __builtin_va_start
#define va_end      __builtin_va_end
#define va_arg      __builtin_va_arg

void printf(const char *fmt, ...);