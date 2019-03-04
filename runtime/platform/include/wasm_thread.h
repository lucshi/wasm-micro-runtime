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
 * @file wasm_thread.h
 * @brief This file contains Beihai platform abstract layer interface for
 *        thread relative function.
 */

#ifndef _WASM_THREAD_H
#define _WASM_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wasm_config.h"
#include "wasm_platform.h"


#define BHT_ERROR (-1)
#define BHT_OK (0)

/**
 * Initialization function for vm thread system.
 * Invoked at the beginning of runtime initialization.
 *
 * @return BHT_OK if succuess.
 */
int ws_thread_sys_init(void);

/**
 * Destroy the vm thread system.
 */
void ws_thread_sys_destroy();

/**
 * This function gets current thread id
 *
 * @return current thread id
 */
korp_tid ws_self_thread(void);

/**
 * This function saves a pointer in thread local storage.
 * One thread can only save one pointer.
 *
 * @param ptr  pointer need save as TLS
 *
 * @return BHT_OK if success
 */
int ws_tls_put(void *ptr);

/**
 * This function gets a pointer saved in TLS.
 *
* @return the pointer saved in TLS.
 */
void *ws_tls_get();

/**
 * This function creates a mutex
 *
 * @param mutex [OUTPUT] pointer to mutex initialized.
 * @param is_recursive whether the mutex is recursive
 *
 * @return BHT_OK if success
 */
int ws_mutex_init(korp_mutex *mutex, bool is_recursive);

/**
 * This function destorys a mutex
 *
 * @param mutex  pointer to mutex need destory
 *
 * @return BHT_OK if success
 */
int ws_mutex_destroy(korp_mutex *mutex);

/**
 * This function locks the mutex
 *
 * @param mutex  pointer to mutex need lock
 *
 * @return Void
 */
void ws_mutex_lock(korp_mutex *mutex);

/**
 * This function unlocks the mutex
 *
 * @param mutex  pointer to mutex need unlock
 *
 * @return Void
 */
void ws_mutex_unlock(korp_mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_THREAD_H */

