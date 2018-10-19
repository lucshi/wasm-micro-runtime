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

#include "bh_log.h"

#include "bh_platform_log.h"
#include "bh_thread.h"
#include "bh_time.h"


/**
 * The verbose level of the log system.  Only those verbose logs whose
 * levels are less than or equal to this value are outputed.
 */
static int log_verbose_level;

/**
 * The lock for protecting the global output stream of logs.
 */
static korp_mutex log_stream_lock;


int
_bh_log_init ()
{
  log_verbose_level = 1;
  return vm_mutex_init (&log_stream_lock, false);
}

void
_bh_log_set_verbose_level (int level)
{
  log_verbose_level = level;
}

bool
_bh_log_begin (int level)
{
  korp_tid self;
#ifndef __ZEPHYR__
  char buf[32];
#endif

  if (level > log_verbose_level) {
    return false;
  }

  /* Try to own the log stream and start the log output.  */
  vm_mutex_lock (&log_stream_lock);
  self = vm_self_thread ();
#ifndef __ZEPHYR__
  bh_time_strftime (buf, 32, "%Y-%m-%d %H:%M:%S",
                    bh_time_get_millisecond_from_1970 ());
  bh_printf ("[%s - %X]: ", buf, (int)self);
#else
  bh_printf ("[%X]: ", (int)self);
#endif

  return true;
}

void
_bh_log_vprintf (const char *fmt, va_list ap)
{
  bh_vprintf (fmt, ap);
}

void
_bh_log_printf (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  _bh_log_vprintf (fmt, ap);
  va_end (ap);
}

void
_bh_log_end ()
{
  vm_mutex_unlock (&log_stream_lock);
}

void
_bh_log (int level, const char *file, int line,
         const char *fmt, ...)
{
  if (_bh_log_begin (level)) {
    va_list ap;

    if (file)
      _bh_log_printf ("%s:%d ", file, line);

    va_start (ap, fmt);
    _bh_log_vprintf (fmt, ap);
    va_end (ap);

    _bh_log_end ();
  }
}
