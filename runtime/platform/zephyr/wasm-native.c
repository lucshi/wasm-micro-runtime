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

#include "wasm-native.h"
#include "wasm-runtime.h"
#include "wasm_log.h"
#include "wasm_memory.h"
#include "wasm_platform_log.h"


#if 0
#define MEMORY(self) (self->module_inst->default_memory)
#define MEMORY_BASE(self) (MEMORY(self)->memory_data)
#else
#define MEMORY_BASE(self) (0)
#endif

static void
abort_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.abort(%i)", code);
  wasm_runtime_set_exception(self->module_inst, buf);
}

static inline va_list
get_va_list(uint32 *args)
{
  union { uint32 u; va_list v; } u;
  u.u  = args[0];
  return u.v;
}

static void
_printf_wrapper(WASMThread *self, uint32 *args)
{
  const char *fmt = (const char*)args[0];
  va_list va_args = get_va_list(args + 1);
  *args = wasm_vprintf(fmt, va_args);
}

static void
_sprintf_wrapper(WASMThread *self, uint32 *args)
{
  char *str = (char*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = get_va_list(args + 2);
  *args = vsprintf(str, fmt, va_args);
}

static void
_snprintf_wrapper(WASMThread *self, uint32 *args)
{
  char *str = (char*)args[0];
  size_t size = args[1];
  const char *fmt = (const char*)args[2];
  va_list va_args = get_va_list(args + 3);
  *args = vsnprintf(str, size, fmt, va_args);
}

static void
_puts_wrapper(WASMThread *self, uint32 *args)
{
  *args = (uint32)printf("%s\n", (char*)args[0]);
}

static void
_malloc_wrapper(WASMThread *self, uint32 *args)
{
  *args = (uint32)wasm_malloc(args[0]);
}

static void
_calloc_wrapper(WASMThread *self, uint32 *args)
{
  char *ptr;
  uint32 total_size = args[0] * args[1];

  if (total_size < args[0] || total_size < args[1]) {
    wasm_runtime_set_exception(self->module_inst,
        "calloc failed: memory size out of range.");
    return;
  }

  if ((ptr = wasm_malloc(total_size)))
    memset(ptr, 0, total_size);

  *args = (uint32)ptr;
}

static void
_free_wrapper(WASMThread *self, uint32 *args)
{
  wasm_free((void*)args[0]);
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

static void
abortStackOverflow_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.abortStackOverflow(%i)", code);
  wasm_runtime_set_exception(self->module_inst, buf);
}

static void
nullFunc_X_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.nullFunc_X(%i)", code);
  wasm_runtime_set_exception(self->module_inst, buf);
}


/* TODO: add function parameter/result types check */
#define REG_NATIVE_FUNC(module_name, func_name) \
    {#module_name, #func_name, func_name##_wrapper}

typedef struct WASMNativeFuncDef {
  const char *module_name;
  const char *func_name;
  void (*func_ptr)(WASMThread *self, uint32 *args);
} WASMNativeFuncDef;

static WASMNativeFuncDef native_func_defs[] = {
  REG_NATIVE_FUNC(env, abort),
  REG_NATIVE_FUNC(env, _printf),
  REG_NATIVE_FUNC(env, _sprintf),
  REG_NATIVE_FUNC(env, _snprintf),
  REG_NATIVE_FUNC(env, _puts),
  REG_NATIVE_FUNC(env, _malloc),
  REG_NATIVE_FUNC(env, _calloc),
  REG_NATIVE_FUNC(env, _free),
  REG_NATIVE_FUNC(env, _emscripten_memcpy_big),
  REG_NATIVE_FUNC(env, abortStackOverflow),
  REG_NATIVE_FUNC(env, nullFunc_X),
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
  { "env", "__memory_base", .global_data.u32 = 0 },
  { "env", "tableBase", .global_data.u32 = 0 },
  { "env", "__table_base", .global_data.u32 = 0 },
  { "env", "DYNAMICTOP_PTR", .global_data.addr = 0 },
  { "env", "tempDoublePtr", .global_data.addr = 0 },
  { "global", "NaN", .global_data.u64 = 0x7FF8000000000000LL },
  { "global", "Infinity", .global_data.u64 = 0x7FF0000000000000LL },
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
  }

  return false;
}

bool
wasm_native_init()
{
  /* TODO: qsort the function defs and global defs. */
  return true;
}


