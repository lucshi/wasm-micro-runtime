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

#ifndef _WASM_RUNTIME_THREAD_H
#define _WASM_RUNTIME_THREAD_H

#include "wasm_assert.h"
#include "wasm_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMModuleInstance;
struct WASMInterpFrame;

typedef struct WASMStack {
  /* The bottom of the stack, must be 8-bytes align. */
  uint8 *bottom;
  /* Top cell index which is free. */
  uint8 *top;
  /* The top boundary of the stack. */
  uint8 *top_boundary;
} WASMStack;

typedef struct WASMThread {
  /* Previous thread's tlr of an instance. */
  struct WASMThread *prev;

  /* Next thread's tlr of an instance. */
  struct WASMThread *next;

  /* The WASM module instance of current thread */
  struct WASMModuleInstance *module_inst;

  /* Current frame of current thread. */
  struct WASMInterpFrame *cur_frame;

  /* The boundary of native stack. When interpreter detects that native
     frame may overrun this boundary, it will throw a stack overflow
     exception. */
  void *native_stack_boundary;

  /* The WASM stack of current thread. */
  WASMStack wasm_stack;

  /* The native thread handle of current thread. */
  korp_tid handle;

  /* Current suspend count of this thread.  */
  uint32 suspend_count;
} WASMThread;

/**
 * Allocate a WASM frame from the WASM stack.
 *
 * @param tlr the current thread
 * @param size size of the WASM frame, it must be a multiple of 4
 *
 * @return the WASM frame if there is enough space in the stack area
 * with a protection area, NULL otherwise
 */
static inline void*
wasm_thread_alloc_wasm_frame(WASMThread *tlr, unsigned size)
{
  uint8 *addr = tlr->wasm_stack.top;

  wasm_assert(!(size & 3));

  /* The outs area size cannot be larger than the frame size, so
     multiplying by 2 is enough. */
  if (addr + size * 2 > tlr->wasm_stack.top_boundary) {
    /* WASM stack overflow. */
    /* When throwing SOE, the preserved space must be enough. */
    /*wasm_assert(!tlr->throwing_soe);*/
    return NULL;
  }

  tlr->wasm_stack.top += size;

  return addr;
}

static inline void
wasm_thread_free_wasm_frame(WASMThread *tlr, void *prev_top)
{
  wasm_assert((uint8 *)prev_top >= tlr->wasm_stack.bottom);
  tlr->wasm_stack.top = (uint8 *)prev_top;
}

/**
 * Get the current WASM stack top pointer.
 *
 * @param tlr the current thread
 *
 * @return the current WASM stack top pointer
 */
static inline void*
wasm_thread_wasm_stack_top(WASMThread *tlr)
{
  return tlr->wasm_stack.top;
}

/**
 * Set the current frame pointer.
 *
 * @param tlr the current thread
 * @param frame the WASM frame to be set for the current thread
 */
static inline void
wasm_thread_set_cur_frame(WASMThread *tlr, struct WASMInterpFrame *frame)
{
  tlr->cur_frame = frame;
}

/**
 * Get the current frame pointer.
 *
 * @param tlr the current thread
 *
 * @return the current frame pointer
 */
static inline struct WASMInterpFrame*
wasm_thread_get_cur_frame(WASMThread *tlr)
{
  return tlr->cur_frame;
}

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_RUNTIME_THREAD_H */
