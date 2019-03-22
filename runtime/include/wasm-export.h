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

#include <inttypes.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Uninstantiated WASM module loaded from WASM binary file */
struct WASMModule;
typedef struct WASMModule *wasm_module_t;

/* Instantiated WASM module */
struct WASMModuleInstance;
typedef struct WASMModuleInstance *wasm_module_inst_t;

/* Function instance */
struct WASMFunctionInstance;
typedef struct WASMFunctionInstance *wasm_function_inst_t;

/* Execution environment, e.g. stack info */
typedef struct WASMExecEnv {
  uint8_t *stack;
  uint32_t stack_size;
} *wasm_exec_env_t;

/* Package Type */
typedef enum {
  Wasm_Module_Bytecode = 0,
  Wasm_Module_AoT,
  Package_Type_Unknown = 0xFFFF
} package_type_t;


/**
 * Initialize the WASM runtime environment.
 *
 * @return true if success, false otherwise
 */
bool
wasm_runtime_init();

/**
 * Destroy the WASM runtime environment.
 */
void
wasm_runtime_destroy();

/**
 * Get the package type of a buffer.
 *
 * @param buf the package buffer
 * @param size the package buffer size
 *
 * @return the package type, return Package_Type_Unknown if the type is unknown
 */
package_type_t
get_package_type(const uint8_t *buf, uint32_t size);

/**
 * Load a WASM module from a specified byte buffer.
 *
 * @param buf the byte buffer which contains the WASM binary data
 * @param size the size of the buffer
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return WASM module loaded, NULL if failed
 */
wasm_module_t
wasm_runtime_load(const uint8_t *buf, uint32_t size,
                  char *error_buf, uint32_t error_buf_size);

/**
 * Unload a WASM module.
 *
 * @param module the module to be unloaded
 */
void
wasm_runtime_unload(wasm_module_t module);

/**
 * Instantiate a WASM module.
 *
 * @param module the WASM module to instantiate
 * @param stack_size the default stack size of the module instance, a stack will be created
 *     when function wasm_runtime_call_wasm() is called to run WASM function and the
 *     exec_env argument passed to wasm_runtime_call_wasm() is NULL. That means this parameter is
 *     ignored if exec_env is not NULL.
 * @param error_buf buffer to output the error info if failed
 * @param error_buf_size the size of the error buffer
 *
 * @return return the instantiated WASM module instance, NULL if failed
 */
wasm_module_inst_t
wasm_runtime_instantiate(const wasm_module_t module,
                         uint32_t stack_size,
                         char *error_buf, uint32_t error_buf_size);

/**
 * Deinstantiate a WASM module instance, destroy the resources.
 *
 * @param module_inst the WASM module instance to destroy
 */
void
wasm_runtime_deinstantiate(wasm_module_inst_t module_inst);

/**
 * Load WASM module instance from AOT file.
 *
 * @param aot_file the AOT file of a WASM module
 * @param aot_file_size the AOT file size
 * @param error_buf buffer to output the error info if failed
 * @param error_buf_size the size of the error buffer
 *
 * @return the instantiated WASM module instance, NULL if failed
 */
wasm_module_inst_t
wasm_runtime_load_aot(uint8_t *aot_file, uint32_t aot_file_size,
                      char *error_buf, uint32_t error_buf_size);

/**
 * Lookup an exported function in the WASM module instance.
 *
 * @param module_inst the module instance
 * @param name the name of the function
 * @param signature the signature of the function, use "i32"/"i64"/"f32"/"f64"
 *        to represent the type of i32/i64/f32/f64, e.g. "(i32i64)" "(i32)f32"
 *
 * @return the function instance found, if the module instance is loaded from AOT file,
 *        the return value is the function pointer
 */
wasm_function_inst_t
wasm_runtime_lookup_function(const wasm_module_inst_t module_inst,
                             const char *name, const char *signature);

/**
 * Create execution environment.
 *
 * @param stack_size the stack size to execute a WASM function
 *
 * @return the execution environment
 */
wasm_exec_env_t
wasm_runtime_create_exec_env(uint32_t stack_size);

/**
 * Destroy the execution environment.
 *
 * @param env the execution environment to destroy
 */
void
wasm_runtime_destory_exec_env(wasm_exec_env_t env);

/**
 * Call the given WASM function of a WASM module instance with arguments (bytecode and AoT).
 *
 * @param module_inst the WASM module instance which the function belongs to
 * @param exec_env the execution environment to call the function. If the module instance is created
 *     by AoT mode, it is ignored and just set it to NULL. If the module instance is created by bytecode
 *     mode and it is NULL, a temporary env object will be created
 * @param function the function to be called
 * @param argc the number of arguments
 * @param argv the arguments.  If the function method has return value,
 *   the first (or first two in case 64-bit return value) element of
 *   argv stores the return value of the called WASM function after this
 *   function returns.
 *
 * @return true if success, false otherwise and exception will be thrown,
 *   the caller can call wasm_runtime_get_exception to get exception info.
 */
bool
wasm_runtime_call_wasm(wasm_module_inst_t module_inst,
                       wasm_exec_env_t exec_env,
                       wasm_function_inst_t function,
                       uint32_t argc, uint32_t argv[]);

/**
 * Get exception info of the WASM module instance.
 *
 * @param module_inst the WASM module instance
 *
 * @return the exception string
 */
const char*
wasm_runtime_get_exception(wasm_module_inst_t module_inst);

/**
 * Clear exception info of the WASM module instance.
 *
 * @param module_inst the WASM module instance
 */
void
wasm_runtime_clear_exception(wasm_module_inst_t module_inst);

/**
 * Attach the current native thread to a WASM module instance.
 * A native thread cannot be attached simultaneously to two WASM module
 * instances. The WASM module instance will be attached to the native
 * thread which it is instantiated in by default.
 *
 * @param module_inst the WASM module instance to attach
 * @param thread_data the thread data that current native thread requires
 *        the WASM module instance to store
 *
 * @return true if SUCCESS, false otherwise
 */
bool
wasm_runtime_attach_current_thread(wasm_module_inst_t module_inst,
                                   void *thread_data);

/**
 * Detach the current native thread from a WASM module instance.
 *
 * @param module_inst the WASM module instance to detach
 */
void
wasm_runtime_detach_current_thread(wasm_module_inst_t module_inst);

/**
 * Get the thread data that the current native thread requires the WASM
 * module instance to store when attaching.
 *
 * @return the thread data stored when attaching
 */
void*
wasm_runtime_get_current_thread_data();

/**
 * Find the unique main function from a WASM module instance
 * and execute that function.
 *
 * @param module_inst the WASM module instance
 * @param argc the number of arguments
 * @param argv the arguments array
 *
 * @return true if the main function is called, false otherwise.
 */
bool
wasm_application_execute_main(wasm_module_inst_t module_inst,
                              int argc, char *argv[]);

#ifdef WASM_ENABLE_REPL
/**
 * Find the specified function in argv[0] from WASM module of current instance
 * and execute that function.
 *
 * @param module_inst the WASM module instance
 * @param argc the number of arguments
 * @param argv the arguments array
 *
 * @return true if the specified function is called, false otherwise.
 */
bool
wasm_application_execute_func(wasm_module_inst_t module_inst,
                              int argc, char *argv[]);
#endif


#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_EXPORT_H */
