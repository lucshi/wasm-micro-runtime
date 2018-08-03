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
      printf("WASM module load failed: unsigned LEB at byte %d overflow\n",
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

static char*
const_str_set_insert(const uint8 *str, int32 len, WASMModule *module)
{
  HashMap *set = module->const_str_set;
  char *c_str = bh_malloc(len + 1), *value;

  if (!c_str) {
    printf("WASM module load failed: alloc memory failed.\n");
    return NULL;
  }

  memcpy(c_str, str, len);
  c_str[len] = '\0';

  if ((value = bh_hash_map_find(set, c_str))) {
    bh_free(c_str);
    return value;
  }

  if (!bh_hash_map_insert(set, c_str, c_str)) {
    printf("WASM module load failed: insert string to hash map failed.\n");
    bh_free(c_str);
    return NULL;
  }

  return c_str;
}

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

      /* Resolve param count and result count firstly */
      p_org = p;
      CHECK_BUF(p, p_end, param_count);
      p += param_count;
      read_leb_uint32(p, p_end, result_count);
      bh_assert(result_count <= 1);
      CHECK_BUF(p, p_end, result_count);
      p = p_org;

      if (!(type = module->types[i] = bh_malloc(offsetof(WASMType, types) +
              sizeof(uint8) * (param_count + result_count))))
        return false;

      /* Resolve param types and result types */
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
load_table(const uint8 **p_buf, const uint8 *buf_end, WASMTable *table)
{
  const uint8 *p = *p_buf, *p_end = buf_end;

  read_leb_uint8(p, p_end, table->elem_type);
  bh_assert(table->elem_type == TABLE_ELEM_TYPE_ANY_FUNC);
  read_leb_uint32(p, p_end, table->flags);
  read_leb_uint32(p, p_end, table->init_size);
  if (table->flags & 1)
    read_leb_uint32(p, p_end, table->max_size);
  else
    table->max_size = 0x10000;

  *p_buf = p;
  return true;
}

static bool
load_memory(const uint8 **p_buf, const uint8 *buf_end, WASMMemory *memory)
{
  const uint8 *p = *p_buf, *p_end = buf_end;

  read_leb_uint32(p, p_end, memory->flags);
  read_leb_uint32(p, p_end, memory->init_page_count);
  if (memory->flags & 1)
    read_leb_uint32(p, p_end, memory->max_page_count);
  else
    /* Limit the maximum memory size to 4GB */
    memory->max_page_count = 0x10000;

  *p_buf = p;
  return true;
}

static bool
load_import_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, import_count, name_len, type_index, i;
  WASMImport *import;
  uint8 mutable;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, import_count);

  if (import_count) {
    module->import_count = import_count;
    if (!(module->imports = bh_malloc(sizeof(WASMImport) * import_count))) {
      printf("Load import section failed: alloc memory failed.\n");
      return false;
    }

    memset(module->imports, 0, sizeof(WASMImport) * import_count);

    import = module->imports;
    for (i = 0; i < import_count; i++, import++) {
      /* load module name */
      read_leb_uint32(p, p_end, name_len);
      CHECK_BUF(p, p_end, name_len);
      if (!(import->module_name = const_str_set_insert(p, name_len, module))) {
        return false;
      }
      p += name_len;

      /* load field name */
      read_leb_uint32(p, p_end, name_len);
      CHECK_BUF(p, p_end, name_len);
      if (!(import->field_name = const_str_set_insert(p, name_len, module))) {
        return false;
      }
      p += name_len;

      read_leb_uint8(p, p_end, import->kind);
      switch (import->kind) {
        case IMPORT_KIND_FUNC: /* import function */
          read_leb_uint32(p, p_end, type_index);
          if (type_index >= module->type_count) {
            printf("Load import section failed: invalid function type index.\n");
            return false;
          }
          import->u.function.func_type = module->types[type_index];
          break;

        case IMPORT_KIND_TABLE: /* import table */
          if (!load_table(&p, p_end, &import->u.table))
            return false;
          break;

        case IMPORT_KIND_MEMORY: /* import memory */
          if (!load_memory(&p, p_end, &import->u.memory))
            return false;
          break;

        case IMPORT_KIND_GLOBAL: /* import global */
          read_leb_uint8(p, p_end, import->u.global.type);
          read_leb_uint8(p, p_end, mutable);
          import->u.global.is_mutable = mutable & 1 ? true : false;
          break;

        default:
          printf("Load import section failed: invalid import type.\n");
          return false;
      }
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    printf("Load import section failed: invalid section size.\n");
    return false;
  }

  *p_buf = p;
  printf("Load import section success.\n");
  return true;
}

static bool
find_code_section(const uint8 *buf, const uint8 *buf_end, const uint8 **p_buf_code)
{
  const uint8 *p = buf, *p_end = buf_end;
  uint8 section_type;
  uint32 section_size;

  p += 8;
  while (p < p_end) {
    CHECK_BUF(p, p_end, 1);
    section_type = read_uint8(p);
    if (section_type <= SECTION_TYPE_DATA) {
      read_leb_uint32(p, p_end, section_size);
      CHECK_BUF(p, p_end, section_size);
      if (section_type != SECTION_TYPE_CODE) {
        p += section_size;
      }
      else {
        *p_buf_code = p;
        return true;
      }
    }
    else {
      printf("WASM module load failed: invalid section type %d.\n",
             section_type);
      return false;
    }
  }

  printf("WASM module load failed: find code section failed.\n");
  return false;
}

static bool
load_function_section(const uint8 **p_buf, const uint8 *buf_end,
                      const uint8 *buf_code, WASMModule *module)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  const uint8 *p_code = buf_code, *p_code_end, *p_code_save;
  uint32 section_size, func_count, total_size;
  uint32 code_count, code_size, type_index, i, j, k, local_type_index;
  uint32 local_count, local_set_count, sub_local_count;
  uint8 type;
  WASMFunction *func;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, func_count);

  read_leb_uint32(p_code, p_end, code_count);
  if (func_count != code_count) {
    printf("Load function section failed: invalid function count.\n");
    return false;
  }

  if (func_count) {
    module->function_count = func_count;
    if (!(module->functions = bh_malloc(sizeof(WASMFunction*) * func_count))) {
      printf("Load function section failed: alloc memory failed.\n");
      return false;
    }

    memset(module->functions, 0, sizeof(WASMFunction*) * func_count);

    for (i = 0; i < func_count; i++) {
      /* Resolve function type */
      read_leb_uint32(p, p_end, type_index);
      if (type_index >= module->type_count) {
        printf("Load function section failed: invalid function type index.\n");
        return false;
      }

      read_leb_uint32(p_code, p_end, code_size);
      if (code_size == 0) {
        printf("Load function section failed: invalid function code size.\n");
        return false;
      }

      /* Resolve local set count */
      p_code_end = p_code + code_size;
      local_count = 0;
      read_leb_uint32(p_code, p_end, local_set_count);
      p_code_save = p_code;

      /* Calculate total local count */
      for (j = 0; j < local_set_count; j++) {
        read_leb_uint32(p_code, p_end, sub_local_count);
        read_leb_uint8(p_code, p_end, type);
        local_count += sub_local_count;
      }

      /* Alloc memory, layout: function structure + code + local types */
      code_size = p_code_end - p_code;
      total_size = offsetof(WASMFunction, code) + code_size + local_count;

      if (!(func = module->functions[i] = bh_malloc(total_size))) {
        printf("Load function section failed: alloc memory failed.\n");
        return false;
      }

      /* Set function type, local count, code size and code body */
      memset(func, 0, total_size);
      func->func_type = module->types[type_index];
      func->local_count = local_count;
      if (local_count > 0)
        func->local_types = (uint8*)func + offsetof(WASMFunction, code)
                            + code_size;
      func->code_size = code_size;
      memcpy(func->code, p_code, code_size);

      /* Load each local type */
      p_code = p_code_save;
      local_type_index = 0;
      for (j = 0; j < local_set_count; j++) {
        read_leb_uint32(p_code, p_end, sub_local_count);
        read_leb_uint8(p_code, p_end, type);
        for (k = 0; k < sub_local_count; k++) {
          func->local_types[local_type_index++] = type;
        }
      }
      p_code = p_code_end;
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    printf("Load function section failed: invalid section size.\n");
    return false;
  }

  *p_buf = p;
  printf("Load function section success.\n");
  return true;
}

static bool
load_table_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, table_count, i;
  WASMTable *table;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, table_count);
  bh_assert(table_count == 1);

  if (table_count) {
    module->table_count = table_count;
    if (!(module->tables = bh_malloc(sizeof(WASMTable) * table_count))) {
      printf("Load table section failed: alloc memory failed.\n");
      return false;
    }

    memset(module->tables, 0, sizeof(WASMTable) * table_count);

    /* load each table */
    table = module->tables;
    for (i = 0; i < table_count; i++, table++)
      if (!load_table(&p, p_end, table))
        return false;
  }

  if (section_size != (uint32)(p - *p_buf)) {
    printf("Load table section failed: invalid section size.\n");
    return false;
  }

  *p_buf = p;
  printf("Load table section success.\n");
  return false;
}

static bool
load_memory_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, memory_count, i;
  WASMMemory *memory;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, memory_count);
  bh_assert(memory_count == 1);

  if (memory_count) {
    module->memory_count = memory_count;
    if (!(module->memories = bh_malloc(sizeof(WASMMemory) * memory_count))) {
      printf("Load memory section failed: alloc memory failed.\n");
      return false;
    }

    memset(module->memories, 0, sizeof(WASMMemory) * memory_count);

    /* load each memory */
    memory = module->memories;
    for (i = 0; i < memory_count; i++, memory++)
      if (!load_memory(&p, p_end, memory))
        return false;
  }

  if (section_size != (uint32)(p - *p_buf)) {
    printf("Load memory section failed: invalid section size.\n");
    return false;
  }

  *p_buf = p;
  printf("Load memory section success.\n");
  return true;
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
  const uint8 *buf_end = buf + size, *buf_code;
  const uint8 *p = buf, *p_end = buf_end;
  uint32 magic_number, version, section_size;

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
        read_leb_uint32(p, p_end, section_size);
        p += section_size;
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
        if (!find_code_section(buf, buf_end, &buf_code))
          return false;
        if (!load_function_section(&p, buf_end, buf_code, module))
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
  return true;
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

  if (module->types) {
    for (i = 0; i < module->type_count; i++)
      bh_free(module->types[i]);
    bh_free(module->types);
  }

  if (module->imports)
    bh_free(module->imports);

  if (module->functions) {
    for (i = 0; i < module->function_count; i++)
      bh_free(module->functions[i]);
    bh_free(module->functions);
  }

  if (module->tables)
    bh_free(module->tables);

  if (module->memories)
    bh_free(module->memories);

  /* TODO: destroy the resource */

  if (module->const_str_set)
    bh_hash_map_destroy(module->const_str_set);

  bh_free(module);
  return true;
}
