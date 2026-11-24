#include <stdio.h>
#include "retarget.h"
#if USB_PRINTF_ENABLE
#include "control_transport.h"
#endif
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

/* USB uses the existing persistent TX queue; a full queue drops debug bytes. */
int fputc(int value, FILE *stream)
{
    (void)stream;
#if USB_PRINTF_ENABLE
    uint8_t byte = (uint8_t)value;
    (void)control_transport_debug_write(&byte, 1U);
#endif
    return value;
}
