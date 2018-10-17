/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010, 2011 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors. Title to the Material
 * remains with Intel Corporation or its suppliers and licensors. The
 * Material contains trade secrets and proprietary and confidential
 * information of Intel or its suppliers and licensors. The Material
 * is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way without Intel's prior express
 * written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for O_DIRECT */
#endif

#include "wasm-native.h"
#include "wasm-runtime.h"
#include "bh_log.h"
#include "bh_memory.h"
#include "bh_platform_log.h"

#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#if 0
#define MEMORY(self) (self->vm_instance->module->default_memory)
#define MEMORY_BASE(self) (MEMORY(self)->memory_data)
#else
#define MEMORY_BASE(self) (0)
#endif

#define DEBUG_PRINT 1

#if DEBUG_PRINT != 0
static void
_print_i32_wrapper(WASMThread *self, uint32 *args)
{
  bh_printf("%d\n", *args);
}

static void
_print_i64_wrapper(WASMThread *self, uint32 *args)
{
  uint64 value;
  memcpy(&value, args, sizeof(uint64));
  bh_printf("%lld\n", value);
}

static void
_print_f32_wrapper(WASMThread *self, uint32 *args)
{
  bh_printf("%f\n", *(float*)args);
}

static void
_print_f64_wrapper(WASMThread *self, uint32 *args)
{
  float64 value;
  memcpy(&value, args, sizeof(float64));
  bh_printf("%f\n", value);
}
#endif

static void
abort_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  bh_printf("env.abort(%i)\n",code);
  abort();
}

static void
_printf_wrapper(WASMThread *self, uint32 *args)
{
  const char *fmt = (const char*)args[0];
  va_list va_args = (va_list)args[1];
  *args = vprintf(fmt, va_args);
}

static void
_fprintf_wrapper(WASMThread *self, uint32 *args)
{
  FILE *stream = (FILE*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = (va_list)args[2];
  *args = vfprintf(stream, fmt, va_args);
}

static void
_sprintf_wrapper(WASMThread *self, uint32 *args)
{
  char *str = (char*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = (va_list)args[2];
  *args = vsprintf(str, fmt, va_args);
}

static void
_snprintf_wrapper(WASMThread *self, uint32 *args)
{
  char *str = (char*)args[0];
  size_t size = args[1];
  const char *fmt = (const char*)args[2];
  va_list va_args = (va_list)args[3];
  *args = vsnprintf(str, size, fmt, va_args);
}

static void
_scanf_wrapper(WASMThread *self, uint32 *args)
{
  const char *fmt = (const char*)args[0];
  va_list va_args = (va_list)args[1];
  *args = vscanf(fmt, va_args);
}

static void
_fscanf_wrapper(WASMThread *self, uint32 *args)
{
  FILE *stream = (FILE*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = (va_list)args[2];
  *args = vfscanf(stream, fmt, va_args);
}

static void
_sscanf_wrapper(WASMThread *self, uint32 *args)
{
  char *str= (char*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = (va_list)args[2];
  *args = vsscanf(str, fmt, va_args);
}

void *__wrap_malloc(size_t size);
void *__wrap_calloc(size_t nmemb, size_t size);
void __wrap_free(void *ptr);

static void
_malloc_wrapper(WASMThread *self, uint32 *args)
{
  *args = (uint32)__wrap_malloc(args[0]);
}

static void
_calloc_wrapper(WASMThread *self, uint32 *args)
{
  *args = (uint32)__wrap_calloc(args[0], args[1]);
}

static void
_free_wrapper(WASMThread *self, uint32 *args)
{
  __wrap_free((void*)args[0]);
}

static void
___syscall140_wrapper(WASMThread *self, uint32 *args)
{
  /* llseek */
  /* could not call _llseek since it's not exposed in glibc, and syscall is also impossible to be called, so use lseek64 instead. */
  /*
  int64 *p = (int64*)args + sizeof(int32);
  int32 fd = p[0];
  int64 off_high = p[1];
  int64 off_low = p[2];
  int64 *result = &p[3];
  int32 whence = p[4];
  int32 ret = syscall(SYS_llseek, fd, off_high, off_low, result, whence);

  *(int64*)args = *result;

  if (ret != 0) {
    LOG_ERROR("___syscall140_wrapper execute failed.\n");
    abort();
  }
  */

  int64 *p = (int64*)args + sizeof(int32);
  int32 fd = p[0];
  int64 off_high = p[1];
  int64 off_low = p[2];
  int32 whence = p[4];
  int64 off = ((off_high << 32) & (0xffffffff00000000)) | off_low;
  int32 ret = lseek64(fd, off, whence);

  *args = ret;
}

static void
___syscall146_wrapper(WASMThread *self, uint32 *args)
{
  /* writev */
  uint32 i, ptr, len, count;
  uint8 *memory_base = MEMORY_BASE(self);
  int32 fileno = args[1];
  int32 iov = args[2];
  uint32 iovcnt = args[3];
  struct iovec *native_iovec;

  if(!(native_iovec = bh_malloc(sizeof(struct iovec) * iovcnt))) {
    LOG_ERROR("alloc iovec failed in native function lookup.\n");
    abort();
  }

  for (i = 0; i < iovcnt; i++) {
    ptr = iov + i * 8;
    len = iov + i * 8 + 4;
    native_iovec[i].iov_base = memory_base + ptr;
    native_iovec[i].iov_len = len;
  }

  count = writev(fileno, native_iovec, iovcnt);
  *args = count;
}

static void
___syscall54_wrapper(WASMThread *self, uint32 *args)
{
  /* ioctl */
  /* TODO: might need support VA args for ioctl() in the future */
  int32 fd = args[1];
  int32 cmd = args[2];
  int32 ret = ioctl(fd, cmd);
  *args = ret;
}

static void
___syscall6_wrapper(WASMThread *self, uint32 *args)
{
  /* close */
  int32 fd = args[1];
  int32 ret = close(fd);
  *args = ret;
}

static void
_emscripten_memcpy_big_wrapper(WASMThread *self, uint32 *args)
{
  uint8 *memory_base = MEMORY_BASE(self);
  int32 off_dest = args[0];
  int32 off_src = args[1];
  int32 len = args[2];
  memcpy(memory_base + off_dest, memory_base + off_src, len);
  *args = off_dest;
}

#ifdef WASM_ENABLE_REPL
static void
print_i32_wrapper(WASMThread *self, uint32 *args)
{
  bh_printf("%d\n", *args);
}

static void
print_wrapper(WASMThread *self, uint32 *args)
{
  bh_printf("%d\n", *args);
}
#endif

/* TODO: add function parameter/result types check */
#define REG_NATIVE_FUNC(module_name, func_name) \
    {#module_name, #func_name, func_name##_wrapper}

typedef struct WASMNativeFuncDef {
  const char *module_name;
  const char *func_name;
  void (*func_ptr)(WASMThread *self, uint32 *args);
} WASMNativeFuncDef;

static WASMNativeFuncDef native_func_defs[] = {
#if DEBUG_PRINT != 0
  REG_NATIVE_FUNC(env, _print_i32),
  REG_NATIVE_FUNC(env, _print_i64),
  REG_NATIVE_FUNC(env, _print_f32),
  REG_NATIVE_FUNC(env, _print_f64),
#endif
  REG_NATIVE_FUNC(env, abort),
  REG_NATIVE_FUNC(env, _printf),
  REG_NATIVE_FUNC(env, _fprintf),
  REG_NATIVE_FUNC(env, _sprintf),
  REG_NATIVE_FUNC(env, _snprintf),
  REG_NATIVE_FUNC(env, _scanf),
  REG_NATIVE_FUNC(env, _fscanf),
  REG_NATIVE_FUNC(env, _sscanf),
  REG_NATIVE_FUNC(env, _malloc),
  REG_NATIVE_FUNC(env, _calloc),
  REG_NATIVE_FUNC(env, _free),
  REG_NATIVE_FUNC(env, ___syscall140),
  REG_NATIVE_FUNC(env, ___syscall146),
  REG_NATIVE_FUNC(env, ___syscall54),
  REG_NATIVE_FUNC(env, ___syscall6),
  REG_NATIVE_FUNC(env, _emscripten_memcpy_big),
#ifdef WASM_ENABLE_REPL
  REG_NATIVE_FUNC(spectest, print_i32),
  REG_NATIVE_FUNC(spectest, print)
#endif
};

void*
wasm_native_func_lookup(const char *module_name, const char *func_name)
{
  uint32 size = sizeof(native_func_defs) / sizeof(WASMNativeFuncDef);
  WASMNativeFuncDef *func_def = native_func_defs;
  WASMNativeFuncDef *func_def_end = func_def + size;

  if (!module_name || !func_name)
    return NULL;

  while (func_def < func_def_end) {
    if (!strcmp(func_def->module_name, module_name)
        && !strcmp(func_def->func_name, func_name))
      return (void*)(uintptr_t)func_def->func_ptr;
    func_def++;
  }

  return NULL;
}

/*************************************
 * Global Variables                  *
 *************************************/

extern char **environ;

typedef struct WASMNativeGlobalDef {
  const char *module_name;
  const char *global_name;
  WASMValue global_data;
} WASMNativeGlobalDef;

static WASMNativeGlobalDef native_global_defs[] = {
  { "env", "STACKTOP", .global_data.u32 = 64 * NumBytesPerPage },
  { "env", "STACK_MAX", .global_data.u32 = 128 * NumBytesPerPage },
  { "env", "ABORT", .global_data.u32 = 0 },
  { "env", "memoryBase", .global_data.u32 = 0 },
  { "env", "tableBase", .global_data.u32 = 0 },
#ifdef WASM_ENABLE_REPL
  { "spectest", "global_i32", .global_data.u32 = 666}
#endif
};

bool
wasm_native_global_lookup(const char *module_name, const char *global_name,
                          WASMGlobalImport *global)
{
  uint32 size = sizeof(native_global_defs)/sizeof(WASMNativeGlobalDef);
  WASMNativeGlobalDef *global_def = native_global_defs;
  WASMNativeGlobalDef *global_def_end = global_def + size;

  if (!module_name || !global_name || !global)
    return false;

  /* Lookup constant globals which can be defined by table */
  while (global_def < global_def_end) {
    if (!strcmp(global_def->module_name, module_name)
        && !strcmp(global_def->global_name, global_name)) {
      global->global_data_linked = global_def->global_data;
      return true;
    }
    global_def++;
  }

  /* Lookup non-constant globals which cannot be defined by table */
  if (!strcmp(module_name, "env")) {
    if (!strcmp(global_name, "_stdin")) {
      global->global_data_linked.addr = (uintptr_t)stdin;
      global->is_addr = true;
      return true;
    }
    else if (!strcmp(global_name, "_stdout")) {
      global->global_data_linked.addr = (uintptr_t)stdout;
      global->is_addr = true;
      return true;
    }
    else if (!strcmp(global_name, "_stderr")) {
      global->global_data_linked.addr = (uintptr_t)stderr;
      global->is_addr = true;
      return true;
    }
    else if (!strcmp(global_name, "_environ")) {
      global->global_data_linked.addr = (uintptr_t)environ;
      global->is_addr = true;
      return true;
    }
  }

  return false;
}

bool
wasm_native_init()
{
  /* TODO: qsort the function defs and global defs. */
  return true;
}


