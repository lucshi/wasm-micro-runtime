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

#ifdef __cplusplus
extern "C" {
#endif

/** Object Kind */
#define OBJ_KIND_FUNCTION 0
#define OBJ_KIND_TABLE 1
#define OBJ_KIND_MEMORY 2
#define OBJ_KIND_GLOBAL 3
#define OBJ_KIND_EXCEPTION_TYPE_INSTANCE 4
#define OBJ_KIND_MODULE 5
#define OBJ_KIND_CONTEXT 6
#define OBJ_KIND_COMPARTMENT 7
#define OBJ_KIND_INVALID 0xff

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
#define SECTION_TYPE_FUNC_DECL 3
#define SECTION_TYPE_TABLE 4
#define SECTION_TYPE_MEMORY 5
#define SECTION_TYPE_GLOBAL 6
#define SECTION_TYPE_EXPORT 7
#define SECTION_TYPE_START 8
#define SECTION_TYPE_ELEM 9
#define SECTION_TYPE_FUNC_DEFS 10
#define SECTION_TYPE_DATA 11

typedef union V128 {
  uint8 u8[16];
  int8 i8[16];
  uint16 u16[8];
  int16 i16[8];
  uint32 u32[4];
  int32 i32[4];
  float32 f32[4];
  uint64 u64[2];
  int64 i64[2];
  float64 f64[2];
} V128;

typedef union UntaggedValue {
  int32 i32;
  uint32 u32;
  int64 i64;
  uint64 u64;
  float32 f32;
  float64 f64;
  V128 v128;
  uint8 bytes[16];
} UntaggedValue;

typedef struct Value {
  UntaggedValue value;
  /* Value Type, must be VALUE_TYPE_XXX */
  uint8 type;
} Value;

typedef struct Object {
  /* Object Kind, must be OBJ_KIND_XXX */
  uint8 kind;
} Object;

typedef struct ModuleImpl {
  /* HashMap of <char*, Function*> */
  HashMap *function_map;
  /* HashMap of <char*, Global*> */
  HashMap *global_map;
  /* HashMap of <char*, Memory*> */
  HashMap *memory_map;
  /* HashMap of <char*, Table*> */
  HashMap *table_map;
} ModuleImpl;

typedef struct Module {
  ModuleImpl *impl;
} Module;

typedef struct TypeTupleImpl {
  uint32 hash;
  uint32 num_elems;
  /* each elem is type of VALUE_TYPE_XXX */
  uint8 elems[1];
} TypeTupleImpl;

typedef struct TypeTuple {
  TypeTupleImpl *impl;
} TypeTuple;

typedef struct FunctionTypeImpl {
  uint32 hash;
  TypeTuple results;
  TypeTuple params;
} FunctionTypeImpl;

typedef struct FunctionType {
  FunctionTypeImpl *impl;
} FunctionType;

typedef struct Function {
  const char *name;
  FunctionType type;
  void *native_function;
  /* type of CALLING_CONV_XXX */
  uint8 calling_convention;
} Function;

typedef struct Global {
  const char *name;
  /* type of VALUE_TYPE_XXX */
  uint8 type;
  Value value;
} Global;

typedef struct GlobalType {
  uint8 value_type;
  bool is_mutable;
} GlobalType;

typedef struct MemoryType {
  bool is_shared;
  uint64 size_min;
  uint64 size_max;
} MemoryType;

typedef struct Memory {
  const char* name;
  const MemoryType type;
} Memory;

typedef struct TableType {
  /* must be TABLE_ELEM_TYPE_ANY_FUNC */
  uint8 element_type;
  bool is_shared;
  uint64 size_min;
  uint64 size_max;
} TableType;

typedef struct Table {
  const char* name;
  const TableType type;
} Table;

typedef struct ExceptionType {
  TypeTuple params;
} ExceptionType;

typedef struct InitializerExpression {
  /* type of INIT_EXPR_TYPE_XXX */
  uint8 type;
  union {
    int32 i32;
    int64 i64;
    float32 f32;
    float64 f64;
    uintptr_t global_index;
  } u;
} InitializerExpression;

typedef struct IndexedFunctionType {
  uintptr_t index;
} IndexedFunctionType;

typedef struct FunctionDef {
  IndexedFunctionType type;
  /* vector of uint8, VALUE_TYPE_XXX */
  Vector non_parameter_local_types;
  /* vector of uint8 */
  Vector code;
  /* Vector of Vector<uint32> */
  Vector branch_tables;
} FunctionDef;

typedef struct TableDef {
  TableType type;
} TableDef;

typedef struct MemoryDef {
  MemoryType type;
} MemoryDef;

typedef struct GlobalDef {
  GlobalType type;
  InitializerExpression initializer;
} GlobalDef;

typedef struct ExceptionTypeDef {
  ExceptionType type;
} ExceptionTypeDef;

typedef struct FunctionImport {
  IndexedFunctionType type;
  char *module_name;
  char *export_name;
} FunctionImport;

typedef struct TableImport {
  TableType type;
  char *module_name;
  char *export_name;
} TableImport;

typedef struct MemoryImport {
  MemoryType type;
  char *module_name;
  char *export_name;
} MemoryImport;

typedef struct GlobalImport {
  GlobalType type;
  char *module_name;
  char *export_name;
} GlobalImport;

typedef struct ExceptionTypeImport {
  TypeTuple type;
  char *module_name;
  char *export_name;
} ExceptionTypeImport;

typedef struct Export {
  char *name;
  /* kind of OBJECT_KIND_XXX */
  uint8 kind;
  uintptr_t index;
} Export;

typedef struct DataSegment {
  uintptr_t memory_index;
  InitializerExpression base_offset;
  /* vector of uint8 */
  Vector data;
} DataSegment;

typedef struct TableSegment {
  uintptr_t table_index;
  InitializerExpression base_offset;
  /* vector of uintptr_t */
  Vector indices;
} TableSegment;

typedef struct UserSection {
  char *name;
  /* vector of uint8 */
  Vector data;
} UserSection;

typedef struct FeatureSpec {
  bool mvp;
  bool import_export_mutable_globals;
  bool extended_name_section;
  bool simd;
  bool atomics;
  bool exception_handling;
  bool non_trapping_float_to_int;
  bool extended_sign_extension;
  bool multiple_results_and_block_params;
  bool shared_tables;
  bool required_shared_flag_for_atomic_operators;
} FeatureSpec;

typedef struct IndexSpace {
  Vector imports;
  Vector defs;
} IndexSpace;

typedef struct WASMModule {
  FeatureSpec feature_spec;
  /* vector of FunctionType */
  Vector types;
  /* IndexSpace of <FunctionImport, FunctionDef> */
  IndexSpace functions;
  /* IndexSpace of <TableImport, TableDef> */
  IndexSpace tables;
  /* IndexSpace of <MemoryImport, MemoryDef> */
  IndexSpace memories;
  /* IndexSpace of <GlobalImport, GlobalDef> */
  IndexSpace globals;
  /* IndexSpace of <ExceptionTypeImport, ExceptionTypeDef> */
  IndexSpace exception_types;
  /* vector of Export */
  Vector exports;
  /* vector of DataSegment */
  Vector data_segments;
  /* vector of TableSegment */
  Vector table_segments;
  /* vector of UserSection */
  Vector user_sections;
  uintptr_t start_function_index;
} WASMModule;

struct ModuleInstance;
struct Compartment;
struct ContextRuntimeData;
struct CompartmentRuntimeData;

typedef struct GCPointer {
  Object *value;
} GCPointer;

typedef struct RootResolver {
  struct Compartment *compartment;
  /* hash map of <char*, ModuleInstance*> */
  HashMap *module_name_to_instance_map;
} RootResolver;

typedef struct EmscriptenInstance {
  /* GCPointer of ModuleInstance */
  GCPointer env;
  /* GCPointer of ModuleInstance */
  GCPointer global;
  /* GCPointer of ModuleInstance */
  GCPointer emscripten_memory;
} EmscriptenInstance;

typedef struct MissingImport {
  char *module_name;
  char *export_name;
  /* type of object, OBJECT_TYPE_XXX */
  uint8 type;
} MissingImport;

typedef struct ImportBindings {
  /* vector of FunctionInstance* */
  Vector functions;
  /* vector of TableInstance* */
  Vector tables;
  /* vector of MemoryInstance* */
  Vector memories;
  /* vector of GlobalInstance* */
  Vector globals;
  /* vector of ExceptionTypeInstance* */
  Vector exception_types;
} ImportBindings;

typedef struct LinkResult {
  /* vector of MissingImport */
  Vector missing_imports;
  ImportBindings resolved_imports;
  bool success;
} LinkResult;

typedef struct ObjectImpl {
  Object obj;
  uint32 num_root_references;
} ObjectImpl;

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

char*
wasm_value_to_string(Value* v);

/**
 * Return whether two values are equal.
 */
bool
wasm_value_eq(Value *v1, Value *v2);

/**
 * Return whether obj is type of type.
 */
bool
wasm_object_is_type(Object *obj, uint8 type);

/**
 * Return the type of obj.
 */
uint8
wasm_object_get_type(Object *obj);

/**
 * Create a type tuple.
 */
TypeTuple*
wasm_type_tuple_create(uint32 num_elems, uint8 *elem_data);

/**
 * Destroy a type tuple.
 */
void
wasm_type_tuple_destroy(TypeTuple *type_tuple);

/**
 * Return element number of a type tuple.
 */
uint32
wasm_type_tuple_get_num_elems(TypeTuple *type_tuple);

/**
 * Return an element of a type tuple.
 */
uint8
wasm_type_tuple_get_elem(TypeTuple *type, uint32 index);

/**
 * Return the elements buffer of a type tuple.
 */
uint8*
wasm_type_tuple_get_elems(TypeTuple *type);

/**
 * Set an element of a type tuple.
 */
bool
wasm_type_tuple_set_elem(TypeTuple *type, uint32 index, uint8 elem);

/**
 * Set elements of a type tuple.
 */
bool
wasm_type_tuple_set_elems(TypeTuple *type, uint32 offset,
                          uint8 *elems, uint32 length);

/**
 * Initialize function def.
 */
bool
wasm_function_def_init(FunctionDef *func_def);

/**
 * Destroy function def.
 */
void
wasm_function_def_destroy(FunctionDef *func_def);

/**
 * Initialize data segment.
 */
bool
wasm_data_segment_init(DataSegment *data_seg);

/**
 * Destroy data segment.
 */
void
wasm_data_segment_destroy(DataSegment *data_seg);

/**
 * Initialize table segment.
 */
bool
wasm_table_segment_init(TableSegment *table_seg);

/**
 * Destroy table segment.
 */
void
wasm_table_segment_destroy(TableSegment *table_seg);

/**
 * Initialize user section.
 */
bool
wasm_user_section_init(UserSection *user_sec);

/**
 * Destroy user section.
 */
void
wasm_user_section_destroy(UserSection *user_sec);

char *
wasm_memory_type_to_string(const MemoryType *type);

/**
 * Return whether two global types are equal.
 */
bool
wasm_global_type_eq(GlobalType *type1, GlobalType *type2);

/**
 * Compare two global types.
 */
int
wasm_global_type_cmp(GlobalType *type1, GlobalType *type2);

char*
wasm_global_type_to_string(GlobalType *type);

/**
 * Return whether two exception types are equal.
 */
bool
wasm_exception_type_eq(const ExceptionType *type1,
                       const ExceptionType *type2);

/**
 * Calculate the bytes number of exception data.
 */
static inline uint32
wasm_exception_data_calc_num_bytes(uint32 num_arguments)
{
  return offsetof(ExceptionData, arguments) + num_arguments * sizeof(UntaggedValue);
}

/**
 * Return the size of index space.
 */
uint32
wasm_index_space_size(IndexSpace *index_space);

/**
 * Initialize gc pointer.
 */
bool
wasm_gc_pointer_init(GCPointer *ptr, Object *obj);

/**
 * Destroy gc pointer.
 */
void
wasm_gc_pointer_destroy(GCPointer *ptr);


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* end of _WASM_H_ */

