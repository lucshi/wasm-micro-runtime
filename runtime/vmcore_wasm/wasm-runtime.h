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

#ifndef _WASM_RUNTIME_H
#define _WASM_RUNTIME_H

#include "wasm.h"
#include "wasm-import.h"
#include "wasm-thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WASMMemoryInstance {
  /* Current page count */
  uint32 cur_page_count;
  /* Maximum page count */
  uint32 max_page_count;
  /* Base address */
  uint8 base_addr[1];
} WASMMemoryInstance;

typedef struct WASMTableInstance {
  /* The element type, TABLE_ELEM_TYPE_ANY_FUNC currently */
  uint8 elem_type;
  /* Current size */
  uint32 cur_size;
  /* Maximum size */
  uint32 max_size;
  /* Base address */
  uint8 base_addr[1];
} WASMTableInstance;

typedef struct WASMGlobalInstance {
  /* value type, VALUE_TYPE_I32/I64/F32/F64 */
  uint8 type;
  /* mutable or constant */
  bool is_mutable;
  /* data offset to WASMModuleInstance.global_data if mutalbe */
  uint32 mutable_data_offset;
  /* initial value if constant */
  WASMValue initial_value;
} WASMGlobalInstance;

typedef struct WASMFunctionInstance {
  /* whether it is import function or WASM function */
  bool is_import_func;
  union {
    WASMFunctionImport *func_import;
    WASMFunction *func;
  } u;
} WASMFunctionInstance;

typedef struct WASMExportFuncInstance {
  char *name;
  WASMFunction *function;
} WASMExportFuncInstance;

typedef struct WASMModuleInstance {
  uint32 memory_count;
  uint32 table_count;
  uint32 global_count;
  uint32 function_count;
  uint32 export_func_count;

  WASMMemoryInstance **memories;
  WASMTableInstance **tables;
  WASMGlobalInstance *globals;
  WASMFunctionInstance *functions;
  WASMExportFuncInstance *export_functions;

  WASMMemoryInstance *default_memory;
  WASMTableInstance *default_table;

  WASMFunctionInstance *start_function;

  /* global data for mutable globals */
  uint8 global_data[1];
} WASMModuleInstance;

typedef struct WASMInterpFrame WASMRuntimeFrame;

/**
 * Return the current thread.
 *
 * @return the current thread
 */
static inline WASMThread*
wasm_runtime_get_self()
{
  return (WASMThread*)vmci_get_tl_root();
}

/**
 * Set self as the current thread.
 *
 * @param self the thread to be set as current thread
 */
static inline void
wasm_runtime_set_tlr(WASMThread *self)
{
  vmci_set_tl_root(self);
}

/**
 * Call the given WASM function with the arguments.
 *
 * @param function the function to be called
 * @param argc the number of arguments
 * @param argv the arguments.  If the function method has return value,
 * the first (or first two in case 64-bit return value) element of
 * argv stores the return value of the called WASM function after this
 * function returns.
 */
void
wasm_runtime_call_wasm(WASMFunction *function,
                       unsigned argc, uint32 argv[]);

WASMModuleInstance*
wasm_runtime_instantiate(const WASMModule *module);

void
wasm_runtime_deinstantiate(WASMModuleInstance *module_inst);

WASMVmInstance*
wasm_runtime_create_instance(WASMModuleInstance *module_inst,
                             unsigned stack_size,
                             void *(*start_routine)(void*), void *arg,
                             void (*cleanup_routine)(void));

void
wasm_runtime_destroy_instance(WASMVmInstance *vm);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_RUNTIME_H */
