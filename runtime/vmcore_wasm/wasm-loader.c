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

#define CHECK_BUF(buf, buf_end, length) do {                    \
  if (buf + length > buf_end) {                                 \
    printf("WASM module load failed: invalid file length.\n");  \
    return false;                                               \
  }                                                             \
} while (0)

static bool
read_leb(const uint8 *buf, const uint8 *buf_end,
         uint32 *p_offset, uint32 maxbits,
         bool sign, uint64 *p_result)
{
  uint64 result = 0;
  uint32 shift = 0;
  uint32 bcnt = 0;
  uint32 start_pos = *p_offset;
  uint64 byte;

  CHECK_BUF(buf, buf_end, *p_offset);
  while (true) {
    byte = buf[*p_offset];
    *p_offset += 1;
    CHECK_BUF(buf, buf_end, *p_offset);
    result |= ((byte & 0x7f) << shift);
    shift += 7;
    if ((byte & 0x80) == 0) {
      break;
    }
    bcnt += 1;
    if (bcnt > (maxbits + 7 - 1) / 7) {
      printf("WASM module load failed: unsigned LEB at byte %d overflow",
             start_pos);
      return false;
    }
  }
  if (sign && (shift < maxbits) && (byte & 0x40)) {
    /* Sign extend */
    result |= - (1 << shift);
  }
  *p_result = result;
  return true;
}

#define read_leb_uint32(p, p_end, res) do {         \
  uint32 off = 0;                                   \
  uint64 res64;                                     \
  if (!read_leb(p, p_end, &off, 32, false, &res64)) \
    return false;                                   \
  p += off;                                         \
  res = (uint32)res64;                              \
} while (0)

#define read_leb_int32(p, p_end, res) do {          \
  uint32 off = 0;                                   \
  uint64 res64;                                     \
  if (!read_leb(p, p_end, &off, 32, true, &res64))  \
    return false;                                   \
  p += off;                                         \
  res = (uint32)res64;                              \
} while (0)

#define read_leb_uint8(p, p_end, res) do {          \
  uint32 off = 0;                                   \
  uint64 res64;                                     \
  if (!read_leb(p, p_end, &off, 7, false, &res64))  \
    return false;                                   \
  p += off;                                         \
  res = (uint32)res64;                              \
} while (0)

static bool
load_type_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  const uint8 *p = *p_buf, *p_end = buf_end, *p_org;
  uint32 section_size, type_count, param_count, result_count, i, j;
  uint8 flag;
  WASMType *type;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, type_count);

  if (type_count) {
    module->type_count = type_count;
    if (!(module->types = bh_malloc(sizeof(WASMType*) * type_count))) {
      printf("Load type section failed: alloc memory failed.\n");
      return false;
    }

    memset(module->types, 0, sizeof(WASMType*) * type_count);

    for (i = 0; i < type_count; i++) {
      CHECK_BUF(p, p_end, 1);
      flag = read_uint8(p);
      if (flag != 0x60) {
        printf("Load type section failed: invalid type flag.\n");
        return false;
      }

      read_leb_uint32(p, p_end, param_count);

      p_org = p;
      CHECK_BUF(p, p_end, param_count);
      read_leb_uint32(p, p_end, result_count);
      bh_assert(result_count == 1);
      CHECK_BUF(p, p_end, result_count);
      p = p_org;

      if (!(type = module->types[i] = bh_malloc(offsetof(WASMType, types) +
              sizeof(uint8) * (param_count + result_count))))
        return false;

      type->param_count = param_count;
      type->result_count = result_count;
      for (j = 0; j < param_count; j++)
        type->types[j] = read_uint8(p);
      read_leb_uint32(p, p_end, result_count);
      for (j = 0; j < result_count; j++)
        type->types[param_count + j] = read_uint8(p);
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    printf("Load type section failed: invalid section size.\n");
    return false;
  }

  *p_buf = p;
  printf("Load type section success.\n");
  return true;
}

static bool
load_import_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
find_code_section(const uint8 *buf, const uint8 **p_buf_code)
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
load_table_segment_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  /* TODO */
  return false;
}

static bool
load_data_segment_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
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
        /* unsupported user section, ignore it. */
        break;
      case SECTION_TYPE_TYPE:
        if (!load_type_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_IMPORT:
        if (!load_import_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_FUNC:
        if (!load_function_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_TABLE:
        if (!load_table_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_MEMORY:
        if (!load_memory_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_GLOBAL:
        if (!load_global_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_EXPORT:
        if (!load_export_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_START:
        if (!load_start_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_ELEM:
        if (!load_table_segment_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_CODE:
        if (!load_code_section(&p, buf_end, module))
          return false;
        break;
      case SECTION_TYPE_DATA:
        if (!load_data_segment_section(&p, buf_end, module))
          return false;
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

  if (!(module->const_str_set = bh_hash_map_create(32, false,
          (HashFunc)wasm_string_hash,
          (KeyEqualFunc)wasm_string_equal,
          NULL,
          bh_free)))
    goto fail;

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
  uint32 i;

  if (!module)
    return true;

  /* TODO: destroy the resource */

  if (module->types) {
    for (i = 0; i < module->type_count; i++)
      bh_free(module->types[i]);
    bh_free(module->types);
  }

  if (module->const_str_set)
    bh_hash_map_destroy(module->const_str_set);

  bh_free(module);
  return true;
}
