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

#include <unistd.h>
#include <sys/timeb.h>
#include <time.h>

/*
 * This function returns milliseconds per tick.
 * @return milliseconds per tick.
 */
uint64 bh_time_get_tick_millisecond() 
{
	return sysconf(_SC_CLK_TCK);
}

/*
 * This function returns milliseconds after boot.
 * @return milliseconds after boot.
 */
uint64 bh_time_get_boot_millisecond()
{
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC,&ts) != 0) {
		return 0;
	}

	return ((uint64)ts.tv_sec) * 1000 + ts.tv_nsec / ( 1000 * 1000 );
}

/*
 * This function returns GMT time milliseconds since from 1970.1.1, AKA UNIX time.
 * @return milliseconds since from 1970.1.1.
 */
uint64 bh_time_get_millisecond_from_1970()
{
	struct timeb tp;
	ftime( &tp);

	return ((uint64)tp.time) * 1000 + tp.millitm - (tp.dstflag == 0 ? 0 : 60 * 60 * 1000) + tp.timezone * 60 * 1000;
}

size_t
bh_time_strftime (char *s, size_t max, const char *format, int64 time)
{
  time_t time_sec = time / 1000;
  struct timeb tp;
  struct tm *ltp;

  ftime( &tp);
  time_sec -= tp.timezone * 60;

  ltp = localtime (&time_sec);
  if (ltp == NULL) {
    return 0;
  }
  return strftime (s, max, format, ltp);
}

