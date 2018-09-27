/* 
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
*/

#ifndef _BH_PLATFORM_LOG
#define _BH_PLATFORM_LOG

#include "bh_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

int bh_printf(const char *fmt, ...);

int bh_vprintf(const char *fmt, va_list ap);

int bh_fprintf(void *stream, const char *fmt, ...);

int bh_fflush(void *stream);

#ifdef __cplusplus
}
#endif

#endif /* _BH_PLATFORM_LOG */
