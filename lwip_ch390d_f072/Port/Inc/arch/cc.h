#ifndef F072_LWIP_CC_H
#define F072_LWIP_CC_H

#include <stdint.h>
#include <stdio.h>
#include "main.h"

typedef int sys_prot_t;
#define BYTE_ORDER LITTLE_ENDIAN
#if defined(__CC_ARM)
#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#else
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#endif
#define LWIP_PLATFORM_DIAG(x) do { printf x; } while (0)
#define LWIP_PLATFORM_ASSERT(x) do { (void)(x); Error_Handler(); } while (0)

#endif
