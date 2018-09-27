#ifndef _BH_ASSERT_H
#define _BH_ASSERT_H

#include "bh_config.h"
#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef BH_DEBUG
extern void bh_assert_internal(int v, const char *file_name, int line_number, const char *expr_string);

#define bh_assert(expr) bh_assert_internal((int)(expr), __FILE__, __LINE__, # expr)

#else /* else of BH_DEBUG */

#define bh_assert(x) ((void)0)

#endif /* end of BH_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* end of _BH_ASSERT_H */

