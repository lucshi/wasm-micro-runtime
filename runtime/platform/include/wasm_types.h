#ifndef _WASM_TYPES_H
#define _WASM_TYPES_H

#include "wasm_config.h"

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;

#include "wasm_platform.h"

#ifndef __cplusplus
#define true 1
#define false 0
#define inline __inline
#endif

#endif /* end of _WASM_TYPES_H */

