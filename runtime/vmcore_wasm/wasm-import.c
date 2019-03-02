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

#include "wasm-import.h"
#include "wasm_memory.h"


bool
wsci_thread_sys_init()
{
  return ws_thread_sys_init();
}

void
wsci_thread_sys_destroy()
{
  ws_thread_sys_destroy();
}

int
wsci_thread_create_with_prio(wsci_thread_t *thread,
                             wsci_thread_start_routine_t start_routine, void *arg,
                             unsigned stack_size, int prio)
{
#ifndef __ZEPHYR__
  return ws_thread_create_with_prio(thread, start_routine, arg, stack_size, prio);
#else
  int ret;
  struct k_thread *kthread = wasm_malloc(sizeof(struct k_thread));
  if (!kthread)
    return BHT_ERROR;

  memset(kthread, 0, sizeof(struct k_thread));
  *thread = (wsci_thread_t)kthread;
  ret = ws_thread_create_with_prio(thread, start_routine, arg, stack_size, prio);
  if (ret != BHT_OK)
    wasm_free(kthread);
  return ret;
#endif
}

int
wsci_thread_create(wsci_thread_t *thread,
                   wsci_thread_start_routine_t start_routine, void *arg,
                   unsigned stack_size)
{
#ifndef __ZEPHYR__
  return ws_thread_create(thread, start_routine, arg, stack_size);
#else
  int ret;
  struct k_thread *kthread = wasm_malloc(sizeof(struct k_thread));
  if (!kthread)
    return BHT_ERROR;

  memset(kthread, 0, sizeof(struct k_thread));
  *thread = (wsci_thread_t)kthread;
  ret = ws_thread_create(thread, start_routine, arg, stack_size);
  if (ret != BHT_OK)
    wasm_free(kthread);
  return ret;
#endif
}

int
wsci_thread_cancel(wsci_thread_t thread)
{
  return ws_thread_cancel(thread);
}

int
wsci_thread_join(wsci_thread_t thread, void **value_ptr, int mills)
{
  return ws_thread_join(thread, value_ptr, mills);
}

int
wsci_thread_detach(wsci_thread_t thread)
{
  return ws_thread_detach(thread);
}

void
wsci_thread_exit(void *value_ptr)
{
  ws_thread_exit(value_ptr);
}

wsci_thread_t
wsci_thread_self()
{
  return ws_self_thread();
}

int
wsci_thread_mutex_init(wsci_thread_mutex_t *mutex)
{
  return ws_mutex_init(mutex, false);
}

int wsci_thread_mutex_init_recursive(wsci_thread_mutex_t *mutex)
{
  return ws_mutex_init(mutex, true);
}

void
wsci_thread_mutex_lock(wsci_thread_mutex_t *mutex)
{
  ws_mutex_lock(mutex);
}

int
wsci_thread_mutex_trylock(wsci_thread_mutex_t *mutex)
{
  return ws_mutex_trylock(mutex);
}

void
wsci_thread_mutex_unlock(wsci_thread_mutex_t *mutex)
{
  ws_mutex_unlock(mutex);
}

int
wsci_thread_mutex_destroy(wsci_thread_mutex_t *mutex)
{
  return ws_mutex_destroy(mutex);
}

