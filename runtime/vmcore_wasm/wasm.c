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
wasm_value_equal(Value *v1, Value *v2)
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

char *
wasm_memory_type_to_string(const MemoryType *type)
{
  /* TODO */
  return NULL;
}

uint8
wasm_object_get_type(Object *obj)
{
  /* TODO */
  return 0;
}

void
wasm_gc_pointer_create(GCPointer *ptr, Object *obj)
{
  /* TODO */
}

void
wasm_gc_pointer_destroy(GCPointer *ptr)
{
  /* TODO */
}

bool
wasm_global_type_equal(GlobalType *type1, GlobalType *type2)
{
  /* TODO */
  return false;
}

bool
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
wasm_exception_type_equal(const ExceptionType *type1,
                          const ExceptionType *type2)
{
  /* TODO */
  return false;
}

uint32
wasm_index_space_size()
{
  /* TODO */
  return 0;
}

