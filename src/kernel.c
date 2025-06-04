#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/*
 * VGA text‐mode buffer starts at physical 0xB8000.
 * Each “cell” is two bytes:
 *   byte[0] = ASCII code,
 *   byte[1] = color (high nibble = background, low nibble = foreground).
 */

#define VGA_ADDR ((volatile uint8_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint8_t terminal_color = 0x07; //default
static size_t terminal_row = 0;
static size_t terminal_col = 0;

//Combine the fg and bg into one byte
static void terminal_setcolor(uint8_t fg, uint8_t bg)
{
    terminal_color = (bg << 4 ) | (fg & 0x0F);
}

static void terminal_newline(void)
{
    terminal_col = 0;
    terminal_row++;
    if (terminal_row >= VGA_HEIGHT)
    {
        //TODO: Introduce wrapping or scrolling
        terminal_row = 0;
    }
}

static void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_newline();
        return;
    }

    //check boundaries
    if ((unsigned)terminal_col >= VGA_WIDTH)
    {
        terminal_newline();
    }

    size_t i = (terminal_row * VGA_WIDTH + terminal_col) * 2;
    VGA_ADDR[i] = (uint16_t)c;
    VGA_ADDR[i + 1] = terminal_color;
    terminal_col++;
}

static void terminal_write(const char* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        terminal_putchar(data[i]);
    }
}

static void terminal_clear(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        terminal_putchar('\0');
    }
}

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

// naive itoa since we don't have libc
//TODO: Use heap memory to return a buffer instead of just printing to the console
static void itoa(uint32_t value)
{
    char buf[16];
    int pos = 0;
    if (value == 0)
    {
        terminal_putchar('0');
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
        terminal_putchar(buf[i]);
    }
}

static void terminal_writestring(const char* string)
{
    terminal_write(string, strlen(string));
}

//very bad printf since we don't have libc
static void printf(const char* format, ...)
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
                terminal_writestring(string);
            }
            else if (*p == 'd')
            {
                uint32_t d = va_arg(args, uint32_t);
                itoa(d);
            }
            else if (*p == '%')
            {
                terminal_putchar('%');
            }
            else //unreconized specifier
            {
                //TODO: Make this an error
                terminal_putchar('%');
                terminal_putchar(*p);
            }
        }
        else
        {
            terminal_putchar(*p);
        }
    }
    va_end(args);
}

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{
    terminal_setcolor(15, 1); //fg white bg blue
    terminal_clear();
    printf("Hello World!\n");
    const char* name = "Justin";
    const int age = 25;
    printf("Your name is %s and you are %d years old!\n", name, age);
    printf("magic:%d\n", multiboot_magic);
    printf("multiboot_info:%d\n", multiboot_info);
    printf("multiboot flag:%d\n", *(uint32_t*)multiboot_info);

    //die
    while (1)
    {
        __asm__("hlt");
    }
}
