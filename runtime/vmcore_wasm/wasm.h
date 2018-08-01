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

#ifndef _WASM_H_
#define _WASM_H_

#include "bh_platform.h"
#include "bh_hashmap.h"
#include "bh_vector.h"
#include "wasm-import.h"
#include "bh_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Value Type */
#define VALUE_TYPE_ANY 0
#define VALUE_TYPE_I32 1
#define VALUE_TYPE_I64 2
#define VALUE_TYPE_F32 3
#define VALUE_TYPE_F64 4
#define VALUE_TYPE_V128 5
#define VALUE_TYPE_NUM 6
#define VALUE_TYPE_MAX 5

/** Calling Convention */
#define CALLING_CONV_WASM 0
#define CALLING_CONV_INTRINSIC 1
#define CALLING_CONV_INSTRINSIC_WITH_CONTEXT_SWITCH 2
#define CALLING_CONV_INSTRINSIC_WITH_MEM_AND_TABLE 3
#define CALLING_CONV_C 5

/* Table Element Type */
#define TABLE_ELEM_TYPE_ANY_FUNC 0x70

#define CompartmentReservedBytes (4ull * 1024 * 1024 * 1024)
#define MaxThunkArgAndReturnBytes 256
#define MaxGlobalBytes (4096 - MaxThunkArgAndReturnBytes)
#define MaxMemories 255
#define MaxTables 256
#define CompartmentRuntimeDataAlignmentLog2 32
#define ContextRuntimeDataAlignment 4096

#define MaxMemoryPages 65536
#define MaxTableElems UINT32_MAX
#define NumBytesPerPage 65536
#define NumBytesPerPageLog2 16
#define MaxReturnValues 16

#define INIT_EXPR_TYPE_I32_CONST 0x41
#define INIT_EXPR_TYPE_I64_CONST 0x42
#define INIT_EXPR_TYPE_F32_CONST 0x43
#define INIT_EXPR_TYPE_F64_CONST 0x44
#define INIT_EXPR_TYPE_GET_GLOBAL 0x23
#define INIT_EXPR_TYPE_ERROR 0xff

#define WASM_MAGIC_NUMBER 0x6d736100
#define WASM_CURRENT_VERSION 1

#define SECTION_TYPE_USER 0
#define SECTION_TYPE_TYPE 1
#define SECTION_TYPE_IMPORT 2
#define SECTION_TYPE_FUNC 3
#define SECTION_TYPE_TABLE 4
#define SECTION_TYPE_MEMORY 5
#define SECTION_TYPE_GLOBAL 6
#define SECTION_TYPE_EXPORT 7
#define SECTION_TYPE_START 8
#define SECTION_TYPE_ELEM 9
#define SECTION_TYPE_CODE 10
#define SECTION_TYPE_DATA 11

#if 0
struct ModuleInstance;
struct Compartment;
struct ContextRuntimeData;
struct CompartmentRuntimeData;

typedef struct FunctionInstance {
  ObjectImpl obj_impl;
  struct ModuleInstance *module_instance;
  FunctionType type;
  void *native_function;
  uint8 calling_convention;
  const char *debug_name;
} FunctionInstance;

typedef struct FunctionElement {
  uintptr_t type_encoding;
  void *value;
} FunctionElement;

typedef struct TableInstance {
  ObjectImpl obj_impl;
  struct Compartment *compartment;
  uint32 id;
  TableType type;
  FunctionElement *base_address;
  uint8 *end_address;
  vmci_thread_mutex_t elements_lock;
  Vector elements;
} TableInstance;

typedef struct MemoryInstance {
  ObjectImpl obj_impl;
  struct Compartment *compartment;
  uint32 id;
  MemoryType type;
  uint8 *base_address;
  uint8 *end_address;
  uint32 num_pages;
} MemoryInstance;

typedef struct GlobalInstance {
  ObjectImpl obj_impl;
  struct Compartment *compartment;
  const GlobalType type;
  const uint32 mutable_data_offset;
  const UntaggedValue initial_value;
} GlobalInstance;

typedef struct ExceptionTypeInstance {
  ObjectImpl obj_impl;
  ExceptionType type;
  const char *debug_name;
} ExceptionTypeInstance;

typedef struct ExceptionData {
  ExceptionTypeInstance *type_instance;
  uint8 is_user_exception;
  UntaggedValue arguments[1];
} ExceptionData;

typedef struct ModuleInstance {
  ObjectImpl obj_impl;
  struct Compartment *compartment;
  HashMap *export_map;
  Vector function_defs;
  Vector functions;
  Vector tables;
  Vector memories;
  Vector globals;
  Vector exception_type_instances;
  FunctionInstance *start_function;
  MemoryInstance *default_memory;
  TableInstance *default_table;
  const char *debug_name;
} ModuleInstance;

typedef struct Context {
  ObjectImpl obj_impl;
  struct Compartment *compartment;
  uint32 id;
  struct ContextRuntimeData *runtime_data;
} Context;

typedef struct Compartment {
  ObjectImpl obj_imp;
  vmci_thread_mutex_t lock;
  struct CompartmentRuntimeData *runtime_data;
  uint8 *unaligned_runtime_data;
  uint32 num_global_bytes;
  Vector globals;
  Vector memories;
  Vector tables;
  Vector contexts;
  uint8 initial_context_global_data[MaxGlobalBytes];
  ModuleInstance *wavm_intrinsics;
} Compartment;

typedef struct ContextRuntimeData {
  uint8 thunk_arg_and_return_data[MaxThunkArgAndReturnBytes];
  uint8 global_data[MaxGlobalBytes];
} ContextRuntimeData;

typedef struct CompartmentRuntimeData {
  Compartment *compartment;
  uint8 *memories[MaxMemories];
  FunctionElement *tables[MaxTables];
  ContextRuntimeData contexts[1];
} CompartmentRuntimeData;
#endif

typedef struct InitializerExpression {
  /* type of INIT_EXPR_TYPE_XXX */
  uint8 type;
  union {
    int32 i32;
    int64 i64;
    float32 f32;
    float64 f64;
    uint32 global_index;
    /* pointer of global data after linked */
    void *global_data_ptr;
  } u;
} InitializerExpression;

typedef struct WASMType {
  uint32 param_count;
  /* only one result is supported currently */
  uint32 result_count;
  /* types of params and results */
  uint8 types[1];
} WASMType;

typedef struct WASMTable {
  uint32 flags;
  uint32 init_size;
  /* specified if (flags & 1), else it is 0x10000 */
  uint32 max_size;
} WASMTable;

typedef struct WASMMemory {
  uint32 flags;
  /* 64 kbytes one page by default */
  uint32 page_count;
} WASMMemory;

typedef struct WASMImport {
  char *module_name;
  char *field_name;
  uint8 kind;
  union {
    struct {
      /* function type */
      WASMType *func_type;
      /* function pointer */
      void *func_ptr;
    } function;
    WASMTable table;
    WASMMemory memory;
    struct {
      uint8 type;
      bool is_mutable;
      /* global data pointer after linked */
      void *global_data_ptr;
    } global;
  } u;
} WASMImport;

typedef struct WASMFunction {
  /* the type of function */
  WASMType *func_type;
  Vector branches;
  uint32 local_count;
  uint32 code_size;
  uint8 code[1];
} WASMFunction;

typedef struct WASMGlobal {
  uint8 type;
  bool is_mutable;
  InitializerExpression init_expr;
} WASMGlobal;

typedef struct WASMExport {
  char *name;
  uint8 kind;
  uint32 index;
  union {
    WASMFunction function;
    WASMTable table;
    WASMMemory memory;
    WASMGlobal global;
  } u;
} WASMExport;

typedef struct WASMTableSeg {
  uint32 table_index;
  InitializerExpression base_offset;
  uint32 function_count;
  void *functions;
} WASMTableSeg;

typedef struct WASMDataSeg {
  uint32 memory_index;
  InitializerExpression base_offset;
  uint32 data_length;
  uint8 data[1];
} WASMDataSeg;

typedef struct WASMModule {
  uint32 type_count;
  uint32 import_count;
  uint32 function_count;
  uint32 table_count;
  uint32 memory_count;
  uint32 global_count;
  uint32 export_count;
  uint32 table_seg_count;
  uint32 data_seg_count;

  WASMType **types;
  WASMImport *imports;
  WASMFunction **functions;
  WASMTable *tables;
  WASMMemory *memories;
  WASMGlobal *globals;
  WASMExport *exports;
  WASMTableSeg *table_segments;
  WASMDataSeg **data_segments;
  WASMFunction *start_function;

  HashMap *const_str_set;
} WASMModule;

/**
 * Align an unsigned value on a alignment boundary.
 *
 * @param v the value to be aligned
 * @param b the alignment boundary (2, 4, 8, ...)
 *
 * @return the aligned value
 */
inline static unsigned
align_uint (unsigned v, unsigned b)
{
  unsigned m = b - 1;
  return (v + m) & ~m;
}

inline static uint32
wasm_string_hash(const char *str)
{
  unsigned h = strlen(str);
  const uint8 *p = (uint8*)str;
  const uint8 *end = p + h;

  while (p != end)
    h = ((h << 5) - h) + *p++;
  return h;
}

inline static bool
wasm_string_equal(const char *s1, const char *s2)
{
  return strcmp(s1, s2) == 0 ? true : false;
}

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* end of _WASM_H_ */

