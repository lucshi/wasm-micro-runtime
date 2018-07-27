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

#include "wasm-loader.h"
#include "wasm.h"
#include "bh_memory.h"


/* Read a value of given type from the address pointed to by the given
   pointer and increase the pointer to the position just after the
   value being read.  */
#define TEMPLATE_READ_VALUE(Type, p)                    \
    (p += sizeof(Type), *(Type *)(p - sizeof(Type)))

#define read_uint8(p)  TEMPLATE_READ_VALUE(uint8, p)
#define read_int8(p)   TEMPLATE_READ_VALUE(int8, p)
#define read_uint16(p) TEMPLATE_READ_VALUE(uint16, p)
#define read_int16(p)  TEMPLATE_READ_VALUE(int16, p)
#define read_uint32(p) TEMPLATE_READ_VALUE(uint32, p)
#define read_int32(p)  TEMPLATE_READ_VALUE(int32, p)

/* Read a value of given type from the aligned address pointed to by
   the given pointer and increase the pointer to the position just
   after the value being read.  */
#define TEMPLATE_READ_VALUE_ALIGN(Type, p)                              \
  (p = (uint8 *)align_uint((unsigned)p, sizeof (Type)), read_##Type (p))

#define read_uint16_align(p) TEMPLATE_READ_VALUE_ALIGN(uint16, p)
#define read_int16_align(p)  TEMPLATE_READ_VALUE_ALIGN(int16, p)
#define read_uint32_align(p) TEMPLATE_READ_VALUE_ALIGN(uint32, p)
#define read_int32_align(p)  TEMPLATE_READ_VALUE_ALIGN(int32, p)

#define CHECK_BUF(buf, buf_end, length) do {                    \
  if (buf + length > buf_end) {                                 \
    printf("WASM module load failed: invalid file length.\n");  \
    return false;                                               \
  }                                                             \
} while (0)

static bool
serialize_table_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_memory_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_global_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_export_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_element_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_data_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_import_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_code_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_start_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_type_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize_function_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
serialize(const uint8 *buf, const uint8 *buf_end, WASMModule *module)
{
  const uint8 *p = buf, *p_end = buf_end;
  uint32 magic_number, version;

  CHECK_BUF(p, p_end, sizeof(uint32));
  if ((magic_number = read_uint32(p)) != WASM_MAGIC_NUMBER) {
    printf("WASM module load failed: invalid magic number.\n");
    return false;
  }

  CHECK_BUF(p, p_end, sizeof(uint32));
  if ((version = read_uint32(p)) != WASM_CURRENT_VERSION) {
    printf("WASM module load failed: invalid version.\n");
    return false;
  }

  while (p < p_end) {
    uint8 section_type = read_uint8(p);
    printf("section_type: %d\n", section_type);

    switch (section_type) {
      case SECTION_TYPE_USER:
        printf("WASM module load failed: unsupported user section.\n");
        return false;
      case SECTION_TYPE_TYPE:
        serialize_type_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_IMPORT:
        serialize_import_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_FUNC_DECL:
        serialize_function_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_TABLE:
        serialize_table_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_MEMORY:
        serialize_memory_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_GLOBAL:
        serialize_global_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_EXPORT:
        serialize_export_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_START:
        serialize_start_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_ELEM:
        serialize_element_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_FUNC_DEFS:
        serialize_code_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_DATA:
        serialize_data_section(&p, buf_end, module);
        break;
      default:
        printf("WASM module load failed: unknow section type %d.\n",
               section_type);
        return false;
    }
  }
  /* TODO */
  return false;
}

WASMModule*
wasm_wasm_module_load(const uint8 *buf, uint32 size)
{
  WASMModule *module = bh_malloc(sizeof(WASMModule));

  if (!module) {
    printf("WASM module load failed: alloc memory failed.\n");
    return NULL;
  }

  memset(module, 0, sizeof(WASMModule));
  module->start_function_index = UINTPTR_MAX;

  if (!bh_vector_init(&module->types,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(FunctionType)))
    goto fail;

  if (!bh_vector_init(&module->functions.imports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(FunctionImport)))
    goto fail;

  if (!bh_vector_init(&module->functions.defs,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(FunctionDef)))
    goto fail;

  if (!bh_vector_init(&module->tables.imports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(TableImport)))
    goto fail;

  if (!bh_vector_init(&module->tables.defs,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(TableDef)))
    goto fail;

  if (!bh_vector_init(&module->memories.imports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(MemoryImport)))
    goto fail;

  if (!bh_vector_init(&module->memories.defs,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(MemoryDef)))
    goto fail;

  if (!bh_vector_init(&module->globals.imports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(GlobalImport)))
    goto fail;

  if (!bh_vector_init(&module->globals.defs,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(GlobalDef)))
    goto fail;

  if (!bh_vector_init(&module->exception_types.imports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(ExceptionTypeImport)))
    goto fail;

  if (!bh_vector_init(&module->exception_types.defs,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(ExceptionTypeDef)))
    goto fail;

  if (!bh_vector_init(&module->exports,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(Export)))
    goto fail;

  if (!bh_vector_init(&module->data_segments,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(DataSegment)))
    goto fail;

  if (!bh_vector_init(&module->table_segments,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(TableSegment)))
    goto fail;

  if (!bh_vector_init(&module->user_sections,
                      DEFAULT_VECTOR_INIT_SIZE, sizeof(UserSection)))
    goto fail;

  if (!serialize(buf, buf + size, module))
    goto fail;

  return module;

fail:
  wasm_wasm_module_unload(module);
  return NULL;
}

bool
wasm_wasm_module_unload(WASMModule *module)
{
  uint32 i, size;
  FunctionDef func_def;
  DataSegment data_seg;
  TableSegment table_seg;
  UserSection user_sec;

  if (!module)
    return false;

  bh_vector_destroy(&module->types);
  bh_vector_destroy(&module->functions.imports);

  size = bh_vector_size(&module->functions.defs);
  for (i = 0; i < size; i++) {
    bh_vector_get(&module->functions.defs, i, &func_def);
    bh_vector_destroy(&func_def.non_parameter_local_types);
    bh_vector_destroy(&func_def.code);
    /* TODO: destroy each branch */
    bh_vector_destroy(&func_def.branch_tables);
  }
  bh_vector_destroy(&module->functions.defs);

  bh_vector_destroy(&module->functions.imports);
  bh_vector_destroy(&module->functions.defs);
  bh_vector_destroy(&module->memories.imports);
  bh_vector_destroy(&module->memories.defs);
  bh_vector_destroy(&module->globals.imports);
  bh_vector_destroy(&module->globals.defs);
  bh_vector_destroy(&module->exception_types.imports);
  bh_vector_destroy(&module->exception_types.defs);
  bh_vector_destroy(&module->exports);

  size = bh_vector_size(&module->data_segments);
  for (i = 0; i < size; i++) {
    bh_vector_get(&module->data_segments, i, &data_seg);
    bh_vector_destroy(&data_seg.data);
  }
  bh_vector_destroy(&module->data_segments);

  size = bh_vector_size(&module->table_segments);
  for (i = 0; i < size; i++) {
    bh_vector_get(&module->table_segments, i, &table_seg);
    bh_vector_destroy(&table_seg.indices);
  }
  bh_vector_destroy(&module->table_segments);

  size = bh_vector_size(&module->user_sections);
  for (i = 0; i < size; i++) {
    bh_vector_get(&module->user_sections, i, &user_sec);
    bh_vector_destroy(&user_sec.data);
  }
  bh_vector_destroy(&module->user_sections);

  bh_free(module);
  return true;
}
