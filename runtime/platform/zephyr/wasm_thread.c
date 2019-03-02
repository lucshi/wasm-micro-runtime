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


typedef struct wasm_thread_wait_node {
  struct k_sem sem;
  wasm_thread_wait_list next;
} wasm_thread_wait_node;

typedef struct wasm_thread_data {
  /* Next thread data */
  struct wasm_thread_data *next;
  /* Zephyr thread handle */
  korp_tid tid;
  /* Jeff thread local root */
  void *tlr;
  /* Lock for waiting list */
  struct k_mutex wait_list_lock;
  /* Waiting list of other threads who are joining this thread */
  wasm_thread_wait_list thread_wait_list;
  /* Thread stack size */
  unsigned stack_size;
  /* Thread stack */
  char stack[1];
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

static void
thread_data_list_remove(wasm_thread_data *thread_data)
{
  k_mutex_lock(&thread_data_lock, K_FOREVER);
  if (thread_data_list) {
    if (thread_data_list == thread_data)
      thread_data_list = thread_data_list->next;
    else {
      /* Search and remove it from list */
      wasm_thread_data *p = thread_data_list;
      while (p && p->next != thread_data)
        p = p->next;
      if (p && p->next == thread_data)
        p->next = p->next->next;
    }
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

static void
ws_thread_cleanup(void)
{
  wasm_thread_data *thread_data = thread_data_current();

  wasm_assert(thread_data != NULL);
  k_mutex_lock(&thread_data->wait_list_lock, K_FOREVER);
  if (thread_data->thread_wait_list) {
    /* Signal each joining thread */
    wasm_thread_wait_list head = thread_data->thread_wait_list;
    while (head) {
      wasm_thread_wait_list next = head->next;
      k_sem_give(&head->sem);
      wasm_free(head);
      head = next;
    }
    thread_data->thread_wait_list = NULL;
  }
  k_mutex_unlock(&thread_data->wait_list_lock);

  thread_data_list_remove(thread_data);
  wasm_free(thread_data);
}

static void
ws_thread_wrapper(void *start, void *arg, void *thread_data)
{
  /* Set thread custom data */
  ((wasm_thread_data*)thread_data)->tid = k_current_get();
  thread_data_list_add(thread_data);

  ((thread_start_routine_t)start)(arg);
  ws_thread_cleanup();
}

int
ws_thread_create(korp_tid *p_tid, thread_start_routine_t start,
                 void *arg, unsigned int stack_size)
{
  return ws_thread_create_with_prio(p_tid, start, arg, stack_size,
                                    WASM_THREAD_DEFAULT_PRIORITY);
}

int
ws_thread_create_with_prio(korp_tid *p_tid, thread_start_routine_t start,
                           void *arg, unsigned int stack_size, int prio)
{
  korp_tid tid;
  wasm_thread_data *thread_data;
  unsigned thread_data_size;

  if (!p_tid || !stack_size)
    return BHT_ERROR;

  /* Create and initialize thread data */
  thread_data_size = offsetof(wasm_thread_data, stack) + stack_size;
  if (!(thread_data = wasm_malloc(thread_data_size)))
    return BHT_ERROR;

  memset(thread_data, 0, thread_data_size);
  k_mutex_init(&thread_data->wait_list_lock);
  thread_data->stack_size = stack_size;

  /* Create the thread */
  if (!(tid = k_thread_create(*p_tid,(k_thread_stack_t *)thread_data->stack, stack_size,
                             ws_thread_wrapper, start, arg, thread_data,
                             prio, 0, K_NO_WAIT))) {
    wasm_free(thread_data);
    return BHT_ERROR;
  }

  /* Set thread custom data */
  thread_data->tid = tid;
  thread_data_list_add(thread_data);
  *p_tid = tid;
  return BHT_OK;
}

korp_tid
ws_self_thread()
{
  return (korp_tid)k_current_get();
}

void
ws_thread_exit(void * code)
{
  (void)code;
  korp_tid self = ws_self_thread();
  ws_thread_cleanup();
  k_thread_abort((k_tid_t)self);
}

int
ws_thread_cancel (korp_tid thread)
{
  k_thread_abort((k_tid_t)thread);
  return 0;
}

int
ws_thread_join (korp_tid thread, void **value_ptr, int mills)
{
  (void)value_ptr;
  wasm_thread_data *thread_data;
  wasm_thread_wait_node *node;

  /* Create wait node and append it to wait list */
  if (!(node = wasm_malloc(sizeof(wasm_thread_wait_node))))
    return BHT_ERROR;

  k_sem_init(&node->sem, 0, 1);
  node->next = NULL;

  /* Get thread data */
  thread_data = thread_data_list_lookup(thread);
  wasm_assert(thread_data != NULL);

  k_mutex_lock(&thread_data->wait_list_lock, K_FOREVER);
  if (!thread_data->thread_wait_list)
    thread_data->thread_wait_list = node;
  else {
    /* Add to end of waiting list */
    wasm_thread_wait_node *p = thread_data->thread_wait_list;
    while (p->next)
      p = p->next;
    p->next = node;
  }
  k_mutex_unlock(&thread_data->wait_list_lock);

  /* Wait the sem */
  k_sem_take(&node->sem, mills);

  /* Wait some time for the thread to be actually terminated */
  k_sleep(100);

  return BHT_OK;
}

int
ws_thread_detach (korp_tid thread)
{
  (void)thread;
  return BHT_OK;
}

void *ws_tls_get(unsigned idx)
{
  (void)idx;
  wasm_thread_data *thread_data;

  wasm_assert (idx == 0);
  thread_data = thread_data_current();

  return thread_data ? thread_data->tlr : NULL;
}

int ws_tls_put(unsigned idx, void * tls)
{
  wasm_thread_data *thread_data;

  (void)idx;
  wasm_assert (idx == 0);
  thread_data = thread_data_current();
  wasm_assert (thread_data != NULL);

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

int
ws_mutex_trylock(korp_mutex *mutex)
{
  return k_mutex_lock(mutex, K_NO_WAIT);
}

void ws_mutex_unlock(korp_mutex *mutex)
{
  k_mutex_unlock(mutex);
}

int ws_sem_init(korp_sem* sem, unsigned int c)
{
  k_sem_init(sem, 0, c);
  return BHT_OK;
}

int ws_sem_destroy(korp_sem *sem)
{
  (void)sem;
  return BHT_OK;
}

int ws_sem_wait(korp_sem *sem)
{
  return k_sem_take(sem, K_FOREVER);
}

int ws_sem_reltimedwait(korp_sem *sem, int mills)
{
  return k_sem_take(sem, mills);
}

int ws_sem_post(korp_sem *sem)
{
  k_sem_give(sem);
  return BHT_OK;
}

int
ws_cond_init(korp_cond *cond)
{
  k_mutex_init(&cond->wait_list_lock);
  cond->thread_wait_list = NULL;
  return BHT_OK;
}

int
ws_cond_destroy(korp_cond *cond)
{
  (void)cond;
  return BHT_OK;
}

static int
ws_cond_wait_internal(korp_cond *cond, korp_mutex *mutex,
                      bool timed, int mills)
{
  wasm_thread_wait_node *node;

  /* Create wait node and append it to wait list */
  if (!(node = wasm_malloc(sizeof(wasm_thread_wait_node))))
    return BHT_ERROR;

  k_sem_init(&node->sem, 0, 1);
  node->next = NULL;

  k_mutex_lock(&cond->wait_list_lock, timed ? mills : K_FOREVER);
  if (!cond->thread_wait_list)
    cond->thread_wait_list = node;
  else {
    /* Add to end of wait list */
    wasm_thread_wait_node *p = cond->thread_wait_list;
    while (p->next)
      p = p->next;
    p->next = node;
  }
  k_mutex_unlock(&cond->wait_list_lock);

  /* Unlock mutex, wait sem and lock mutex again */
  k_mutex_unlock(mutex);
  k_sem_take(&node->sem, K_FOREVER);
  k_mutex_lock(mutex, K_FOREVER);

  /* Remove wait node from wait list */
  k_mutex_lock(&cond->wait_list_lock, K_FOREVER);
  if (cond->thread_wait_list == node)
    cond->thread_wait_list = node->next;
  else {
    /* Remove from the wait list */
    wasm_thread_wait_node *p = cond->thread_wait_list;
    while (p->next != node)
      p = p->next;
    p->next = node->next;
  }
  wasm_free(node);
  k_mutex_unlock(&cond->wait_list_lock);

  return BHT_OK;
}

int
ws_cond_wait(korp_cond *cond, korp_mutex *mutex)
{
  return ws_cond_wait_internal(cond, mutex, false, 0);
}

int
ws_cond_reltimedwait(korp_cond *cond, korp_mutex *mutex, int mills)
{
  return ws_cond_wait_internal(cond, mutex, true, mills);
}

int
ws_cond_signal(korp_cond *cond)
{
  /* Signal the head wait node of wait list */
  k_mutex_lock(&cond->wait_list_lock, K_FOREVER);
  if (cond->thread_wait_list)
    k_sem_give(&cond->thread_wait_list->sem);
  k_mutex_unlock(&cond->wait_list_lock);

  return BHT_OK;
}

int
ws_cond_broadcast (korp_cond *cond)
{
  /* Signal each wait node of wait list */
  k_mutex_lock(&cond->wait_list_lock, K_FOREVER);
  if (cond->thread_wait_list) {
    wasm_thread_wait_node *p = cond->thread_wait_list;
    while (p) {
      k_sem_give(&p->sem);
      p = p->next;
    }
  }
  k_mutex_unlock(&cond->wait_list_lock);

  return BHT_OK;
}

