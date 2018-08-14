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

/**
 * @file bh_thread.h
 * @brief This file contains Beihai platform abstract layer interface for
 *        thread relative function.
 */

#ifndef _BH_THREAD_H
#define _BH_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bh_config.h"
#include "bh_platform.h"

#define BH_MAX_THREAD 32
#define BH_MAX_TLS_NUM 2

#define BHT_ERROR (-1)
#define BHT_TIMED_OUT (1)
#define BHT_OK (0)

#define BHT_NO_WAIT 0x00000000
#define BHT_WAIT_FOREVER 0xFFFFFFFF

/**
 * Initialization function for vm thread system.
 * Invoked at the beginning of runtime initialization.
 *
 * @return BH_SUCCESS if succuess.
 */
int vm_thread_sys_init(void);

/**
 * Destroy the vm thread system.
 */
void vm_thread_sys_destroy();

/**
 * This function creates a thread
 *
 * @param p_tid  [OUTPUT] the pointer of tid
 * @param start  main routine of the thread
 * @param arg  argument passed to main routine
 * @param stack_size  bytes of stack size
 *
 * @return BH_SUCCESS if success.
 */
int vm_thread_create(korp_tid *p_tid, thread_start_routine_t start,
                     void *arg, unsigned int stack_size);

/**
 * This function creates a thread
 *
 * @param p_tid  [OUTPUT] the pointer of tid
 * @param start  main routine of the thread
 * @param arg  argument passed to main routine
 * @param stack_size  bytes of stack size
 * @param prio the priority
 *
 * @return BH_SUCCESS if success.
 */
int vm_thread_create_with_prio(korp_tid *p_tid, thread_start_routine_t start,
                               void *arg, unsigned int stack_size, int prio);

/**
 * This function never returns.
 *
 * @param code not used
 */
void vm_thread_exit(void *code);

/**
 * This function gets current thread id
 *
 * @return current thread id
 */
korp_tid vm_self_thread(void);

/**
 * This function send a cancellation request to a thread
 *
 * @param thread the thread id
 *
 * @return BH_SUCCESS if success.
 */
int vm_thread_cancel(korp_tid thread);

/**
 * This function waits for the thread specified by thread to terminate.
 *
 * @param thread the thread id
 * @value_ptr if not NULL, return the exit status of the target thread
 * @mills maximum waiting timeout in milliseconds
 *
 * @return BH_SUCCESS if success.
 */
int vm_thread_join(korp_tid thread, void **value_ptr, int mills);

/**
 * This function marks the thread identified by thread as detached.
 *
 * @param thread the thread id
 *
 * @return BH_SUCCESS if success.
 */
int vm_thread_detach(korp_tid thread);

/**
 * This function saves a pointer in thread local storage.
 * One thread can only save one pointer.
 *
 * @param idx  tls array index
 * @param ptr  pointer need save as TLS
 *
 * @return BH_SUCCESS if success
 */
int vm_tls_put(unsigned idx, void *ptr);

/**
 * This function gets a pointer saved in TLS.
 *
 * @param idx  tls array index
 *
* @return the pointer saved in TLS.
 */
void *vm_tls_get(unsigned idx);

/**
 * This function creates a mutex
 *
 * @param mutex [OUTPUT] pointer to mutex initialized.
 * @param is_recursive whether the mutex is recursive
 *
 * @return BH_SUCCESS if success
 */
int vm_mutex_init(korp_mutex *mutex, bool is_recursive);

/**
 * This function destorys a mutex
 *
 * @param mutex  pointer to mutex need destory
 *
 * @return BH_SUCCESS if success
 */
int vm_mutex_destroy(korp_mutex *mutex);

/**
 * This function locks the mutex
 *
 * @param mutex  pointer to mutex need lock
 *
 * @return Void
 */
void vm_mutex_lock(korp_mutex *mutex);

/**
 * This function locks the mutex without waiting
 *
 * @param mutex  pointer to mutex need lock
 *
 * @return BH_SUCCESS if success
 */
int vm_mutex_trylock(korp_mutex *mutex);

/**
 * This function unlocks the mutex
 *
 * @param mutex  pointer to mutex need unlock
 *
 * @return Void
 */
void vm_mutex_unlock(korp_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif /* end of _BH_THREAD_H */

