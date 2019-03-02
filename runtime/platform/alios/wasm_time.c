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

#include "wasm_time.h"


uint64 wasm_time_get_millisecond_from_1970()
{
  return (uint64)aos_now_ms();
}

size_t
wasm_time_strftime (char *str, size_t max, const char *format, int64 time)
{
  str = aos_now_time_str(str, max);
  return str ? strlen(str) + 1 : 0;
}

