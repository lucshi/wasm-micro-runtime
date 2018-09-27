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

#include "wasm-thread.h"
#include "wasm-runtime.h"
#include "bh_log.h"
#include "bh_memory.h"


/**
 * Initialize a thread control block that belongs to the given app
 * instance.  The memory pointed to by tlr must have been set to zero
 * (e.g. allocated by calloc-like functions).
 */
static bool
wasm_thread_tlr_init(WASMThread *tlr, WASMVmInstance *ilr)
{
  tlr->vm_instance = ilr;
  tlr->state = WASM_THREAD_RUNNING;
  tlr->stack_cell_num = vmci_default_stack_cell_num;
  tlr->block_cell_num = vmci_default_block_cell_num;
  tlr->wasm_stack.s.top_boundary =
    tlr->wasm_stack.s.bottom + ilr->wasm_stack_size;
  tlr->wasm_stack.s.top = tlr->wasm_stack.s.bottom;

  return true;
}

static void
wasm_thread_tlr_destroy(WASMThread *tlr)
{
}

WASMVmInstance*
wasm_thread_create_ilr(struct WASMModuleInstance *module_inst,
                       uint32 native_stack_size, uint32 wasm_stack_size,
                       void* (*start_routine)(void *), void *arg,
                       void (*cleanup_routine)())
{
  uint32 total_size;
  WASMVmInstance *ilr;

  /* Make the stack size 8-aligned. */
  native_stack_size = align_uint(native_stack_size, 8);

  total_size = offsetof(WASMVmInstance, main_tlr.wasm_stack.s.bottom)
               + wasm_stack_size;
  if (!(ilr = bh_malloc(total_size))) {
    LOG_ERROR("Initialize VM instance failed: allocate memory failed.\n");
    return NULL;
  }
  memset(ilr, 0, total_size);

  ilr->module = module_inst;
  ilr->native_stack_size = native_stack_size;
  ilr->wasm_stack_size = wasm_stack_size;
  ilr->start_routine = start_routine;
  ilr->start_routine_arg = arg;
  ilr->cleanup_routine = cleanup_routine;

  if (!wasm_thread_tlr_init(&ilr->main_tlr, ilr)) {
    bh_free(ilr);
    return NULL;
  }

  return ilr;
}

void
wasm_thread_destroy_ilr(WASMVmInstance *ilr)
{
  wasm_thread_tlr_destroy(&ilr->main_tlr);
  bh_free(ilr);
}

WASMThread*
wasm_thread_create_thread(WASMVmInstance *ilr,
                          void* (*start_routine)(void *), void *arg,
                          int priority)
{
  /* TODO */
  return NULL;
}

/**
 * Unlink the thread from the thread list of its VM instance. 
 */
static void
unlink_from_thread_list(WASMThread *tlr)
{
  /* The main thread cannot be unlinked. */
  bh_assert (tlr->prev);

  tlr->prev->next = tlr->next;
  if (tlr->next)
    tlr->next->prev = tlr->prev;
}

void
wasm_thread_detach()
{
  WASMThread *self = wasm_runtime_get_self();
  vmci_thread_t handle;

  /* TODO: lock thread list */

  self->state = WASM_THREAD_ZOMBIE;
  /* After unlinked from thread list, we cannot be joined when
     destroing the VM instance, so we must detach ourselves.  */
  unlink_from_thread_list(self);

  /* Store the thread handle to a local variable because after
     unlocking, the self struct may be freed at any time.  */
  handle = self->handle;

  wasm_thread_tlr_destroy (self);

  bh_free(self);

  wasm_runtime_set_tlr(NULL);

  /* TODO: unlock thread list */

  vmci_thread_detach(handle);
}

void
wasm_thread_wait_for_instance(WASMVmInstance *ilr, int mills)
{
  vmci_thread_join(ilr->main_tlr.handle, NULL, mills);
}
