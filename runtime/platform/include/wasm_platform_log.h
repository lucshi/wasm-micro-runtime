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

#ifndef _WASM_PLATFORM_LOG
#define _WASM_PLATFORM_LOG

#include "wasm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

int wasm_printf(const char *fmt, ...);

int wasm_vprintf(const char *fmt, va_list ap);

int wasm_fprintf(void *stream, const char *fmt, ...);

int wasm_fflush(void *stream);

#ifdef __cplusplus
}
#endif

#endif /* _WASM_PLATFORM_LOG */
