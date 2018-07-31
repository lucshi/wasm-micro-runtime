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
load_table_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_memory_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_global_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_export_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_element_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_data_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_import_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_code_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_start_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_type_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_function_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load(const uint8 *buf, uint32 size, WASMModule *module)
{
  const uint8 *buf_end = buf + size;
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
        load_type_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_IMPORT:
        load_import_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_FUNC_DECL:
        load_function_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_TABLE:
        load_table_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_MEMORY:
        load_memory_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_GLOBAL:
        load_global_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_EXPORT:
        load_export_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_START:
        load_start_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_ELEM:
        load_element_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_FUNC_DEFS:
        load_code_section(&p, buf_end, module);
        break;
      case SECTION_TYPE_DATA:
        load_data_section(&p, buf_end, module);
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

  if (!load(buf, size, module))
    goto fail;

  return module;

fail:
  wasm_wasm_module_unload(module);
  return NULL;
}

bool
wasm_wasm_module_unload(WASMModule *module)
{
  bh_free(module);
  return true;
}
