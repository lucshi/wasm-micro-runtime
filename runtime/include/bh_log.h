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
/**
 * @brief This log system supports wrapping multiple outputs into one
 * log message.  This is useful for outputting variable-length logs
 * without additional memory overhead (the buffer for concatenating
 * the message), e.g. exception stack trace, which cannot be printed
 * by a single log calling without the help of an additional buffer.
 * Avoiding additional memory buffer is useful for resource-constraint
 * systems.  It can minimize the impact of log system on applications
 * and logs can be printed even when no enough memory is available.
 * Functions with prefix "_" are private functions.  Only macros that
 * are not start with "_" are exposed and can be used.
 */

#ifndef _BH_LOG_H
#define _BH_LOG_H

#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * The following functions are the primitive operations of this log system.
 * A normal usage of the log system is to call bh_log_begin and then call
 * bh_log_printf or bh_log_vprintf one or multiple times and then call
 * bh_log_end to wrap (mark) the previous outputs into one log message.
 * The bh_log and macros LOG_ERROR etc. can be used to output log messages
 * by one log calling.
 */
int  _bh_log_init (void);
void _bh_log_set_verbose_level (int level);
bool _bh_log_begin (int level);
void _bh_log_printf (const char *fmt, ...);
void _bh_log_vprintf (const char *fmt, va_list ap);
void _bh_log_end (void);
void _bh_log (int level, const char *file, int line,
              const char *fmt, ...);

#if WASM_ENABLE_LOG != 0
# define bh_log_init()               _bh_log_init ()
# define bh_log_set_verbose_level(l) _bh_log_set_verbose_level (l)
# define bh_log_begin(l)             _bh_log_begin (l)
# define bh_log_printf(...)          _bh_log_printf (__VA_ARGS__)
# define bh_log_vprintf(...)         _bh_log_vprintf (__VA_ARGS__)
# define bh_log_end()                _bh_log_end ()
# define bh_log(...)                 _bh_log (__VA_ARGS__)
#else  /* WASM_ENABLE_LOG != 0 */
# define bh_log_init()               0
# define bh_log_set_verbose_level(l) (void)0
# define bh_log_begin()              false
# define bh_log_printf(...)          (void)0
# define bh_log_vprintf(...)         (void)0
# define bh_log_end()                (void)0
# define bh_log(...)                 (void)0
#endif  /* WASM_ENABLE_LOG != 0 */

#define LOG_ERROR(...)          bh_log (0, NULL, 0, __VA_ARGS__)
#define LOG_WARNING(...)        bh_log (1, NULL, 0, __VA_ARGS__)
#define LOG_VERBOSE(...)        bh_log (2, NULL, 0, __VA_ARGS__)

#if defined(BH_DEBUG)
# define LOG_DEBUG(...)         _bh_log (1, __FILE__, __LINE__, __VA_ARGS__)
#else  /* defined(BH_DEBUG) */
# define LOG_DEBUG(...)         (void)0
#endif  /* defined(BH_DEBUG) */

#define LOG_PROFILE_HEAP_GC(heap, size)                         \
  LOG_VERBOSE("PROF.HEAP.GC: HEAP=%08X SIZE=%d", heap, size)

#ifdef __cplusplus
}
#endif


#endif  /* _BH_LOG_H */
