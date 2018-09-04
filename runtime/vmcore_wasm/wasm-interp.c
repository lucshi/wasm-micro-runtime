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
#include "wasm-loader.h"
#include "bh_memory.h"

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

#define BR_TABLE_TMP_BUF_LEN 32

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

static inline uint8*
get_global_addr(WASMMemoryInstance *memory, WASMGlobalInstance *global)
{
  return memory->global_data + global->data_offset;
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

#define PUSH_CSP(type, start, else, end) do {   \
    if (frame_csp >= frame_csp_boundary)        \
      goto got_exception;                       \
    frame_csp->block_type = type;               \
    frame_csp->start_addr = start;              \
    frame_csp->else_addr = else;                \
    frame_csp->end_addr = end;                  \
    frame_csp->frame_sp = frame_sp;             \
    frame_csp++;                                \
  } while (0)

#define POP_I32() (--frame_sp, *FRAME_REF(frame_sp) = 0, *(int32*)frame_sp)

#define POP_F32() (--frame_sp, *FRAME_REF(frame_sp) = 0, *(float32*)frame_sp)

#define POP_I64() (frame_sp -= 2, *FRAME_REF(frame_sp) = *FRAME_REF(frame_sp+1) = 0,\
                   GET_I64_FROM_ADDR(frame_sp))

#define POP_F64() (frame_sp -= 2, *FRAME_REF(frame_sp) = *FRAME_REF(frame_sp+1) = 0,\
                   GET_F64_FROM_ADDR(frame_sp))

#define POP_CSP_CHECK_OVERFLOW() do {                          \
    if (frame_csp <= frame->csp_bottom)                        \
      goto got_exception;                                      \
  } while (0)

#define POP_CSP() do {                                         \
    --frame_csp;                                               \
    frame_sp = frame_csp->frame_sp;                            \
    POP_CSP_CHECK_OVERFLOW();                                  \
  } while (0)

#define POP_CSP_N(n) do {                                      \
    frame_csp -= n;                                            \
    frame_sp = frame_csp->frame_sp;                            \
    POP_CSP_CHECK_OVERFLOW();                                  \
  } while (0)

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
    frame->csp = frame_csp;                     \
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

#define RECOVER_CONTEXT(new_frame) do {                              \
    frame = (new_frame);                                             \
    cur_func = frame->function;                                      \
    prev_frame = frame->prev_frame;                                  \
    frame_ip = frame->ip;                                            \
    frame_ip_end = wasm_runtime_get_func_code_end(cur_func);         \
    frame_lp = frame->lp;                                            \
    frame_sp = frame->sp;                                            \
    frame_csp = frame->csp;                                          \
    frame_csp_boundary = frame->csp_boundary;                        \
    frame_csp_bottom = frame->csp_bottom;                            \
    frame_ref = (uint8*)(frame_lp + cur_func->param_cell_num +       \
    cur_func->local_cell_num + self->stack_cell_num +                \
    self->block_cell_num);                                           \
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
  WASMBranchBlock *frame_csp_bottom = NULL;
  WASMBranchBlock *frame_csp = NULL;
  WASMBranchBlock *frame_csp_boundary = NULL;
  uint8 *frame_ip_end = frame_ip + 1;
  uint8 opcode, block_type;
  uint32 *depths = NULL;
  uint32 depth_buf[BR_TABLE_TMP_BUF_LEN];
  uint32 i, depth, cond, count, fidx, tidx, frame_size = 0, all_cell_num = 0;
  int32 didx, val;
  uint8 *else_addr, *end_addr;

  while (frame_ip < frame_ip_end) {
    opcode = *frame_ip++;
    switch (opcode) {
      /* control instructions */
      case WASM_OP_UNREACHABLE:
        printf("wasm interp failed: opcode is unreachable.\n");
        goto got_exception;

      case WASM_OP_NOP:
        break;

      case WASM_OP_BLOCK:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, block_type);

        if (!wasm_loader_find_block_addr(module->branch_set, frame_ip,
                                         frame_ip_end, block_type, &else_addr,
                                         &end_addr)) {
          goto got_exception;
        }

        PUSH_CSP(BLOCK_TYPE_BLOCK, frame_ip, NULL, end_addr);
        frame_ip = end_addr + 1;
        break;

      case WASM_OP_LOOP:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, block_type);
        PUSH_CSP(BLOCK_TYPE_LOOP, frame_ip, NULL, NULL);
        break;

      case WASM_OP_IF:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, block_type);

        if (!wasm_loader_find_block_addr(module->branch_set, frame_ip,
                                         frame_ip_end, block_type, &else_addr,
                                         &end_addr)) {
          goto got_exception;
        }

        PUSH_CSP(BLOCK_TYPE_IF, frame_ip, else_addr, end_addr);
        cond = POP_I32();
        /* condition of the if branch is false, else condition is met */
        if (cond == 0) {
          /* if there is no else branch, go to the end addr */
          if (else_addr == NULL) {
            POP_CSP();
            frame_ip = end_addr + 1;
          }
          /* if there is an else branch, go to the else addr */
          else
            frame_ip = else_addr + 1;
        }
        break;

      case WASM_OP_ELSE:
        /* TODO: test */
        /* comes from the if branch in WASM_OP_IF */
        frame_ip = frame_csp->end_addr;
        break;

      case WASM_OP_END:
        POP_CSP();
        break;

      case WASM_OP_BR:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, depth);
        POP_CSP_N(depth);
        frame_ip = frame_csp->start_addr;
        break;

      case WASM_OP_BR_IF:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, depth);
        cond = POP_I32();
        if (cond) {
          POP_CSP_N(depth);
          frame_ip = frame_csp->start_addr;
        }
        break;

      case WASM_OP_BR_TABLE:
        /* TODO:test */
        read_leb_uint32(frame_ip, frame_ip_end, count);
        if (count <= BR_TABLE_TMP_BUF_LEN)
          depths = depth_buf;
        else {
          if (!(depths = bh_malloc(sizeof(uint32) * count))) {
            printf("wasm interp failed, alloc block memory for br_table failed.\n");
            goto got_exception;
          }
        }
        for (i = 0; i < count; i++) {
          read_leb_uint32(frame_ip, frame_ip_end, depths[i]);
        }
        read_leb_uint32(frame_ip, frame_ip_end, depth);
        didx = POP_I32();
        if (didx >= 0 && (uint32)didx < count) {
          depth = depths[didx];
        }
        if (depths != depth_buf) {
          bh_free(depths);
          depths = NULL;
        }
        POP_CSP_N(depth);
        frame_ip = frame_csp->start_addr;
        break;

      case WASM_OP_RETURN:
        for (i = 0; i < cur_func->ret_cell_num; i++)
          *prev_frame->sp++ = *--frame_sp;
        goto return_func;

      case WASM_OP_CALL:
        read_leb_uint32(frame_ip, frame_ip_end, fidx);
        if (fidx >= module->function_count)
          goto got_exception;
        cur_func = module->functions + fidx;
        goto call_func_from_interp;

      case WASM_OP_CALL_INDIRECT:
        /* TODO: test */
        read_leb_uint32(frame_ip, frame_ip_end, tidx);
        /* to skip 0x00 here */
        frame_ip++;
        val = POP_I32();
        if (val < 0 || val >= (int32)table->cur_size)
          goto got_exception;
        fidx = ((uint32*)table->base_addr)[val];
        if (fidx >= module->function_count)
          goto got_exception;
        cur_func = module->functions + fidx;
        goto call_func_from_interp;

      /* parametric instructions */
      case WASM_OP_DROP:
        frame_sp--;
        break;

      case WASM_OP_SELECT:
        cond = POP_I32();
        frame_sp--;
        if (!cond)
          *frame_sp = *(uint32*)(frame_sp + 1);
        break;

      /* variable instructions */
      case WASM_OP_GET_LOCAL:
        {
          uint32 local_idx;
          uint8 local_type;

          read_leb_uint32(frame_ip, frame_ip_end, local_idx);
          if (local_idx >= cur_func->param_cell_num + cur_func->local_cell_num)
            goto got_exception;

          local_type = cur_func->u.func->local_types[local_idx];
          switch (local_type) {
            case VALUE_TYPE_I32:
              PUSH_I32(LOCAL_I32(local_idx));
              break;
            case VALUE_TYPE_F32:
              PUSH_F32(LOCAL_F32(local_idx));
              break;
            case VALUE_TYPE_I64:
              PUSH_I64(LOCAL_I64(local_idx));
              break;
            case VALUE_TYPE_F64:
              PUSH_F64(LOCAL_F64(local_idx));
              break;
            default:
              goto got_exception;
          }
          break;
        }

      case WASM_OP_SET_LOCAL:
        {
          uint32 local_idx;
          uint8 local_type;

          read_leb_uint32(frame_ip, frame_ip_end, local_idx);
          if (local_idx >= cur_func->param_cell_num + cur_func->local_cell_num)
            goto got_exception;

          local_type = cur_func->u.func->local_types[local_idx];
          switch (local_type) {
            case VALUE_TYPE_I32:
              SET_LOCAL_I32(local_idx, POP_I32());
              break;
            case VALUE_TYPE_F32:
              SET_LOCAL_F32(local_idx, POP_F32());
              break;
            case VALUE_TYPE_I64:
              SET_LOCAL_I64(local_idx, POP_I64());
              break;
            case VALUE_TYPE_F64:
              SET_LOCAL_F64(local_idx, POP_F64());
              break;
            default:
              goto got_exception;
          }
          break;
        }

      case WASM_OP_TEE_LOCAL:
        {
          uint32 local_idx;
          uint8 local_type;

          read_leb_uint32(frame_ip, frame_ip_end, local_idx);
          if (local_idx >= cur_func->param_cell_num + cur_func->local_cell_num)
            goto got_exception;

          local_type = cur_func->u.func->local_types[local_idx];
          switch (local_type) {
            case VALUE_TYPE_I32:
              SET_LOCAL_I32(local_idx, *frame_sp);
              break;
            case VALUE_TYPE_F32:
              SET_LOCAL_F32(local_idx, *(float32*)frame_sp);
              break;
            case VALUE_TYPE_I64:
              SET_LOCAL_I64(local_idx, GET_I64_FROM_ADDR(frame_sp));
              break;
            case VALUE_TYPE_F64:
              SET_LOCAL_F64(local_idx, GET_F64_FROM_ADDR(frame_sp));
              break;
            default:
              goto got_exception;
          }
          break;
        }

      case WASM_OP_GET_GLOBAL:
        {
          WASMGlobalInstance *global;
          uint32 global_idx;

          read_leb_uint32(frame_ip, frame_ip_end, global_idx);
          if (!(global = get_global(module, global_idx))
              || global_idx >= module->global_count)
            goto got_exception;

          switch (global->type) {
            case VALUE_TYPE_I32:
              PUSH_I32(*(uint32*)get_global_addr(memory, global));
              break;
            case VALUE_TYPE_F32:
              PUSH_F32(*(float32*)get_global_addr(memory, global));
              break;
            case VALUE_TYPE_I64:
              PUSH_I64(*(uint64*)get_global_addr(memory, global));
              break;
            case VALUE_TYPE_F64:
              PUSH_F64(*(float64*)get_global_addr(memory, global));
              break;
            default:
              goto got_exception;
          }
          break;
        }

      case WASM_OP_SET_GLOBAL:
        {
          WASMGlobalInstance *global;
          uint32 global_idx;
          uint8 *global_addr;

          read_leb_uint32(frame_ip, frame_ip_end, global_idx);
          if (!(global = get_global(module, global_idx))
              || global_idx >= module->global_count)
            goto got_exception;

          global_addr = get_global_addr(memory, global);
          switch (global->type) {
            case VALUE_TYPE_I32:
              *(uint32*)global_addr = POP_I32();
              break;
            case VALUE_TYPE_F32:
              *(float32*)global_addr = POP_F32();
              break;
            case VALUE_TYPE_I64:
              PUT_I64_TO_ADDR((uint32*)global_addr, POP_I64());
              break;
            case VALUE_TYPE_F64:
              PUT_F64_TO_ADDR((uint32*)global_addr, POP_F64());
              break;
            default:
              goto got_exception;
          }
          break;
        }

      /* memory load instructions */
      case WASM_OP_I32_LOAD:
      case WASM_OP_I64_LOAD:
      case WASM_OP_F32_LOAD:
      case WASM_OP_F64_LOAD:
      case WASM_OP_I32_LOAD8_S:
      case WASM_OP_I32_LOAD8_U:
      case WASM_OP_I32_LOAD16_S:
      case WASM_OP_I32_LOAD16_U:
      case WASM_OP_I64_LOAD8_S:
      case WASM_OP_I64_LOAD8_U:
      case WASM_OP_I64_LOAD16_S:
      case WASM_OP_I64_LOAD16_U:
      case WASM_OP_I64_LOAD32_S:
      case WASM_OP_I64_LOAD32_U:
        {
          uint32 offset, flags, addr;

          read_leb_uint32(frame_ip, frame_ip_end, flags);
          read_leb_uint32(frame_ip, frame_ip_end, offset);
          addr = POP_I32();
          /* TODO: check addr, flags */
          PUSH_I32(*(uint32*)(memory->memory_data + addr + offset));
          (void)flags;
          break;
        }

      /* memory store instructions */
      case WASM_OP_I32_STORE:
      case WASM_OP_I64_STORE:
      case WASM_OP_F32_STORE:
      case WASM_OP_F64_STORE:
      case WASM_OP_I32_STORE8:
      case WASM_OP_I32_STORE16:
      case WASM_OP_I64_STORE8:
      case WASM_OP_I64_STORE16:
      case WASM_OP_I64_STORE32:
        /* TODO */
        break;

      case WASM_OP_MEMORY_SIZE:
      case WASM_OP_MEMORY_GROW:
        /* TODO */
        break;

      /* constant instructions */
      case WASM_OP_I32_CONST:
        {
          uint32 value;
          read_leb_uint32(frame_ip, frame_ip_end, value);
          PUSH_I32(value);
          break;
        }

      case WASM_OP_I64_CONST:
        /* TODO */
        break;

      case WASM_OP_F32_CONST:
        /* TODO */
        break;

      case WASM_OP_F64_CONST:
        /* TODO */
        break;

      /* comparison instructions of i32 */
      case WASM_OP_I32_EQZ:
      case WASM_OP_I32_EQ:
      case WASM_OP_I32_NE:
      case WASM_OP_I32_LT_S:
      case WASM_OP_I32_LT_U:
      case WASM_OP_I32_GT_S:
      case WASM_OP_I32_GT_U:
      case WASM_OP_I32_LE_S:
      case WASM_OP_I32_LE_U:
      case WASM_OP_I32_GE_S:
      case WASM_OP_I32_GE_U:
        /* TODO */
        break;

      /* comparison instructions of i64 */
      case WASM_OP_I64_EQZ:
      case WASM_OP_I64_EQ:
      case WASM_OP_I64_NE:
      case WASM_OP_I64_LT_S:
      case WASM_OP_I64_LT_U:
      case WASM_OP_I64_GT_S:
      case WASM_OP_I64_GT_U:
      case WASM_OP_I64_LE_S:
      case WASM_OP_I64_LE_U:
      case WASM_OP_I64_GE_S:
      case WASM_OP_I64_GE_U:
        /* TODO */
        break;

      /* comparison instructions of f32 */
      case WASM_OP_F32_EQ:
      case WASM_OP_F32_NE:
      case WASM_OP_F32_LT:
      case WASM_OP_F32_GT:
      case WASM_OP_F32_LE:
      case WASM_OP_F32_GE:
        /* TODO */
        break;

      /* comparison instructions of f64 */
      case WASM_OP_F64_EQ:
      case WASM_OP_F64_NE:
      case WASM_OP_F64_LT:
      case WASM_OP_F64_GT:
      case WASM_OP_F64_LE:
      case WASM_OP_F64_GE:
        /* TODO */
        break;

      /* numberic instructions of i32 */
      case WASM_OP_I32_CLZ:
      case WASM_OP_I32_CTZ:
      case WASM_OP_I32_POPCNT:
      case WASM_OP_I32_ADD:
      case WASM_OP_I32_SUB:
      case WASM_OP_I32_MUL:
      case WASM_OP_I32_DIV_S:
      case WASM_OP_I32_DIV_U:
      case WASM_OP_I32_REM_S:
      case WASM_OP_I32_REM_U:
      case WASM_OP_I32_AND:
      case WASM_OP_I32_OR:
      case WASM_OP_I32_XOR:
      case WASM_OP_I32_SHL:
      case WASM_OP_I32_SHR_S:
      case WASM_OP_I32_SHR_U:
      case WASM_OP_I32_ROTL:
      case WASM_OP_I32_ROTR:
        /* TODO */
        break;

      /* numberic instructions of i64 */
      case WASM_OP_I64_CLZ:
      case WASM_OP_I64_CTZ:
      case WASM_OP_I64_POPCNT:
      case WASM_OP_I64_ADD:
      case WASM_OP_I64_SUB:
      case WASM_OP_I64_MUL:
      case WASM_OP_I64_DIV_S:
      case WASM_OP_I64_DIV_U:
      case WASM_OP_I64_REM_S:
      case WASM_OP_I64_REM_U:
      case WASM_OP_I64_AND:
      case WASM_OP_I64_OR:
      case WASM_OP_I64_XOR:
      case WASM_OP_I64_SHL:
      case WASM_OP_I64_SHR_S:
      case WASM_OP_I64_SHR_U:
      case WASM_OP_I64_ROTL:
      case WASM_OP_I64_ROTR:
        /* TODO */
        break;

      /* numberic instructions of f32 */
      case WASM_OP_F32_ABS:
      case WASM_OP_F32_NEG:
      case WASM_OP_F32_CEIL:
      case WASM_OP_F32_FLOOR:
      case WASM_OP_F32_TRUNC:
      case WASM_OP_F32_NEAREST:
      case WASM_OP_F32_SQRT:
      case WASM_OP_F32_ADD:
      case WASM_OP_F32_SUB:
      case WASM_OP_F32_MUL:
      case WASM_OP_F32_DIV:
      case WASM_OP_F32_MIN:
      case WASM_OP_F32_MAX:
      case WASM_OP_F32_COPYSIGN:
        /* TODO */
        break;

      /* numberic instructions of f64 */
      case WASM_OP_F64_ABS:
      case WASM_OP_F64_NEG:
      case WASM_OP_F64_CEIL:
      case WASM_OP_F64_FLOOR:
      case WASM_OP_F64_TRUNC:
      case WASM_OP_F64_NEAREST:
      case WASM_OP_F64_SQRT:
      case WASM_OP_F64_ADD:
      case WASM_OP_F64_SUB:
      case WASM_OP_F64_MUL:
      case WASM_OP_F64_DIV:
      case WASM_OP_F64_MIN:
      case WASM_OP_F64_MAX:
      case WASM_OP_F64_COPYSIGN:
        /* TODO */
        break;

      /* conversions of i32 */
      case WASM_OP_I32_WRAP_I64:
      case WASM_OP_I32_TRUNC_S_F32:
      case WASM_OP_I32_TRUNC_U_F32:
      case WASM_OP_I32_TRUNC_S_F64:
      case WASM_OP_I32_TRUNC_U_F64:
        /* TODO */
        break;

      /* conversions of i64 */
      case WASM_OP_I64_EXTEND_S_I32:
      case WASM_OP_I64_EXTEND_U_I32:
      case WASM_OP_I64_TRUNC_S_F32:
      case WASM_OP_I64_TRUNC_U_F32:
      case WASM_OP_I64_TRUNC_S_F64:
      case WASM_OP_I64_TRUNC_U_F64:
        /* TODO */
        break;

      /* conversions of f32 */
      case WASM_OP_F32_CONVERT_S_I32:
      case WASM_OP_F32_CONVERT_U_I32:
      case WASM_OP_F32_CONVERT_S_I64:
      case WASM_OP_F32_CONVERT_U_I64:
      case WASM_OP_F32_DEMOTE_F64:
        /* TODO */
        break;

      /* conversions of f64 */
      case WASM_OP_F64_CONVERT_S_I32:
      case WASM_OP_F64_CONVERT_U_I32:
      case WASM_OP_F64_CONVERT_S_I64:
      case WASM_OP_F64_CONVERT_U_I64:
      case WASM_OP_F64_PROMOTE_F32:
        /* TODO */
        break;

      /* reinterpretations */
      case WASM_OP_I32_REINTERPRET_F32:
      case WASM_OP_I64_REINTERPRET_F64:
      case WASM_OP_F32_REINTERPRET_I32:
      case WASM_OP_F64_REINTERPRET_I64:
        /* TODO */
        break;

      case WASM_OP_IMPDEP2:
        frame = prev_frame;
        frame_ip = frame->ip;
        frame_sp = frame->sp;
        frame_csp = frame->csp;
        goto call_func_from_entry;

      default:
        printf("wasm interp failed: unsupported opcode 0x%02x.\n",
               opcode);
        goto got_exception;
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
                       + self->stack_cell_num + self->block_cell_num;
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
        frame_sp = frame_lp + cur_func->param_cell_num +
                   cur_func->local_cell_num;
        frame_csp_bottom = (WASMBranchBlock*)(frame_sp + self->stack_cell_num);
        frame_csp = frame_csp_bottom;
        frame_ref = (uint8*)((uint32*)frame_csp + self->block_cell_num);
        frame_csp_boundary = (WASMBranchBlock*)frame_ref;

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
    if (depths && depths != depth_buf) {
      bh_free(depths);
      depths = NULL;
    }
    return;
  }
  (void)tidx;
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
