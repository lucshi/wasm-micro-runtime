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
#include "wasm-thread.h"


#define MEMORY(self) (self->vm_instance->module->default_memory)
#define MEMORY_BASE(self) (MEMORY(self)->base_addr)

static void
_getc_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
_ungetc_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
_fread_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
_fwrite_wrapper(WASMThread *self, uint32 *args)
{
  uint8 *memory_base = MEMORY_BASE(self);
  int32 off = args[0];
  int32 size = args[1];
  int32 nmemb = args[2];
  int32 file = args[3];
  int32 ret = fwrite(memory_base + off, size, nmemb, (void*)file);
  *args = ret;
}

static void
_fputc_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
_fflush_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
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
  REG_NATIVE_FUNC(env, _getc),
  REG_NATIVE_FUNC(env, _ungetc),
  REG_NATIVE_FUNC(env, _fread),
  REG_NATIVE_FUNC(env, _fwrite),
  REG_NATIVE_FUNC(env, _fputc),
  REG_NATIVE_FUNC(env, _fflush)
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
  uintptr_t global_data;
} WASMNativeGlobalDef;

static WASMNativeGlobalDef native_global_defs[] = {
  { "env", "_STACKTOP", (uintptr_t)64 * NumBytesPerPage },
  { "env", "_STACK_MAX", 128 * NumBytesPerPage },
  { "env", "_ABORT", 0 },
  { "env", "_memoryBase", 1024 },
  { "env", "_tableBase", 0 }
};

void*
wasm_native_global_lookup(const char *module_name, const char *global_name)
{
  uint32 size = sizeof(native_global_defs)/sizeof(WASMNativeGlobalDef);
  WASMNativeGlobalDef *global_def = native_global_defs;
  WASMNativeGlobalDef *global_def_end = global_def + size;

  if (!module_name || !global_name)
    return NULL;

  /* Lookup constant globals which can be defined by table */
  while (global_def < global_def_end) {
    if (!strcmp(global_def->module_name, module_name)
        && !strcmp(global_def->global_name, global_name))
      return &global_def->global_data;
    global_def++;
  }

  /* Lookup non-constant globals which cannot be defined by table */
  if (!strcmp(module_name, "env")) {
    if (!strcmp(global_name, "_stdin"))
      return &stdin;
    else if (!strcmp(global_name, "_stdout"))
      return &stdout;
    else if (!strcmp(global_name, "_stderr"))
      return &stderr;
    if (!strcmp(global_name, "_environ"))
      return &environ;
  }

  return NULL;
}
