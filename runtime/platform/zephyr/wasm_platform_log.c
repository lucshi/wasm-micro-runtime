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

#include "wasm_platform.h"
#include <stdio.h>


struct out_context {
  int count;
};

typedef int (*out_func_t)(int c, void *ctx);

extern void *__printk_get_hook(void);
static int (*_char_out)(int) = NULL;

static int
char_out(int c, struct out_context *ctx)
{
  ctx->count++;
  if(_char_out == NULL){
    _char_out = __printk_get_hook();
  }
  return _char_out(c);
}

static int
wasm_vprintk(const char *fmt, va_list ap)
{
  struct out_context ctx = { 0 };
  _vprintk((out_func_t)char_out, &ctx, fmt, ap);
  return ctx.count;
}

int wasm_vprintf (const char *fmt, va_list ap)
{
  return wasm_vprintk(fmt, ap);
}

void wasm_log_emit(const char *fmt, va_list ap)
{
  wasm_vprintk(fmt, ap);
}

int wasm_printf (const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start (ap, fmt);
  ret = wasm_vprintk (fmt, ap);
  va_end (ap);

  return ret;
}

int wasm_fprintf (FILE *stream, const char *fmt, ...)
{
  (void)stream;
  va_list ap;
  int ret;

  va_start (ap, fmt);
  ret = wasm_vprintk (fmt, ap);
  va_end (ap);

  return ret;
}

int wasm_fflush (void *stream)
{
  (void)stream;
  return 0;
}

