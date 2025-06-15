#ifndef ERROR_H
#define ERROR_H

//these shouldn't be called by users but need to be here for the macro
extern void panic_start();
extern void panic_end();

//TODO: Impliment vprintf to make panic a function call
#define panic(fmt, ...)                     \
    do {                                           \
        panic_start();                      \
        earlyprintf("PANIC: " fmt, ##__VA_ARGS__); \
        panic_end();                      \
    } while (0)

#endif //ERROR_H
