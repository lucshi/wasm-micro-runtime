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


static pthread_key_t thread_local_storage_key;

int ws_thread_sys_init()
{
  return pthread_key_create(&thread_local_storage_key, NULL);
}

void ws_thread_sys_destroy()
{
  pthread_key_delete(thread_local_storage_key);
}

korp_tid ws_self_thread()
{
  return (korp_tid) pthread_self();
}

void *ws_tls_get()
{
  return pthread_getspecific(thread_local_storage_key);
}

int ws_tls_put(void * tls)
{
  return pthread_setspecific(thread_local_storage_key, tls);
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

