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

#ifndef _WASM_THREAD_H
#define _WASM_THREAD_H

#include "wasm-import.h"
#include "bh_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMVmInstance;
struct WASMInterpFrame;

typedef enum WASMThreadState {
  WASM_THREAD_ZOMBIE    = 0, /* terminated */
  WASM_THREAD_RUNNING   = 1, /* ready or running */
  WASM_THREAD_SLEEPING  = 2, /* timed waiting */
  WASM_THREAD_MONITOR   = 3, /* blocked on a monitor */
  WASM_THREAD_WAIT      = 4, /* waiting */
  /* Other thread states */
  WASM_THREAD_VMWAIT    = 5
} WASMThreadState;

typedef struct WASMThread {
  /* Previous thread's tlr of an instance. */
  struct WASMThread *prev;

  /* Next thread's tlr of an instance. */
  struct WASMThread *next;

  /* The VM instance of the thread */
  struct WASMVmInstance *vm_instance;

  /* The native thread handle of this VM thread. */
  vmci_thread_t handle;

  /* The start routine of this thread.  The main thread of instance is
     set to the WASM thread's start routine to denote that it's a WASM
     thread. */
  void* (*start_routine)(void *);

  /* The argument to start routine. */
  void *start_routine_arg;

  /* Current state of this thread. */
  WASMThreadState state;

  /* The cell num of the stack */
  uint16 stack_cell_num;

  /* Current frame of a WASM thread. */
  struct WASMInterpFrame *cur_frame;

  /* The boundary of native stack. When interpreter detects that native
     frame may overrun this boundary, it will throw a stack overflow
     exception. */
  void *native_stack_boundary;

  /* The WASM stack of a WASM thread. */
  union {
    uint64 _make_it_8_byte_aligned_;

    struct {
      /* The top boundary of the stack. */
      uint8 *top_boundary;

      /* Top cell index which is free. */
      uint8 *top;

      /* The WASM stack. */
      uint8 bottom[1];
    } s;
  } wasm_stack;
} WASMThread;

typedef struct WASMVmInstance {
  /* Next VM instance in the global VM instance list. */
  struct WASMVmInstance *prev;

  /* Previous VM instance in the global VM instance list. */
  struct WASMVmInstance *next;

  /* The WASM module instance. */
  struct WASMModuleInstance *module;

  /* Default stack size of threads of this VM instance. */
  uint32 native_stack_size;

  /* WASM stacksize of threads of this VM instance. */
  uint32 wasm_stack_size;

  /* The start routine of the main thread of this VM instance. */
  void* (*start_routine)(void *);

  /* The argument to start routine. */
  void *start_routine_arg;

  /* Cleanup routine that is called after the instance terminates. */
  void (*cleanup_routine)();

  /* The main thread of this VM instance. */
  WASMThread main_tlr;
} WASMVmInstance;

/**
 * Create an instance control block and initialize its fields.
 *
 * @param module_inst the WASM module instance
 * @param native_stack_size the stack size of threads of the new instance
 * @param wasm_stack_size WASM stack size of threads of new instance
 * @param start_routine start routine of the main thread
 * @param arg the argument to the start routine
 * @param cleanup_routine the cleanup routine for the instance
 *
 * @return the VM instance handle if succeeds, NULL otherwise
 */
WASMVmInstance*
wasm_thread_create_ilr(struct WASMModuleInstance *module_inst,
                       uint32 native_stack_size,
                       uint32 wasm_stack_size,
                       void* (*start_routine)(void *), void *arg,
                       void (*cleanup_routine)());

/**
 * Release resources contained in the instance control block,
 * e.g. locks and condition variables.
 *
 * @param ilr the instance control block to be destroyed
 */
void
wasm_thread_destroy_ilr(WASMVmInstance *ilr);

/**
 * Create a native thread for a WASM thread and attach it
 * to the given VM instance.  After the new thread is successfully
 * created, its initial state is WASM_THREAD_RUNNING.  This function
 * can be called from threads that don't belong to the given VM
 * instance.
 *
 * @param ilr the instance for which the new thread is created
 * @param start the start routine of the new thread
 * @param arg the argument to the start routine
 *
 * @return the new thread if succeeds, NULL otherwise
 */
WASMThread*
wasm_thread_create_thread(WASMVmInstance *ilr,
                          void* (*start_routine)(void *), void *arg,
                          int priority);

/**
 * Detach the current thread from the current VM instance and make it
 * able to release its resources after terminated.  After calling this
 * function, don't access any instance local and thread local data
 * structures because those resources may be freed at any time.
 */
void
wasm_thread_detach();

/**
 * Set native stack boundary for the current thread. This must be
 * called at the beginning of a new VM thread.
 *
 * @param self the current thread
 */
static inline void
wasm_thread_set_native_stack_boundary(WASMThread *self)
{
  bh_assert(!self->native_stack_boundary);

  /* TODO: this depends on stack growing direction. */
  self->native_stack_boundary =
    (uint8 *)&self - (self->vm_instance->native_stack_size -
                      vmci_reserved_native_stack_size);
}

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
  uint8 *addr = tlr->wasm_stack.s.top;

  bh_assert(!(size & 3));

  /* The outs area size cannot be larger than the frame size, so
     multiplying by 2 is enough. */
  if (addr + size * 2 > tlr->wasm_stack.s.top_boundary) {
    /* WASM stack overflow. */
    /* When throwing SOE, the preserved space must be enough. */
    /*bh_assert(!tlr->throwing_soe);*/
    return NULL;
  }

  tlr->wasm_stack.s.top += size;

  return addr;
}

static inline void
wasm_thread_free_wasm_frame(WASMThread *tlr, void *prev_top)
{
  bh_assert((uint8 *)prev_top >= tlr->wasm_stack.s.bottom);
  tlr->wasm_stack.s.top = (uint8 *)prev_top;
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
  return tlr->wasm_stack.s.top;
}

/**
 * Wait for the given VM instance to terminate.
 *
 * @param ilr the VM instance to be waited for
 * @param mills wait millseconds to return
 */
void
wasm_thread_wait_for_instance(WASMVmInstance *ilr, int mills);

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

#endif /* end of _WASM_THREAD_H */
