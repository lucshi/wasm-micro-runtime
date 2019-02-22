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

struct WASMFunctionInstance;
typedef struct WASMFunctionInstance *wasm_function_inst_t;

/**
 * Load a WASM module from a specified byte buffer.
 *
 * @param buf the byte buffer which contains the WASM binary data
 * @param size the size of the buffer
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return module loaded, NULL if failed
 */
wasm_module_t
wasm_runtime_load(const uint8 *buf, uint32 size,
                  char *error_buf, uint32 error_buf_size);

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
 * @param argc the number of arguments
 * @param argv the arguments array
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return the instantiated WASM module instance, NULL if failed
 */
wasm_module_inst_t
wasm_runtime_instantiate(const wasm_module_t module,
                         int argc, char ** argv,
                         char *error_buf, uint32 error_buf_size);

/**
 * Destroy a WASM module instance
 *
 * @param module_inst the WASM module instance to destroy
 */
void
wasm_runtime_deinstantiate(wasm_module_inst_t module_inst);

/**
 * Initialize the runtime environment (global locks,
 * supervisor instance etc.) of the VM.
 */
bool
wasm_runtime_init();

/**
 * Destroy the runtime environment (global locks,
 * supervisor instance etc.) of the VM.
 */
void
wasm_runtime_destroy();

/**
 * Create a WASM VM instance with the given WASM module instance.
 *
 * @param module_inst the WASM module instance
 * @param native_stack_size the stack size of WASM native stack
 * @param wasm_stack_size the stack size of WASM functions of
 * the new instance
 * @param start_routine start routine of the main thread
 * @param arg the instance argument that will be passed to
 * the start routine
 * @param cleanup_routine the optional cleanup routine for the
 * instance, which may be NULL
 *
 * @return the VM instance handle if succeeds, NULL otherwise
 */
wasm_vm_instance_t
wasm_runtime_create_instance(wasm_module_inst_t module_inst,
                             uint32 native_stack_size,
                             uint32 wasm_stack_size,
                             void *(*start_routine)(void*), void *arg,
                             void (*cleanup_routine)(void));

/**
 * Destroy the given VM instance. It can be called from any VM thread.
 * If there are alive threads of the instance, they will be terminated
 * mandatorily and then the cleanup routine is called if it's not NULL.
 *
 * @param handle the handle of the instance to be destroyed
 */
void
wasm_runtime_destroy_instance(wasm_vm_instance_t handle);

/**
 * Wait for the given VM instance to terminate.
 *
 * @param handle the VM instance to be waited for
 * @param mills wait millseconds to return
 */
void
wasm_runtime_wait_for_instance(wasm_vm_instance_t handle, int mills);

/**
 * Execute start function of WASM module of current instance.
 *
 * @return true if the start function is called, false otherwise.
 */
bool
wasm_application_execute_start(void);

/**
 * Find the unique main function from WASM module of current instance
 * and execute that function.
 *
 * @param argc the number of arguments
 * @param argv the arguments array
 *
 * @return true if the main function is called, false otherwise.
 */
bool
wasm_application_execute_main(int argc, char *argv[]);

/**
 * Get current exception string.
 *
 * @return return exception string if exception is thrown, NULL otherwise.
 */
const char*
wasm_runtime_get_exception();

#ifdef WASM_ENABLE_REPL
/**
 * Find the specified function in argv[0] from WASM module of current instance
 * and execute that function.
 *
 * @param argc the number of arguments
 * @param argv the arguments array
 *
 * @return true if the specified function is called, false otherwise.
 */
bool
wasm_application_execute_func(int argc, char *argv[]);
#endif

/**
 * Lookup a export function in the WASM module instance
 *
 * @param module_inst the module instance
 * @param name the name of the function
 * @param signature the signature of the function, use "i32"/"i64"/"f32"/"f64"
 *        to represent the type of i32/i64/f32/f64, e.g. "(i32i64)" "(i32)f32"
 */
wasm_function_inst_t
wasm_runtime_lookup_function(const wasm_module_inst_t module_inst,
                             const char *name,
                             const char *signature);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_EXPORT_H */
