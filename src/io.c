#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "vga.h"

/* TODO: Move this function to string.h when made
// naive strlen since we don't have libc
static size_t strlen(const char* string)
{
    size_t len = 0;
    while (string[len])
    {
        len++;
    }

    return len;
}
*/

// naive itoa since we don't have libc
//TODO: Use heap memory to return a buffer instead of just printing to the console
static void itoa(uint32_t value)
{
    char buf[16];
    int pos = 0;
    if (value == 0)
    {
        term_put_char('0');
        return;
    }

    while (value > 0)
    {
        buf[pos++] = '0' + (value % 10);
        value = value / 10;
    }

    //buffer is backwards, write in reverse order
    for (int i = pos - 1; i >= 0; i--)
    {
        term_put_char(buf[i]);
    }
}

//very bad printf since we don't have libc
void printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p; p++)
    {
        if (*p == '%')
        {
            p++;
            if (*p == 's')
            {
                const char* string = va_arg(args, const char*);
                term_write(string);
            }
            else if (*p == 'd')
            {
                uint32_t d = va_arg(args, uint32_t);
                itoa(d);
            }
            else if (*p == '%')
            {
                term_put_char('%');
            }
            else //unreconized specifier
            {
                //TODO: Make this an error
                term_put_char('%');
                term_put_char(*p);
            }
        }
        else
        {
            term_put_char(*p);
        }
    }
    va_end(args);
}
