#include "common.h"

void putchar(char ch);

// fmt: format string
// va_list -> used by functions w/ varying number of args of varying types
void printf(const char *fmt, ...)
{
    va_list vargs; 
    va_start(vargs, fmt);

    while (*fmt)
    {
        if (*fmt == '%')
        {
            // Reading the format specifiers (%d, %s, %x)
            fmt++;                      // Skip '%' sign
            switch (*fmt)               // Read next char
            {
                // '%' at the end of format string
                case '\0': 
                {              
                    putchar('%');
                    goto end;
                }

                // Print '%'
                case '%': 
                {              
                    putchar('%');
                    break;
                }

                // Print a NULL-terminated string
                case 's': 
                {               
                    const char *s = va_arg(vargs, const char *);
                    while (*s)
                    {
                        putchar(*s);
                        s++;
                    }
                    break;
                }

                // Print an integer in decimal
                case 'd': 
                {               
                    int value = va_arg(vargs, int);
                    unsigned magnitude = value;        // negative values
                    
                    if (value < 0)
                    {
                        putchar('-');
                        magnitude = -magnitude;
                    }

                    unsigned divisor = 1;
                    while (magnitude / divisor > 9)
                        divisor *= 10;                  // *= compound operator

                    while (divisor > 0)
                    {
                        putchar('0' + magnitude / divisor);
                        magnitude %= divisor;
                        divisor /= 10;
                    }

                    break;
                }

                // Print int in hexadecimal
                case 'x':
                {
                    unsigned value = va_arg(vargs, unsigned);
                    for (int i = 7; i >= 0; i--)
                    {
                        // nibble: a hexidecimal digit (4 bits)
                        unsigned nibble = (value >> (i * 4)) & 0xf; 
                        putchar("0123456789abcdef"[nibble]);
                    }
                }
            }
        }
        else
        {
            putchar(*fmt);
        }

        fmt++;
    }

    end: 
        va_end(vargs);
}
