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

#ifndef _WASM_TIME_H
#define _WASM_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wasm_types.h"

/*
 * This function returns milliseconds per tick.
 * @return milliseconds per tick.
 */
uint64 wasm_time_get_tick_millisecond(void);

/*
 * This function returns milliseconds after boot.
 * @return milliseconds after boot.
 */
uint64 wasm_time_get_boot_millisecond(void);

/*
 * This function returns GMT milliseconds since from 1970.1.1, AKA UNIX time.
 * @return milliseconds since from 1970.1.1.
 */
uint64 wasm_time_get_millisecond_from_1970(void);

size_t wasm_time_strftime(char *s, size_t max, const char *format, int64 time);

#ifdef __cplusplus
}
#endif

#endif
