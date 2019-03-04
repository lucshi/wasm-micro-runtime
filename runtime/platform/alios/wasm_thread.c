/* 
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
*/

#include "wasm_thread.h"
#include "wasm_assert.h"
#include "wasm_log.h"
#include "wasm_memory.h"
#include <stdio.h>
#include <stdlib.h>


/* Thread data key */
static aos_task_key_t thread_local_storage_key;

int
ws_thread_sys_init()
{
  if (aos_task_key_create(&thread_local_storage_key) != 0)
    return BHT_ERROR;

  return BHT_OK;
}

void
ws_thread_sys_destroy()
{
  aos_task_key_delete(thread_local_storage_key);
}

korp_tid
ws_self_thread()
{
  /* TODO: no related API in AliOS system */
  return NULL;
}

void *
ws_tls_get()
{
  return aos_task_getspecific(thread_local_storage_key);
}

int
ws_tls_put(void *tls)
{
  return aos_task_setspecific(thread_local_storage_key, tls);
}

int
ws_mutex_init(korp_mutex *mutex, bool is_recursive)
{
  return aos_mutex_new(mutex) == 0 ? BHT_OK : BHT_ERROR;
}

int
ws_mutex_destroy(korp_mutex *mutex)
{
  aos_mutex_free(mutex);
  return BHT_OK;
}

void
ws_mutex_lock(korp_mutex *mutex)
{
  aos_mutex_lock(mutex, AOS_WAIT_FOREVER);
}

int
ws_mutex_trylock(korp_mutex *mutex)
{
  return aos_mutex_lock(mutex, AOS_NO_WAIT);
}

void ws_mutex_unlock(korp_mutex *mutex)
{
  aos_mutex_unlock(mutex);
}

