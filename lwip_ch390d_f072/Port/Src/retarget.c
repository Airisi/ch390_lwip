#include <stdio.h>
#if defined(__CC_ARM)
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int value) { (void)value; for (;;) {} }
int _ttywrch(int value) { return value; }
char *_sys_command_string(char *command, int length)
{
    (void)command;
    (void)length;
    return NULL;
}
#endif

#if defined(__GNUC__)
void _init(void) {}
void _fini(void) {}
#endif

/* Do not interleave diagnostic text with the binary USB configuration frames. */
int fputc(int value, FILE *stream) { (void)stream; return value; }
