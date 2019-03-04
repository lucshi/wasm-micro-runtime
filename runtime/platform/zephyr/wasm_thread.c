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


typedef struct wasm_thread_data {
  /* Next thread data */
  struct wasm_thread_data *next;
  /* Zephyr thread handle */
  korp_tid tid;
  /* Jeff thread local root */
  void *tlr;
} wasm_thread_data;

/* Thread data of supervisor thread */
static wasm_thread_data supervisor_thread_data;

/* Lock for thread data list */
static struct k_mutex thread_data_lock;

/* Thread data list */
static wasm_thread_data *thread_data_list = NULL;

static void
thread_data_list_add(wasm_thread_data *thread_data)
{
  k_mutex_lock(&thread_data_lock, K_FOREVER);
  if (!thread_data_list)
    thread_data_list = thread_data;
  else {
    /* If already in list, just return */
    wasm_thread_data *p = thread_data_list;
    while (p) {
      if (p == thread_data) {
        k_mutex_unlock(&thread_data_lock);
        return;
      }
      p = p->next;
    }

    /* Set as head of list */
    thread_data->next = thread_data_list;
    thread_data_list = thread_data;
  }
  k_mutex_unlock(&thread_data_lock);
}

static wasm_thread_data *
thread_data_list_lookup(k_tid_t tid)
{
  k_mutex_lock(&thread_data_lock, K_FOREVER);
  if (thread_data_list) {
    wasm_thread_data *p = thread_data_list;
    while (p) {
      if (p->tid == tid) {
        /* Found */
        k_mutex_unlock(&thread_data_lock);
        return p;
      }
      p = p->next;
    }
  }
  k_mutex_unlock(&thread_data_lock);
  return NULL;
}

int
ws_thread_sys_init()
{
  k_mutex_init(&thread_data_lock);

  /* Initialize supervisor thread data */
  memset(&supervisor_thread_data, 0, sizeof(supervisor_thread_data));
  supervisor_thread_data.tid = k_current_get();
  /* Set as head of thread data list */
  thread_data_list = &supervisor_thread_data;
  return BHT_OK;
}

void
ws_thread_sys_destroy()
{
}

static wasm_thread_data *
thread_data_current()
{
  k_tid_t tid = k_current_get();
  return thread_data_list_lookup(tid);
}

korp_tid
ws_self_thread()
{
  return (korp_tid)k_current_get();
}

void *ws_tls_get()
{
  wasm_thread_data *thread_data = thread_data_current();
  return thread_data ? thread_data->tlr : NULL;
}

int ws_tls_put(void * tls)
{
  wasm_thread_data *thread_data = thread_data_current();

  if (!thread_data) {
    if (!(thread_data = wasm_malloc(sizeof(wasm_thread_data))))
      return BHT_ERROR;

    thread_data->next = NULL;
    thread_data->tid = ws_self_thread();
    thread_data_list_add(thread_data);
  }

  thread_data->tlr = tls;
  return BHT_OK;
}

int
ws_mutex_init(korp_mutex *mutex, bool is_recursive)
{
  k_mutex_init(mutex);
  return BHT_OK;
}

int
ws_mutex_destroy(korp_mutex *mutex)
{
  return BHT_OK;
}

void
ws_mutex_lock(korp_mutex *mutex)
{
  k_mutex_lock(mutex, K_FOREVER);
}

void ws_mutex_unlock(korp_mutex *mutex)
{
  k_mutex_unlock(mutex);
}

