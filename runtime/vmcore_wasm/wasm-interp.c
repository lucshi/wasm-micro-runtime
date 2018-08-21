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

#include "wasm-interp.h"
#include "wasm-runtime.h"
#include "wasm-thread.h"
#include "wasm-opcode.h"


typedef int32 CellType_I32;
typedef int64 CellType_I64;
typedef float32 CellType_F32;
typedef float64 CellType_F64;

#define REF_EMPTY 0
#define REF_I32   1
#define REF_F32   2
#define REF_I64_1 3
#define REF_I64_2 4
#define REF_F64_1 5
#define REF_F64_2 6

/**
 * Return the corresponding ref slot of the given address of local
 * variable or stack pointer.
 */
#define COMPUTE_FRAME_REF(ref, lp, p) (ref + (unsigned)((uint32 *)p - lp))

#define FRAME_REF_FOR(frame, p) COMPUTE_FRAME_REF(get_frame_ref(frame), frame->lp, p)

#define FRAME_REF(p) COMPUTE_FRAME_REF(frame_ref, frame_lp, p)

#define CLEAR_FRAME_REF(p, n) do {              \
    int ref_i;                                  \
    uint8 *ref = FRAME_REF(p);                  \
    for (ref_i = 0; ref_i < n; ref_i++)         \
      ref[ref_i] = 0;                           \
  } while (0)

/* 64-bit Memory accessors. */
#if WASM_CPU_SUPPORTS_UNALIGNED_64BIT_ACCESS != 0
#define PUT_I64_TO_ADDR(addr, value) do {       \
    *(int64*)(addr) = (int64)(value);           \
  } while (0)
#define PUT_F64_TO_ADDR(addr, value) do {       \
    *(float64*)(addr) = (float64)(value);       \
  } while (0)
#define GET_I64_FROM_ADDR(addr) (*(int64*)(addr))
#define GET_F64_FROM_ADDR(addr) (*(float64*)(addr))
#else  /* WASM_CPU_SUPPORTS_UNALIGNED_64BIT_ACCESS != 0 */
#define PUT_I64_TO_ADDR(addr, value) do {       \
    union { int64 val; uint32 parts[2]; } u;    \
    u.val = (value);                            \
    (addr)[0] = u.parts[0];                     \
    (addr)[1] = u.parts[1];                     \
  } while (0)
#define PUT_F64_TO_ADDR(addr, value) do {       \
    union { float64 val; uint32 parts[2]; } u;  \
    u.val = (value);                            \
    (addr)[0] = u.parts[0];                     \
    (addr)[1] = u.parts[1];                     \
  } while (0)

static inline int64
GET_I64_FROM_ADDR(uint32 *addr)
{
  union { int64 val; uint32 parts[2]; } u;
  u.parts[0] = addr[0];
  u.parts[1] = addr[1];
  return u.val;
}

static inline float64
GET_F64_FROM_ADDR (uint32 *addr)
{
  union { float64 val; uint32 parts[2]; } u;
  u.parts[0] = addr[0];
  u.parts[1] = addr[1];
  return u.val;
}
#endif  /* WASM_CPU_SUPPORTS_UNALIGNED_64BIT_ACCESS != 0 */


static inline WASMGlobalInstance*
get_global(const WASMModuleInstance *module, uint32 global_idx)
{
  if (global_idx >= module->global_count)
    return NULL;

  return module->globals + global_idx;
}

static inline uint32
get_global_addr(const WASMMemoryInstance *memory,
                const WASMGlobalInstance *global)
{
    return global->data_offset + NumBytesPerPage * memory->cur_page_count;
}

#define PUSH_I32(value) do {                    \
    *(int32*)frame_sp++ = (int32)(value);       \
    *FRAME_REF(frame_sp) = REF_I32;             \
  } while (0)

#define PUSH_F32(value) do {                    \
    *(float32*)frame_sp++ = (float32)(value);   \
    *FRAME_REF(frame_sp) = REF_F32;             \
  } while (0)

#define PUSH_I64(value) do {                    \
    PUT_I64_TO_ADDR(frame_sp, value);           \
    *FRAME_REF(frame_sp) = REF_I64_1;           \
    *FRAME_REF(frame_sp+1) = REF_I64_2;         \
    frame_sp += 2;                              \
  } while (0)

#define PUSH_F64(value) do {                    \
    PUT_F64_TO_ADDR(frame_sp, value);           \
    *FRAME_REF(frame_sp) = REF_F64_1;           \
    *FRAME_REF(frame_sp+1) = REF_F64_2;         \
    frame_sp += 2;                              \
  } while (0)

#define POP_I32() (--frame_sp, *FRAME_REF(frame_sp) = 0, *(int32*)frame_sp)

#define POP_F32() (--frame_sp, *FRAME_REF(frame_sp) = 0, *(float32*)frame_sp)

#define POP_I64() (frame_sp -= 2, *FRAME_REF(frame_sp) = *FRAME_REF(frame_sp+1) = 0,\
                   GET_I64_FROM_ADDR(frame_sp))

#define POP_F64() (frame_sp -= 2, *FRAME_REF(frame_sp) = *FRAME_REF(frame_sp+1) = 0,\
                   GET_F64_FROM_ADDR(frame_sp))

#define LOCAL_I32(n) (*(int32*)(frame_lp + n))

#define SET_LOCAL_I32(N, val) do {              \
    int n = (N);                                \
    frame_ref[n] = REF_I32;                     \
    *(int32*)(frame_lp + n) = (int32)(val);     \
  } while (0)

#define LOCAL_F32(n) (*(float32*)(frame_lp + n))

#define SET_LOCAL_F32(N, val) do {              \
    int n = (N);                                \
    frame_ref[n] = REF_F32;                     \
    *(float32*)(frame_lp + n) = (float32)(val); \
  } while (0)

#define LOCAL_I64(n) (GET_I64_FROM_ADDR(frame_lp + n))

#define SET_LOCAL_I64(N, val) do {              \
    int n = (N);                                \
    frame_ref[n] = REF_I64_1;                   \
    frame_ref[n + 1] = REF_I64_2;               \
    PUT_I64_TO_ADDR(frame_lp + n, val);         \
  } while (0)

#define LOCAL_F64(n) (GET_F64_FROM_ADDR (frame_lp + n))

#define SET_LOCAL_F64(N, val) do {              \
    int n = (N);                                \
    frame_ref[n] = REF_F64_1;                   \
    frame_ref[n + 1] = REF_F64_2;               \
    PUT_F64_TO_ADDR (frame_lp + n, val);        \
  } while (0)

/* Pop the given number of elements from the given frame's stack.  */
#define POP(N) do {                             \
    int n = (N);                                \
    frame_sp -= n;                              \
    CLEAR_FRAME_REF (frame_sp, n);              \
  } while (0)

#define SYNC_SP_IP() do {                       \
    frame->sp = frame_sp;                       \
    frame->ip = frame_ip;                       \
  } while (0)

#define UPDATE_SP() do {                        \
    frame_sp = frame->sp;                       \
  } while (0)

#define read_leb_uint64(p, p_end, res) do {     \
  uint32 _off = 0;                              \
  uint64 _res64;                                \
  if (!read_leb(p, p_end, &_off, 64, false,     \
                &_res64))                       \
    goto got_exception;                         \
  p += _off;                                    \
  res = (uint64)_res64;                         \
} while (0)

#define read_leb_uint32(p, p_end, res) do {     \
  uint32 _off = 0;                              \
  uint64 _res64;                                \
  if (!read_leb(p, p_end, &_off, 32, false,     \
                &_res64))                       \
    goto got_exception;                         \
  p += _off;                                    \
  res = (uint32)_res64;                         \
} while (0)

#define read_leb_int32(p, p_end, res) do {      \
  uint32 _off = 0;                              \
  uint64 _res64;                                \
  if (!read_leb(p, p_end, &_off, 32, true,      \
                &_res64))                       \
    goto got_exception;                         \
  p += _off;                                    \
  res = (int32)_res64;                          \
} while (0)

#define read_leb_uint8(p, p_end, res) do {      \
  uint32 _off = 0;                              \
  uint64 _res64;                                \
  if (!read_leb(p, p_end, &_off, 7, false,      \
                &_res64))                       \
    goto got_exception;                         \
  p += _off;                                    \
  res = (uint8)_res64;                          \
} while (0)

#define UNSUPPORTED_OPCODE() do {               \
    bh_assert (0);                              \
  } while (0)

#define RECOVER_CONTEXT(new_frame) do {                     \
    frame = (new_frame);                                    \
    cur_func = frame->function;                             \
    prev_frame = frame->prev_frame;                         \
    frame_ip = frame->ip;                                   \
    frame_ip_end = wasm_runtime_get_func_code_end(cur_func);\
    frame_lp = frame->lp;                                   \
    frame_sp = frame->sp;                                   \
    frame_ref = (uint8*)                                    \
     (frame->lp + cur_func->param_cell_num                  \
      + cur_func->local_cell_num);                          \
  } while (0)

static inline void
word_copy(uint32 *dest, uint32 *src, unsigned num)
{
  for (; num > 0; num--)
    *dest++ = *src++;
}

static inline WASMInterpFrame*
ALLOC_FRAME(WASMThread *self, uint32 size, WASMInterpFrame *prev_frame)
{
  WASMInterpFrame *frame = wasm_thread_alloc_wasm_frame(self, size);

  if (frame)
    frame->prev_frame = prev_frame;
  else {
    /* TODO: throw soe */
  }

  return frame;
}

static inline void
FREE_FRAME(WASMThread *self, WASMInterpFrame *frame)
{
  wasm_thread_free_wasm_frame(self, frame);
}

static void
wasm_interp_call_func_native(WASMThread *self,
                             WASMFunctionInstance *cur_func,
                             WASMInterpFrame *prev_frame)
{
  unsigned local_cell_num = 2;
  WASMInterpFrame *frame;
  typedef void (*F)(WASMThread*, uint32 *argv);
  union { F f; void *v; } u;
  uint32 argv_buf[128], *argv;

  if (!(frame = ALLOC_FRAME
        (self, wasm_interp_interp_frame_size(local_cell_num), prev_frame)))
    return;

  frame->function = cur_func;
  frame->ip = NULL;
  frame->sp = frame->lp + local_cell_num;

  wasm_thread_set_cur_frame (self, frame);

  argv = argv_buf; /* TODO: allocate memory if buf length is not enough. */
  word_copy(argv, frame->lp, cur_func->param_cell_num);

  u.v = cur_func->u.func_import->func_ptr_linked;
  u.f(self, argv);

  if (cur_func->ret_cell_num == 1) {
    prev_frame->sp[0] = argv[0];
    prev_frame->sp++;
  }
  else if (cur_func->ret_cell_num == 2) {
    prev_frame->sp[0] = argv[0];
    prev_frame->sp[1] = argv[1];
    prev_frame->sp += 2;
  }

  FREE_FRAME(self, frame);
  wasm_thread_set_cur_frame(self, prev_frame);
}

static void
wasm_interp_call_func_bytecode(WASMThread *self,
                               WASMFunctionInstance *cur_func,
                               WASMInterpFrame *prev_frame)
{
  WASMModuleInstance *module = self->vm_instance->module;
  WASMMemoryInstance *memory = module->default_memory;
  WASMTableInstance *table = module->default_table;
  uint8 opcode_IMPDEP2 = WASM_OP_IMPDEP2;
  WASMInterpFrame *frame = NULL;
  /* Points to this special opcode so as to jump to the
     call_method_from_entry.  */
  register uint8  *frame_ip = &opcode_IMPDEP2; /* cache of frame->ip */
  register uint32 *frame_lp = NULL;  /* cache of frame->lp */
  register uint32 *frame_sp = NULL;  /* cache of frame->sp */
  register uint8  *frame_ref = NULL; /* cache of frame->ref */
  uint8 *frame_ip_end = NULL;
  uint32 all_cell_num = 0;
  uint32 frame_size = 0;
  uint8 opcode;

  for (;;) {
    opcode = *frame_ip++;

    switch (opcode) {
      case WASM_OP_END:
        UNSUPPORTED_OPCODE();
        break;

      case WASM_OP_RETURN:
        {
          int i;
          for (i = 0; i < cur_func->ret_cell_num; i++)
            *prev_frame->sp++ = *--frame_sp;
          goto return_func;
        }

      case WASM_OP_CALL:
        {
          uint32 func_idx;

          read_leb_uint32(frame_ip, frame_ip_end, func_idx);
          if (func_idx >= module->function_count)
            goto got_exception;

          cur_func = module->functions + func_idx;
          goto call_func_from_interp;
        }

      case WASM_OP_CALL_INDIRECT:
        /* TODO */
        UNSUPPORTED_OPCODE();
        break;

      case WASM_OP_GET_LOCAL:
        {
          uint32 local_idx;

          read_leb_uint32(frame_ip, frame_ip_end, local_idx);
          /* TODO: check local_idx, check local type */
          PUSH_I32(LOCAL_I32(local_idx));
          break;
        }

      case WASM_OP_SET_LOCAL:
        {
          uint32 local_idx;

          read_leb_uint32(frame_ip, frame_ip_end, local_idx);
          /* TODO: check local_idx, check local type */
          SET_LOCAL_I32(local_idx, POP_I32());
          break;
        }

      case WASM_OP_TEE_LOCAL:
        /* TODO */
        UNSUPPORTED_OPCODE();
        break;

      case WASM_OP_GET_GLOBAL:
        {
          WASMGlobalInstance *global;
          uint32 global_idx;

          read_leb_uint32(frame_ip, frame_ip_end, global_idx);
          if (!(global = get_global(module, global_idx)))
            goto got_exception;

          PUSH_I32(get_global_addr(memory, global));
          break;
        }

      case WASM_OP_SET_GLOBAL:
        /* TODO */
        UNSUPPORTED_OPCODE();
        break;

      case WASM_OP_I32_LOAD:
        {
          uint32 offset, flags, addr;

          read_leb_uint32(frame_ip, frame_ip_end, flags);
          read_leb_uint32(frame_ip, frame_ip_end, offset);
          addr = POP_I32();
          /* TODO: check addr, flags */
          PUSH_I32(*(uint32*)(memory->base_addr + addr + offset));
          (void)flags;
          break;
        }

      case WASM_OP_I32_CONST:
        {
          uint32 value;

          read_leb_uint32(frame_ip, frame_ip_end, value);
          PUSH_I32(value);
          break;
        }


      case WASM_OP_IMPDEP2:
        frame = prev_frame;
        frame_ip = frame->ip;
        frame_sp = frame->sp;
        goto call_func_from_entry;

      default:
        UNSUPPORTED_OPCODE();
    }

    continue;

  call_func_from_interp:
    /* Only do the copy when it's called from interpreter.  */
    {
      WASMInterpFrame *outs_area = wasm_thread_wasm_stack_top(self);
      POP(cur_func->param_cell_num);
      SYNC_SP_IP();
      word_copy(outs_area->lp, frame_sp, cur_func->param_cell_num);
      prev_frame = frame;
    }

  call_func_from_entry:
    {
      if (cur_func->is_import_func) {
        wasm_interp_call_func_native(self, cur_func, prev_frame);
        prev_frame = frame->prev_frame;
        cur_func = frame->function;
        UPDATE_SP();

        /* TODO: check exception */
      }
      else {
        all_cell_num = cur_func->param_cell_num + cur_func->local_cell_num
                       + self->stack_cell_num;
        frame_size = wasm_interp_interp_frame_size(all_cell_num);

        if (!(frame = ALLOC_FRAME(self, frame_size, prev_frame))) {
          frame = prev_frame;
          goto got_exception;
        }

        /* Initialize the interpreter context. */
        frame->function = cur_func;
        frame_ip = wasm_runtime_get_func_code(cur_func);
        frame_ip_end = wasm_runtime_get_func_code_end(cur_func);
        frame_lp = frame->lp;
        frame_sp = frame_lp + cur_func->param_cell_num
                   + cur_func->local_cell_num;
        frame_ref = (uint8*)(frame_sp + self->stack_cell_num);

        wasm_thread_set_cur_frame(self, (WASMRuntimeFrame*)frame);
      }
      continue;
    }

  return_func:
    {
      FREE_FRAME(self, frame);
      wasm_thread_set_cur_frame(self, (WASMRuntimeFrame*)prev_frame);

      if (!prev_frame->ip)
        /* Called from native. */
        return;

      RECOVER_CONTEXT(prev_frame);
      continue;
    }

  got_exception:
    printf("WASM Interpreter failed: got exception.\n");
    return;
  }
  (void)table;
}

void
wasm_interp_call_wasm(WASMFunctionInstance *function,
                      uint32 argc, uint32 argv[])
{
  WASMThread *self = wasm_runtime_get_self();
  WASMRuntimeFrame *prev_frame = wasm_thread_get_cur_frame(self);
  WASMInterpFrame *frame, *outs_area;
  /* Allocate sufficient cells for all kinds of return values.  */
  unsigned all_cell_num = 2;
  /* This frame won't be used by JITed code, so only allocate interp
     frame here.  */
  unsigned frame_size = wasm_interp_interp_frame_size(all_cell_num);

  bh_assert(argc == function->param_cell_num);

  /* TODO: check stack overflow. */

  if (!(frame = ALLOC_FRAME(self, frame_size, (WASMInterpFrame*)prev_frame)))
    return;

  outs_area = wasm_thread_wasm_stack_top(self);
  frame->function = NULL;
  frame->ip = NULL;
  /* There is no local variable. */
  frame->sp = frame->lp + 0;

  if (argc > 0)
    word_copy(outs_area->lp, argv, argc);

  wasm_thread_set_cur_frame(self, frame);

  if (function->is_import_func)
    wasm_interp_call_func_native(self, function, frame);
  else
    wasm_interp_call_func_bytecode(self, function, frame);

  /* TODO: check exception */

  wasm_thread_set_cur_frame(self, prev_frame);
  FREE_FRAME(self, frame);
}
