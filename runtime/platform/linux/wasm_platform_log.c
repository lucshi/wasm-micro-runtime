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

#include "wasm_platform_log.h"

#include <stdarg.h>
#include <stdio.h>

int wasm_vprintf (const char *fmt, va_list ap)
{
    return vprintf(fmt, ap);
}

int wasm_printf (const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start (ap, fmt);
    ret = wasm_vprintf (fmt, ap);
    va_end (ap);

    return ret;
}

int wasm_fprintf (void *stream, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start (ap, fmt);
    ret = vfprintf (stream ? stream : stdout, fmt, ap);
    va_end (ap);

    return ret;
}

int wasm_fflush (void *stream)
{
    return fflush (stream ? stream : stdout);
}
