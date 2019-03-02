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

#include "wasm_log.h"
#include "wasm_thread.h"
#include "wasm_assert.h"
#include "wasm_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


static korp_mutex thread_list_lock;
static pthread_key_t thread_local_storage_key[WASM_MAX_TLS_NUM];

int ws_thread_sys_init()
{
  unsigned i;

  for (i = 0; i < WASM_MAX_TLS_NUM; i++)
    pthread_key_create(&thread_local_storage_key[i], NULL);

  return ws_mutex_init(&thread_list_lock, false);
}

void ws_thread_sys_destroy()
{
  unsigned i;

  for (i = 0; i < WASM_MAX_TLS_NUM; i++)
    pthread_key_delete(thread_local_storage_key[i]);

  ws_mutex_destroy(&thread_list_lock);
}

typedef struct {
  thread_start_routine_t start;
  void* stack;
  int stack_size;
  void* arg;
} thread_wrapper_arg;

static void *ws_thread_wrapper(void *arg)
{
  thread_wrapper_arg * targ = arg;
  targ->stack = (void *)((uintptr_t)(&arg) & ~0xfff);
  ws_tls_put(1, targ);
  targ->start(targ->arg);
  wasm_free(targ);
  ws_tls_put(1, NULL);
  return NULL;
}

int ws_thread_create_with_prio(korp_tid *tid,
                               thread_start_routine_t start, void *arg,
                               unsigned int stack_size, int prio)
{
  pthread_attr_t tattr;
  thread_wrapper_arg *targ;

  wasm_assert(stack_size > 0);
  wasm_assert(tid);
  wasm_assert(start);

  *tid = INVALID_THREAD_ID;

  pthread_attr_init(&tattr);
  pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
  if(pthread_attr_setstacksize(&tattr, stack_size) != 0) {
    LOG_ERROR("Invalid thread stack size %u. Min stack size on Linux = %u\n",
              stack_size, PTHREAD_STACK_MIN);
    pthread_attr_destroy(&tattr);
    return BHT_ERROR;
  }

  targ = (thread_wrapper_arg*) wasm_malloc(sizeof(*targ));
  if(!targ) {
    pthread_attr_destroy(&tattr);
    return BHT_ERROR;
  }

  targ->start = start;
  targ->arg = arg;
  targ->stack_size = stack_size;

  if(pthread_create(tid, &tattr, ws_thread_wrapper, targ) != 0) {
    pthread_attr_destroy(&tattr);
    wasm_free(targ);
    return BHT_ERROR;
  }

  pthread_attr_destroy(&tattr);
  return BHT_OK;
}

int ws_thread_create(korp_tid *tid, thread_start_routine_t start, void *arg,
                      unsigned int stack_size)
{
  return ws_thread_create_with_prio(tid, start, arg, stack_size,
                                    WASM_THREAD_DEFAULT_PRIORITY);
}

korp_tid ws_self_thread()
{
  return (korp_tid) pthread_self();
}

void ws_thread_exit(void * code)
{
  wasm_free(ws_tls_get(1));
  ws_tls_put(1, NULL);
  pthread_exit(code);
}

void *ws_tls_get(unsigned idx)
{
  wasm_assert(idx < WASM_MAX_TLS_NUM);
  return pthread_getspecific(thread_local_storage_key[idx]);
}

int ws_tls_put(unsigned idx, void * tls)
{
  wasm_assert(idx < WASM_MAX_TLS_NUM);
  pthread_setspecific(thread_local_storage_key[idx], tls);
  return BHT_OK;
}

int ws_mutex_init(korp_mutex *mutex, bool is_recursive)
{
  if (!is_recursive)
    return pthread_mutex_init(mutex, NULL) == 0 ? BHT_OK : BHT_ERROR;
  else {
    int ret;

    pthread_mutexattr_t mattr;

    wasm_assert(mutex);
    ret = pthread_mutexattr_init(&mattr);
    if(ret) return BHT_ERROR;

    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);
    ret = pthread_mutex_init(mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    return ret == 0 ? BHT_OK : BHT_ERROR;
  }
}

int ws_mutex_destroy(korp_mutex *mutex)
{
  int ret;

  wasm_assert(mutex);
  ret = pthread_mutex_destroy(mutex);

  return ret == 0 ? BHT_OK : BHT_ERROR;
}


/* Returned error (EINVAL, EAGAIN and EDEADLK) from
locking the mutex indicates some logic error present in
the program somewhere.
Don't try to recover error for an existing unknown error.*/
void ws_mutex_lock(korp_mutex *mutex)
{
  int ret;

  wasm_assert(mutex);
  ret = pthread_mutex_lock(mutex);
  if (0 != ret) {
    fprintf(stderr, "vm mutex lock failed (ret=%d)!\n", ret);
    exit(-1);
  }
}

int ws_mutex_trylock(korp_mutex *mutex)
{
  int ret;

  wasm_assert(mutex);
  ret = pthread_mutex_trylock(mutex);

  return ret == 0 ? BHT_OK : BHT_ERROR;
}

/* Returned error (EINVAL, EAGAIN and EPERM) from
unlocking the mutex indicates some logic error present
in the program somewhere.
Don't try to recover error for an existing unknown error.*/
void ws_mutex_unlock(korp_mutex *mutex)
{
  int ret;

  wasm_assert(mutex);
  ret = pthread_mutex_unlock(mutex);
  if (0 != ret) {
    fprintf(stderr, "vm mutex unlock failed (ret=%d)!\n", ret);
    exit(-1);
  }
}

int ws_thread_cancel(korp_tid thread)
{
  return pthread_cancel(thread);
}

int ws_thread_join(korp_tid thread, void **value_ptr, int mills)
{
  return pthread_join(thread, value_ptr);
}

int ws_thread_detach(korp_tid thread)
{
  return pthread_detach(thread);
}

