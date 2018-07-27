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
#include "bh_memory.h"
#include "bh_vector.h"


char*
wasm_value_to_string(const Value* v)
{
  /* TODO */
  return NULL;
}

bool
wasm_type_tuple_init(TypeTuple *type_tuple,
                     uint32 num_elems, uint8 *elem_data)
{
  TypeTupleImpl *impl;

  if (!(impl = bh_malloc(offsetof(TypeTupleImpl, elems) +
                         sizeof(uint8) * num_elems)))
    return false;

  impl->num_elems = num_elems;
  if (elem_data)
    memcpy(impl->elems, elem_data, sizeof(uint8) *num_elems);
  type_tuple->impl = impl;

  return true;
}

void
wasm_type_tuple_destroy(TypeTuple *type_tuple)
{
  bh_free(type_tuple->impl);
  type_tuple->impl = NULL;
}

bool
wasm_function_def_init(FunctionDef *func_def)
{
  memset(func_def, 0, sizeof(FunctionDef));

  if (!bh_vector_init(&func_def->non_parameter_local_types,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(uint8)))
    return false;

  if (!bh_vector_init(&func_def->code,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(uint8)))
    goto fail1;

  if (!bh_vector_init(&func_def->branch_tables,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(Vector)))
    goto fail2;

  return true;

fail2:
  bh_vector_destroy(&func_def->branch_tables);

fail1:
  bh_vector_destroy(&func_def->code);
  return false;
}

void
wasm_function_def_destroy(FunctionDef *func_def)
{
  bh_vector_destroy(&func_def->non_parameter_local_types);
  bh_vector_destroy(&func_def->code);
  bh_vector_destroy(&func_def->branch_tables);
  memset(func_def, 0, sizeof(FunctionDef));
}

bool
wasm_data_segment_init(DataSegment *data_seg)
{
  memset(data_seg, 0, sizeof(DataSegment));
  return bh_vector_init(&data_seg->data, DEFAULT_VECTOR_INIT_SIZE,
                        sizeof(uint8));
}

void
wasm_data_segment_destroy(DataSegment *data_seg)
{
  bh_vector_destroy(&data_seg->data);
  memset(data_seg, 0, sizeof(DataSegment));
}

bool
wasm_table_segment_init(TableSegment *table_seg)
{
  memset(table_seg, 0, sizeof(TableSegment));
  return bh_vector_init(&table_seg->indices, DEFAULT_VECTOR_INIT_SIZE,
                        sizeof(uintptr_t));
}

void
wasm_table_segment_destroy(TableSegment *table_seg)
{
  bh_vector_destroy(&table_seg->indices);
  memset(table_seg, 0, sizeof(TableSegment));
}

bool
wasm_user_section_init(UserSection *user_sec)
{
  memset(user_sec, 0, sizeof(UserSection));
  return bh_vector_init(&user_sec->data, DEFAULT_VECTOR_INIT_SIZE,
                        sizeof(uint8));
}

void
wasm_user_section_destroy(UserSection *user_sec)
{
  bh_vector_destroy(&user_sec->data);
  memset(user_sec, 0, sizeof(UserSection));
}

char *
wasm_memory_type_to_string(const MemoryType *type)
{
  /* TODO */
  return NULL;
}

char*
wasm_global_type_to_string(const GlobalType *type)
{
  /* TODO */
  return false;
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

