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

#include "wasm_log.h"

#include "wasm_platform_log.h"
#include "wasm_thread.h"


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
_wasm_log_init ()
{
  log_verbose_level = 1;
  return ws_mutex_init (&log_stream_lock, false);
}

void
_wasm_log_set_verbose_level (int level)
{
  log_verbose_level = level;
}

bool
_wasm_log_begin (int level)
{
  korp_tid self;

  if (level > log_verbose_level) {
    return false;
  }

  /* Try to own the log stream and start the log output.  */
  ws_mutex_lock (&log_stream_lock);
  self = ws_self_thread ();
  wasm_printf ("[%X]: ", (int)self);

  return true;
}

void
_wasm_log_vprintf (const char *fmt, va_list ap)
{
  wasm_vprintf (fmt, ap);
}

void
_wasm_log_printf (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  _wasm_log_vprintf (fmt, ap);
  va_end (ap);
}

void
_wasm_log_end ()
{
  ws_mutex_unlock (&log_stream_lock);
}

void
_wasm_log (int level, const char *file, int line,
           const char *fmt, ...)
{
  if (_wasm_log_begin (level)) {
    va_list ap;

    if (file)
      _wasm_log_printf ("%s:%d ", file, line);

    va_start (ap, fmt);
    _wasm_log_vprintf (fmt, ap);
    va_end (ap);

    _wasm_log_end ();
  }
}
