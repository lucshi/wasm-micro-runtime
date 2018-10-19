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

#include "bh_time.h"


uint64 bh_time_get_millisecond_from_1970()
{
  return k_uptime_get();
}

size_t
bh_time_strftime (char *str, size_t max, const char *format, int64 time)
{
  (void)format;
  (void)time;
  uint32 t = k_uptime_get_32();
  int h, m, s;

  t = t % (24 * 60 * 60);
  h = t / (60 * 60);
  t = t % (60 * 60);
  m = t / 60;
  s = t % 60;

  return snprintf(str, max, "%02d:%02d:%02d", h, m, s);
}

