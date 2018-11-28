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
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>


#define MEMORY(self) (self->vm_instance->module->default_memory)
#define MEMORY_BASE(self) (MEMORY(self)->memory_data)

#define DEBUG_PRINT 0

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

#if WASM_ENABLE_EMCC_LIBC != 0
#ifdef __i386__
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
  *args = bh_vprintf(fmt, va_args);
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
_fprintf_wrapper(WASMThread *self, uint32 *args)
{
  FILE *stream = (FILE*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = get_va_list(args + 2);
  *args = vfprintf(stream, fmt, va_args);
}

static void
_scanf_wrapper(WASMThread *self, uint32 *args)
{
  const char *fmt = (const char*)args[0];
  va_list va_args = get_va_list(args + 1);
  *args = vscanf(fmt, va_args);
}

static void
_fscanf_wrapper(WASMThread *self, uint32 *args)
{
  FILE *stream = (FILE*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = get_va_list(args + 2);
  *args = vfscanf(stream, fmt, va_args);
}

static void
_sscanf_wrapper(WASMThread *self, uint32 *args)
{
  char *str= (char*)args[0];
  const char *fmt = (const char*)args[1];
  va_list va_args = get_va_list(args + 2);
  *args = vsscanf(str, fmt, va_args);
}
#endif

static void
_malloc_wrapper(WASMThread *self, uint32 *args)
{
  *(uintptr_t*)args = (uintptr_t)bh_malloc(args[0]);
}

static void
_calloc_wrapper(WASMThread *self, uint32 *args)
{
  char *ptr;
  uint32 total_size = args[0] * args[1];

  if (total_size < args[0] || total_size < args[1]) {
    wasm_runtime_set_exception("calloc failed: memory size out of range.");
    return;
  }

  if ((ptr = bh_malloc(total_size)))
    memset(ptr, 0, total_size);
  *(uintptr_t*)args = (uintptr_t)ptr;
}

static void
_free_wrapper(WASMThread *self, uint32 *args)
{
  void *ptr;
  #ifdef __i386__
    ptr = (void*)args[0];
  #elif __x86_64__
    memcpy(&ptr, args, 8);
  #endif
  bh_free(ptr);
}

static void
nullFunc_X_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.nullFunc_X(%i)", code);
  wasm_runtime_set_exception(buf);
}

void*
vmci_get_std_cout();

static void
_cout_wrapper(WASMThread *self, uint32 *args)
{
  *(uintptr_t*)args = (uintptr_t)vmci_get_std_cout();
}

static void
_stdout_wrapper(WASMThread *self, uint32 *args)
{
  *(uintptr_t*)args = (uintptr_t)stdout;
}

static void
_stderr_wrapper(WASMThread *self, uint32 *args)
{
  *(uintptr_t*)args = (uintptr_t)stderr;
}

static void
_atexit_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO: implement callback for atexit */
  wasm_runtime_set_exception("atexit unsupported");
}
#endif

#if WASM_ENABLE_EMCC_LIBC != 0 || WASM_ENABLE_EMCC_SYSCALL != 0
static void
abort_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.abort(%i)", code);
  wasm_runtime_set_exception(buf);
}

static void
abortStackOverflow_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.abortStackOverflow(%i)", code);
  wasm_runtime_set_exception(buf);
}
#endif

#if WASM_ENABLE_WASMCEPTION != 0 || WASM_ENABLE_EMCC_SYSCALL != 0
#if WASM_ENABLE_WASMCEPTION != 0
static void
__syscall_wrapper(WASMThread *self, uint32 *args)
{
  printf("##_syscall called, args[0]: %d\n", args[0]);
}
#endif

static void
__syscall0_wrapper(WASMThread *self, uint32 *args)
{
  switch (args[0]) {
    case 199: /* getuid */
      *args = (uint32)getuid();
      break;
    default:
      printf("##_syscall0 called, args[0]: %d\n", args[0]);
  }
}

static void
__syscall1_wrapper(WASMThread *self, uint32 *args)
{
  switch (args[0]) {
    case 6: /* close */
      {
        *args = close(args[1]);
        break;
      }
    default:
      printf("##_syscall1 called, args[0]: %d\n", args[0]);
  }
}

static void
__syscall2_wrapper(WASMThread *self, uint32 *args)
{
  switch (args[0]) {
    case 183: /* getcwd */
      {
        char *buf = args[1] > 0
          ? (char*)(MEMORY_BASE(self) + args[1]) : NULL;
        size_t size = args[2];
        if (getcwd(buf, size))
          *args = args[1];
        else
          *args = 0;
        break;
      }
    default:
      printf("##_syscall2 called, args[0]: %d\n", args[0]);
  }
}

static void
__syscall3_wrapper(WASMThread *self, uint32 *args)
{
  switch (args[0]) {
    case 3: /* read*/
      {
        int fd = args[1];
        char *buf = args[2] ? (char*)(MEMORY_BASE(self) + args[2]) : NULL;
        int count = args[3];
        *args = read(fd, buf, count);
        break;
      }

    case 5: /* open */
      {
        char *path = args[1] ? (char*)(MEMORY_BASE(self) + args[1]) : NULL;
        int flags = args[2];
        mode_t mode = args[3];
        *args = open(path, flags, mode);
        break;
      }

    case 54: /* ioctl */
      {
        struct winsize *wsz = (struct winsize*)(MEMORY_BASE(self) + args[3]);
        *args = ioctl(args[1], args[2], wsz);
        break;
      }

    case 145: /* readv */
    case 146: /* writev */
      {
        uint32 iovcnt = args[3], i;
        struct iovec *vec_begin, *vec;
#ifdef __i386__
        vec_begin = vec = (struct iovec*)(MEMORY_BASE(self) + args[2]);
        for (i = 0; i < iovcnt; i++, vec++) {
          if (vec->iov_len > 0) {
            vec->iov_base = MEMORY_BASE(self) + (uint32)vec->iov_base;
          }
        }
#elif __x86_64__
        vec_begin = vec = (struct iovec*)bh_malloc(sizeof(struct iovec) * iovcnt);
        memset(vec, 0, sizeof(struct iovec) * iovcnt);
        for (i = 0; i < iovcnt; i++, vec++) {
          vec->iov_base = (void*)(uintptr_t)(*(uint32*)(MEMORY_BASE(self) + args[2] + 8 * i));
          vec->iov_len = *(uint32*)(MEMORY_BASE(self) + args[2] + 8 * i + 4);
          if (vec->iov_len > 0) {
            vec->iov_base = MEMORY_BASE(self) + (uint64)vec->iov_base;
          }
        }
#endif
        if (args[0] == 145)
          *args = readv(args[1], vec_begin, args[3]);
        else
          *args = writev(args[1], vec_begin, args[3]);
#ifdef __x86_64__
        bh_free(vec_begin);
#endif
        break;
      }

    case 221: /* fcntl */
      {
        int fd = args[1];
        int cmd = args[2];
        char *arg = args[3] ? (char*)(MEMORY_BASE(self) + args[3]) : NULL;
        *args = fcntl(fd, cmd, arg);
        break;
      }

    default:
      printf("##_syscall3 called, args[0]: %d\n", args[0]);
  }
}

#if WASM_ENABLE_WASMCEPTION != 0
static void
__syscall4_wrapper(WASMThread *self, uint32 *args)
{
  printf("##_syscall4 called, args[0]: %d\n", args[0]);
}
#endif

static void
__syscall5_wrapper(WASMThread *self, uint32 *args)
{
  switch (args[0]) {
    case 140: /* llseek */
      {
        unsigned int fd = args[1];
        unsigned long offset_high = args[2];
        unsigned long offset_low = args[3];
        loff_t *result = args[4] ? (loff_t*)(MEMORY_BASE(self) + args[4]) : NULL;
        unsigned int whence = args[5];

        *args = syscall(140, fd, offset_high, offset_low, result, whence);
        break;
      }

    default:
      printf("##_syscall5 called, args[0]: %d\n", args[0]);
  }
}
#endif /* end of WASM_ENABLE_WASMCEPTION || WASM_ENABLE_EMCC_SYSCALL */

#if WASM_ENABLE_EMCC_SYSCALL != 0

#define EMCC_SYSCALL_WRAPPER(id, argc)                      \
static void                                                 \
___syscall##id##_wrapper(WASMThread *self, uint32 *args) {  \
    uint32 argv[argc + 1] = { id }, *ptr;                   \
    ptr = (uint32*)(MEMORY_BASE(self) + args[1]);           \
    memcpy(argv + 1, ptr, sizeof(uint32) * argc);           \
    __syscall##argc##_wrapper(self, argv);                  \
    *args = *argv;                                          \
}

#define EMCC_SYSCALL_WRAPPER0(id) EMCC_SYSCALL_WRAPPER(id, 0)
#define EMCC_SYSCALL_WRAPPER1(id) EMCC_SYSCALL_WRAPPER(id, 1)
#define EMCC_SYSCALL_WRAPPER2(id) EMCC_SYSCALL_WRAPPER(id, 2)
#define EMCC_SYSCALL_WRAPPER3(id) EMCC_SYSCALL_WRAPPER(id, 3)
#define EMCC_SYSCALL_WRAPPER4(id) EMCC_SYSCALL_WRAPPER(id, 4)
#define EMCC_SYSCALL_WRAPPER5(id) EMCC_SYSCALL_WRAPPER(id, 5)

EMCC_SYSCALL_WRAPPER0(199)

EMCC_SYSCALL_WRAPPER1(6)

EMCC_SYSCALL_WRAPPER2(183)

EMCC_SYSCALL_WRAPPER3(3)
EMCC_SYSCALL_WRAPPER3(5)
EMCC_SYSCALL_WRAPPER3(54)
EMCC_SYSCALL_WRAPPER3(145)
EMCC_SYSCALL_WRAPPER3(146)
EMCC_SYSCALL_WRAPPER3(221)

EMCC_SYSCALL_WRAPPER5(140)

static void
_abort_wrapper(WASMThread *self, uint32 *args)
{
  int32 code = args[0];
  char buf[32];

  snprintf(buf, sizeof(buf), "env.abort(%i)", code);
  wasm_runtime_set_exception(buf);
}

static void
___lock_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
___unlock_wrapper(WASMThread *self, uint32 *args)
{
  /* TODO */
}

static void
getTotalMemory_wrapper(WASMThread *self, uint32 *args)
{
  WASMMemoryInstance *memory = self->vm_instance->module->default_memory;
  *args = NumBytesPerPage * memory->cur_page_count;
}

static void
enlargeMemory_wrapper(WASMThread *self, uint32 *args)
{
  bool ret;
  WASMMemoryInstance *memory = self->vm_instance->module->default_memory;
  uint32 DYNAMICTOP_PTR_offset = self->vm_instance->module->DYNAMICTOP_PTR_offset;
  uint32 addr_data_offset = *(uint32*)(memory->global_data + DYNAMICTOP_PTR_offset);
  uint32 *DYNAMICTOP_PTR = (uint32*)(memory->memory_data + addr_data_offset);
  uint32 memory_size_expected = *DYNAMICTOP_PTR;
  uint32 total_page_count = (memory_size_expected + NumBytesPerPage - 1) / NumBytesPerPage;

  if (total_page_count < memory->cur_page_count) {
    *args = 1;
    return;
  }
  else {
    ret = wasm_runtime_enlarge_memory(self->vm_instance->module, total_page_count -
                                      memory->cur_page_count);
    *args = ret ? 1 : 0;
    return;
  }
}

static void
abortOnCannotGrowMemory_wrapper(WASMThread *self, uint32 *args)
{
  wasm_runtime_set_exception("abort on cannot grow memory");
}

static void
___setErrNo_wrapper(WASMThread *self, uint32 *args)
{
  errno = *args;
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
#endif

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
#if WASM_ENABLE_EMCC_LIBC != 0 || WASM_ENABLE_EMCC_SYSCALL != 0
  REG_NATIVE_FUNC(env, abort),
  REG_NATIVE_FUNC(env, abortStackOverflow),
#endif
#if WASM_ENABLE_EMCC_LIBC != 0
  REG_NATIVE_FUNC(env, nullFunc_X),
  REG_NATIVE_FUNC(env, _atexit),
#ifdef __i386__
  REG_NATIVE_FUNC(env, _printf),
  REG_NATIVE_FUNC(env, _sprintf),
  REG_NATIVE_FUNC(env, _snprintf),
  REG_NATIVE_FUNC(env, _fprintf),
  REG_NATIVE_FUNC(env, _scanf),
  REG_NATIVE_FUNC(env, _fscanf),
  REG_NATIVE_FUNC(env, _sscanf),
#endif
  REG_NATIVE_FUNC(env, _malloc),
  REG_NATIVE_FUNC(env, _calloc),
  REG_NATIVE_FUNC(env, _free),
  { "env", "g$__ZSt4cout", _cout_wrapper },
  { "env", "g$_stdout", _stdout_wrapper },
  { "env", "g$_stderr", _stderr_wrapper },
#endif
#if WASM_ENABLE_EMCC_SYSCALL != 0
  REG_NATIVE_FUNC(env, ___syscall3),
  REG_NATIVE_FUNC(env, ___syscall5),
  REG_NATIVE_FUNC(env, ___syscall6),
  REG_NATIVE_FUNC(env, ___syscall54),
  REG_NATIVE_FUNC(env, ___syscall140),
  REG_NATIVE_FUNC(env, ___syscall145),
  REG_NATIVE_FUNC(env, ___syscall146),
  REG_NATIVE_FUNC(env, ___syscall183),
  REG_NATIVE_FUNC(env, ___syscall199),
  REG_NATIVE_FUNC(env, ___syscall221),
  REG_NATIVE_FUNC(env, _abort),
  REG_NATIVE_FUNC(env, abortOnCannotGrowMemory),
  REG_NATIVE_FUNC(env, enlargeMemory),
  REG_NATIVE_FUNC(env, getTotalMemory),
  REG_NATIVE_FUNC(env, ___lock),
  REG_NATIVE_FUNC(env, ___unlock),
  REG_NATIVE_FUNC(env, _emscripten_memcpy_big),
  REG_NATIVE_FUNC(env, ___setErrNo),
#endif
#if WASM_ENABLE_WASMCEPTION != 0
  REG_NATIVE_FUNC(env, __syscall),
  REG_NATIVE_FUNC(env, __syscall0),
  REG_NATIVE_FUNC(env, __syscall1),
  REG_NATIVE_FUNC(env, __syscall2),
  REG_NATIVE_FUNC(env, __syscall3),
  REG_NATIVE_FUNC(env, __syscall4),
  REG_NATIVE_FUNC(env, __syscall5),
#endif
#ifdef WASM_ENABLE_REPL
  REG_NATIVE_FUNC(spectest, print_i32),
  REG_NATIVE_FUNC(spectest, print),
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

typedef struct WASMNativeGlobalDef {
  const char *module_name;
  const char *global_name;
  bool is_addr;
  WASMValue global_data;
} WASMNativeGlobalDef;

static WASMNativeGlobalDef native_global_defs[] = {
#if WASM_ENABLE_EMCC_SYSCALL != 0 || WASM_ENABLE_EMCC_LIBC != 0
  { "env", "memoryBase", false, .global_data.i32 = 0 },
  { "env", "__memory_base", false, .global_data.i32 = 0 },
  { "env", "tableBase", false, .global_data.i32 = 0 },
  { "env", "__table_base", false, .global_data.i32 = 0 },
  { "env", "STACKTOP", false, .global_data.i32 = 0 },
  { "env", "STACK_MAX", false, .global_data.i32 = 0 },
  { "env", "DYNAMICTOP_PTR", true, .global_data.addr = 0 },
  { "env", "tempDoublePtr", true, .global_data.addr = 0 },
  { "global", "NaN", false, .global_data.u64 = 0x7FF8000000000000LL },
  { "global", "Infinity", false, .global_data.u64 = 0x7FF0000000000000LL },
#endif

#ifdef WASM_ENABLE_REPL
  { "spectest", "global_i32", false, .global_data.u32 = 666}
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
      global->is_addr = global_def->is_addr;
      global->global_data_linked = global_def->global_data;
      return true;
    }
    global_def++;
  }

  return false;
}

bool
wasm_native_init()
{
  /* TODO: qsort the function defs and global defs. */
  return true;
}

