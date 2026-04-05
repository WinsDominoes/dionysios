#include "common.h"

void putchar(char ch);

// memset: writes c into buf with the size n)
// n decreases because n decrease -> stack grows
// as the stack grows, we write c into that buffer
// which is referenced by *p pointer.
void *memset(void *buf, char c, size_t n)
{
    uint8_t *p = (uint8_t *) buf;
    while (n--)
    {
        *p++ = c;		// Pointer dereferencing -> 1. dereferences p (and set p to c), then p = p + 1
    } 
    return buf;
}

/* String operations */
// safer alternative: strncpy
char *strcpy(char *dst, const char *src)
{
	char *d = dst;
	while (*src)
	{
		*d++ = *src; 
	}
	*d = '\0';		// set to null terminator
	return dst;
}

/*
	Return values
	- s1 == s1 (the same): 0
	- s1 > s2: positive value
	- s1 < s2: negative value
*/
int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2)
	{
		if (*s1 != *s2) 
		{	
			break;		// this means the string is no longer the same (different char detected)
		}

		s1++;			// looping through the string 
		s2++;
	}

	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// fmt: format string
// va_list -> used by functions w/ varying number of args of varying types
void printf(const char *fmt, ...)
{
	// Following standard just like K&R C (stdarg.h)
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
