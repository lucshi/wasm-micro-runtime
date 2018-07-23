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

#include "bh_platform.h"
#include "bh_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZEPHYR__
typedef korp_thread vmci_thread_data_t;
#endif
typedef korp_tid vmci_thread_t;
typedef korp_mutex vmci_thread_mutex_t;
typedef thread_start_routine_t vmci_thread_start_routine_t;

/**
 * Return a thread local pointer.
 *
 * @return the thread local pointer
 */
void* vmci_get_tl_root(void);

/**
 * Set the thread local pointer.
 *
 * @param tlr the pointer to be stored in the thread local storage
 */
void vmci_set_tl_root(void *tlr);

int vmci_thread_create_with_prio(vmci_thread_t *thread,
                                 vmci_thread_start_routine_t start_routine, void *arg,
                                 void *heap_for_stack, unsigned stack_size, int prio);

int vmci_thread_create(vmci_thread_t *thread,
                        vmci_thread_start_routine_t start_routine, void *arg,
                        void *heap_for_stack, unsigned stack_size);

int vmci_thread_cancel(vmci_thread_t thread);

int vmci_thread_join(vmci_thread_t thread, void **value_ptr, int mills);

int vmci_thread_detach(vmci_thread_t thread);

void vmci_thread_exit(void *value_ptr);

vmci_thread_t vmci_thread_self();

int vmci_thread_mutex_init(vmci_thread_mutex_t *mutex, bool is_recursive);

void vmci_thread_mutex_lock(vmci_thread_mutex_t *mutex);

int vmci_thread_mutex_trylock(vmci_thread_mutex_t *mutex);

void vmci_thread_mutex_unlock(vmci_thread_mutex_t *mutex);

int vmci_thread_mutex_destroy(vmci_thread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_IMPORT_H */
