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

#ifdef __cplusplus
extern "C" {
#endif

struct WASMVmInstance;
struct WASMRuntimeFrame;

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
     set to the java thread's start routine to denote that it's a Java
     thread.  */
  void* (*start_routine)(void *);

  /* The argument to start routine. */
  void *start_routine_arg;

  /* Current frame of a WASM thread.  */
  struct WASMRuntimeFrame *cur_frame;

  /* The WASM stack of a WASM thread.  */
  union {
    uint64 _make_it_8_byte_aligned_;

    struct {
      /* The top boundary of the stack.  */
      uint8 *top_boundary;

      /* Top cell index which is free.  */
      uint8 *top;

      /* The WASM stack.  */
      uint8 bottom[1];
    } s;
  } wasm_stack;
} WASMThread;

typedef struct WASMVmInstance {
  /* Next VM instance in the global VM instance list.  */
  struct WASMVmInstance *prev;

  /* Previous VM instance in the global VM instance list.  */
  struct WASMVmInstance *next;

  struct WASMModuleInstance *module;

  /* Default stack size of threads of this VM instance.  */
  uint32 stack_size;

  /* The start routine of the main thread of this VM instance.  */
  void* (*start_routine)(void *);

  /* The argument to start routine.  */
  void *start_routine_arg;

  /* Cleanup routine that is called after the instance terminates.  */
  void (*cleanup_routine)();

  WASMThread main_tlr;
} WASMVmInstance;

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_THREAD_H */
