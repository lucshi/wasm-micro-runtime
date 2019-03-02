#ifndef _WASM_ASSERT_H
#define _WASM_ASSERT_H

#include "wasm_config.h"
#include "wasm_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WASM_DEBUG
extern void wasm_assert_internal(int v, const char *file_name, int line_number, const char *expr_string);

#define wasm_assert(expr) wasm_assert_internal((int)(expr), __FILE__, __LINE__, # expr)

#else /* else of WASM_DEBUG */

#define wasm_assert(x) ((void)0)

#endif /* end of WASM_DEBUG */

#ifdef WASM_TEST
#   define WASM_STATIC
#else
#   define WASM_STATIC static
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_ASSERT_H */

