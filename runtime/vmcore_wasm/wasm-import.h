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

#ifndef _WASM_IMPORT_H
#define _WASM_IMPORT_H

#include "wasm_platform.h"
#include "wasm_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZEPHYR__
typedef korp_thread wsci_thread_data_t;
#endif
typedef korp_tid wsci_thread_t;
typedef korp_mutex wsci_thread_mutex_t;
typedef thread_start_routine_t wsci_thread_start_routine_t;

#define wsci_thread_start_routine_modifier WASM_ROUTINE_MODIFIER

#define wsci_reserved_native_stack_size \
    WASM_APPLET_PRESERVED_NATIVE_STACK_SIZE

#define wsci_reserved_wasm_stack_size \
    WASM_APPLET_PRESERVED_WASM_STACK_SIZE

/**
 * Initialize the thread system.
 *
 * @return true if success, false otherwise.
 */
bool
wsci_thread_sys_init();

/**
 * Destroy the thread system.
 */
void
wsci_thread_sys_destroy();

/**
 * Return a thread local pointer.
 *
 * @return the thread local pointer
 */
void*
wsci_get_tl_root(void);

/**
 * Set the thread local pointer.
 *
 * @param tlr the pointer to be stored in the thread local storage
 */
void
wsci_set_tl_root(void *tlr);

int
wsci_thread_create_with_prio(wsci_thread_t *thread,
                             wsci_thread_start_routine_t start_routine,
                             void *arg,
                             unsigned stack_size, int prio);

int
wsci_thread_create(wsci_thread_t *thread,
                   wsci_thread_start_routine_t start_routine, void *arg,
                   unsigned stack_size);

int
wsci_thread_cancel(wsci_thread_t thread);

int
wsci_thread_join(wsci_thread_t thread, void **value_ptr, int mills);

int
wsci_thread_detach(wsci_thread_t thread);

void
wsci_thread_exit(void *value_ptr);

wsci_thread_t
wsci_thread_self();

int wsci_thread_mutex_init (wsci_thread_mutex_t *mutex);

int wsci_thread_mutex_init_recursive (wsci_thread_mutex_t *mutex);

void
wsci_thread_mutex_lock(wsci_thread_mutex_t *mutex);

int
wsci_thread_mutex_trylock(wsci_thread_mutex_t *mutex);

void
wsci_thread_mutex_unlock(wsci_thread_mutex_t *mutex);

int
wsci_thread_mutex_destroy(wsci_thread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_IMPORT_H */
