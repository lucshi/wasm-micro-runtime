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

#include "wasm.h"


char*
wasm_value_to_string(Value* v)
{
  /* TODO */
  return NULL;
}

bool
wasm_value_eq(Value *v1, Value *v2)
{
  /* TODO */
  return false;
}

bool
wasm_object_is_type(Object *obj, uint8 type)
{
  /* TODO */
  return false;
}

uint8
wasm_object_get_type(Object *obj)
{
  /* TODO */
  return 0;
}

TypeTuple*
wasm_type_tuple_create(uint32 num_elems, uint8 *elem_data)
{
  /* TODO */
  return NULL;
}

void
wasm_type_tuple_destroy(TypeTuple *type_tuple)
{
  /* TODO */
}

uint32
wasm_type_tuple_get_num_elems(TypeTuple *type_tuple)
{
  /* TODO */
  return 0;
}

uint8
wasm_type_tuple_get_elem(TypeTuple *type, uint32 index)
{
  /* TODO */
  return 0;
}

uint8*
wasm_type_tuple_get_elems(TypeTuple *type)
{
  /* TODO */
  return NULL;
}

bool
wasm_type_tuple_set_elem(TypeTuple *type, uint32 index, uint8 elem)
{
  /* TODO */
  return false;
}

bool
wasm_type_tuple_set_elems(TypeTuple *type, uint32 offset,
                          uint8 *elems, uint32 length)
{
  /* TODO */
  return false;
}

bool
wasm_function_def_init(FunctionDef *func_def)
{
  /* TODO */
  return false;
}

void
wasm_function_def_destroy(FunctionDef *func_def)
{
  /* TODO */
}

bool
wasm_data_segment_init(DataSegment *data_seg)
{
  /* TODO */
  return false;
}

void
wasm_data_segment_destroy(DataSegment *data_seg)
{
  /* TODO */
}

bool
wasm_table_segment_init(TableSegment *table_seg)
{
  /* TODO */
  return false;
}

void
wasm_table_segment_destroy(TableSegment *table_seg)
{
  /* TODO */
}

bool
wasm_user_section_init(UserSection *user_sec)
{
  /* TODO */
  return false;
}

void
wasm_user_section_destroy(UserSection *user_sec)
{
  /* TODO */
}

char *
wasm_memory_type_to_string(const MemoryType *type)
{
  /* TODO */
  return NULL;
}

bool
wasm_global_type_eq(GlobalType *type1, GlobalType *type2)
{
  /* TODO */
  return false;
}

int
wasm_global_type_cmp(GlobalType *type1, GlobalType *type2)
{
  /* TODO */
  return false;
}

char*
wasm_global_type_to_string(GlobalType *type)
{
  /* TODO */
  return false;
}

bool
wasm_exception_type_eq(const ExceptionType *type1,
                       const ExceptionType *type2)
{
  /* TODO */
  return false;
}

uint32
wasm_index_space_size(IndexSpace *index_space)
{
  /* TODO */
  return 0;
}

bool
wasm_gc_pointer_init(GCPointer *ptr, Object *obj)
{
  /* TODO */
  return false;
}

void
wasm_gc_pointer_destroy(GCPointer *ptr)
{
  /* TODO */
}

