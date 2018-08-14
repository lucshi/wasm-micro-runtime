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


bool
vmci_thread_sys_init()
{
  return vm_thread_sys_init();
}

void
vmci_thread_sys_destroy()
{
  vm_thread_sys_destroy();
}

int
vmci_thread_create_with_prio(vmci_thread_t *thread,
                             vmci_thread_start_routine_t start_routine, void *arg,
                             unsigned stack_size, int prio)
{
  return vm_thread_create_with_prio(thread, start_routine, arg, stack_size, prio);
}

int
vmci_thread_create(vmci_thread_t *thread,
                   vmci_thread_start_routine_t start_routine, void *arg,
                   unsigned stack_size)
{
  return vm_thread_create(thread, start_routine, arg, stack_size);
}

int
vmci_thread_cancel(vmci_thread_t thread)
{
  return vm_thread_cancel(thread);
}

int
vmci_thread_join(vmci_thread_t thread, void **value_ptr, int mills)
{
  return vm_thread_join(thread, value_ptr, mills);
}

int
vmci_thread_detach(vmci_thread_t thread)
{
  return vm_thread_detach(thread);
}

void
vmci_thread_exit(void *value_ptr)
{
  vm_thread_exit(value_ptr);
}

vmci_thread_t
vmci_thread_self()
{
  return vm_self_thread();
}

int
vmci_thread_mutex_init(vmci_thread_mutex_t *mutex, bool is_recursive)
{
  return vm_mutex_init(mutex, is_recursive);
}

void
vmci_thread_mutex_lock(vmci_thread_mutex_t *mutex)
{
  vm_mutex_lock(mutex);
}

int
vmci_thread_mutex_trylock(vmci_thread_mutex_t *mutex)
{
  return vm_mutex_trylock(mutex);
}

void
vmci_thread_mutex_unlock(vmci_thread_mutex_t *mutex)
{
  vm_mutex_unlock(mutex);
}

int
vmci_thread_mutex_destroy(vmci_thread_mutex_t *mutex)
{
  return vm_mutex_destroy(mutex);
}

