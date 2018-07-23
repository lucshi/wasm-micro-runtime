#ifndef _BH_ASSERT_H
#define _BH_ASSERT_H

#include "bh_config.h"
#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef BH_DEBUG
extern void bh_assert_internal(int v, const char *file_name, int line_number, const char *expr_string);
extern void bh_debug_internal(const char *file_name, int line_number, const char *fmt, ...);

#define bh_assert(expr) bh_assert_internal((int)(expr), __FILE__, __LINE__, # expr)

#if defined(WIN32)
#	define bh_debug(fmt, ...) bh_debug_internal(__FILE__, __LINE__, fmt, __VA_ARGS__)
#elif defined(__linux__)
# define bh_debug bh_debug_internal(__FILE__, __LINE__, "");printf
#else
#	error "Unsupported platform"
#endif

#else /* else of BH_DEBUG */

#define bh_assert(x) ((void)0)
#define bh_debug if(0)printf

#endif /* end of BH_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* end of _BH_ASSERT_H */

