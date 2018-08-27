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
#include "bh_hashmap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WASMMemoryInstance {
  /* Current page count */
  uint32 cur_page_count;
  /* Maximum page count */
  uint32 max_page_count;
  /* Base address, the layout is:
     memory data + global data
     memory data size is NumBytesPerPage * cur_page_count
     global data size is calculated in module instantiating
     Note: when memory is re-allocated, the global data must
           be copied again.
   */
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
  /* data offset to base_addr of WASMMemoryInstance */
  uint32 data_offset;
  /* initial value */
  WASMValue initial_value;
} WASMGlobalInstance;

typedef struct WASMFunctionInstance {
  /* whether it is import function or WASM function */
  bool is_import_func;
  /* cell num of parameters */
  uint16 param_cell_num;
  /* cell num of return type */
  uint16 ret_cell_num;
  /* cell num of local variables, 0 for import function */
  uint16 local_cell_num;
  union {
    WASMFunctionImport *func_import;
    WASMFunction *func;
  } u;
} WASMFunctionInstance;

typedef struct WASMExportFuncInstance {
  char *name;
  WASMFunctionInstance *function;
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
  HashMap *branch_set;

  /* global data of globals, point to
     default_memory->base_addr + memory size */
  uint8 *global_data;
} WASMModuleInstance;

struct WASMInterpFrame;
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
 * Return the code block of a function.
 *
 * @param func the WASM function instance
 *
 * @return the code block of the function
 */
static inline uint8*
wasm_runtime_get_func_code(WASMFunctionInstance *func)
{
  return func->is_import_func ? NULL : func->u.func->code;
}

/**
 * Return the code block end of a function.
 *
 * @param func the WASM function instance
 *
 * @return the code block end of the function
 */
static inline uint8*
wasm_runtime_get_func_code_end(WASMFunctionInstance *func)
{
  return func->is_import_func
         ? NULL : func->u.func->code + func->u.func->code_size;
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
wasm_runtime_call_wasm(WASMFunctionInstance *function,
                       unsigned argc, uint32 argv[]);


#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_RUNTIME_H */
