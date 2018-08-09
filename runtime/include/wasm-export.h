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

#ifndef _WASM_EXPORT_H
#define _WASM_EXPORT_H

#include "bh_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMModule;
typedef struct WASMModule *wasm_module_t;

struct WASMModuleInstance;
typedef struct WASMModuleInstance *wasm_module_inst_t;

struct WASMVmInstance;
typedef struct WASMVmInstance *wasm_vm_instance_t;

/**
 * Load a WASM module from a specified byte buffer.
 *
 * @param buf the byte buffer which contains the WASM binary data
 * @param size the size of the buffer
 *
 * @return return module loaded, NULL if failed
 */
wasm_module_t
wasm_runtime_load(const uint8 *buf, uint32 size);

/**
 * Unload a WASM module.
 *
 * @param module the module to be unloaded
 */
void
wasm_runtime_unload(wasm_module_t module);

/**
 * Instantiate a WASM module
 *
 * @param module the WASM module to instantiate
 *
 * @return return the instantiated WASM module instance, NULL if failed
 */
wasm_module_inst_t
wasm_runtime_instantiate(const wasm_module_t module);

/**
 * Destroy a WASM module instance
 *
 * @param module_inst the WASM module instance to destroy
 */
void
wasm_runtime_deinstantiate(wasm_module_inst_t module_inst);

wasm_vm_instance_t
wasm_runtime_create_instance(wasm_module_inst_t module_inst,
                             unsigned stack_size,
                             void *(*start_routine)(void*), void *arg,
                             void (*cleanup_routine)(void));

void
wasm_runtime_destroy_instance(wasm_vm_instance_t vm);

void
wasm_runtime_wait_for_instance(wasm_vm_instance_t vm);


#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_EXPORT_H */
