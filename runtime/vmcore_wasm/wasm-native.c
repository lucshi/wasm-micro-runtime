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
#include "bh_memory.h"

#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define MEMORY(self) (self->vm_instance->module->default_memory)
#define MEMORY_BASE(self) (MEMORY(self)->base_addr)

static void
_getc_wrapper(WASMThread *self, uint32 *args)
{
  int32 file = args[0];
  int32 ret = getc((void*)file);
  *args = ret;
}

static void
_ungetc_wrapper(WASMThread *self, uint32 *args)
{
  int32 character = args[0];
  int32 file = args[1];
  int32 ret = ungetc(character, (void*)file);
  *args = ret;
}

static void
_fread_wrapper(WASMThread *self, uint32 *args)
{
  uint8 *memory_base = MEMORY_BASE(self);
  int32 off = args[0];
  int32 size = args[1];
  int32 nmemb = args[2];
  int32 file = args[3];
  int32 ret = fread(memory_base + off, size, nmemb, (void*)file);
  *args = ret;
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
  int32 character = args[0];
  int32 file = args[1];
  int32 ret = fputc(character, (void*)file);
  *args = ret;
}

static void
_fflush_wrapper(WASMThread *self, uint32 *args)
{
  int32 file = args[0];
  int32 ret = fflush((void*)file);
  *args = ret;
}

static void
abort_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  printf("env.abort(%i)\n",code);
  abort();
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
    printf("___syscall140_wrapper execute failed.\n");
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
    printf("alloc iovec failed in native function lookup.\n");
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
  REG_NATIVE_FUNC(env, _fflush),
  REG_NATIVE_FUNC(env, abort),
  REG_NATIVE_FUNC(env, ___syscall140),
  REG_NATIVE_FUNC(env, ___syscall146),
  REG_NATIVE_FUNC(env, ___syscall54),
  REG_NATIVE_FUNC(env, ___syscall6),
  REG_NATIVE_FUNC(env, _emscripten_memcpy_big)
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
  { "env", "STACKTOP", (uintptr_t)64 * NumBytesPerPage },
  { "env", "STACK_MAX", 128 * NumBytesPerPage },
  { "env", "ABORT", 0 },
  { "env", "memoryBase", 1024 },
  { "env", "tableBase", 0 }
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

bool
wasm_native_init()
{
  /* TODO: qsort the function defs and global defs. */
  return true;
}

