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
#include "wasm-native.h"
#include "wasm-opcode.h"
#include "bh_memory.h"

/* Read a value of given type from the address pointed to by the given
   pointer and increase the pointer to the position just after the
   value being read.  */
#define TEMPLATE_READ_VALUE(Type, p)                    \
    (p += sizeof(Type), *(Type *)(p - sizeof(Type)))

#define read_uint8(p)  TEMPLATE_READ_VALUE(uint8, p)
#define read_uint32(p) TEMPLATE_READ_VALUE(uint32, p)
#define read_bool(p)   TEMPLATE_READ_VALUE(bool, p)

#define CHECK_BUF(buf, buf_end, length) do {                    \
  if (buf + length > buf_end) {                                 \
    set_error_buf(error_buf, error_buf_size,                    \
                  "WASM module load failed: "                   \
                  "invalid file length.");                      \
    return false;                                               \
  }                                                             \
} while (0)

#define read_leb_uint64(p, p_end, res) do {         \
  uint32 off = 0;                                   \
  uint64 res64;                                     \
  if (!read_leb(p, p_end, &off, 64, false, &res64)) \
    return false;                                   \
  p += off;                                         \
  res = (uint64)res64;                              \
} while (0)

#define read_leb_int64(p, p_end, res) do {          \
  uint32 off = 0;                                   \
  uint64 res64;                                     \
  if (!read_leb(p, p_end, &off, 64, true, &res64))  \
    return false;                                   \
  p += off;                                         \
  res = (int64)res64;                               \
} while (0)

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

static void
set_error_buf(char *error_buf, uint32 error_buf_size, const char *string)
{
  if (error_buf != NULL)
    snprintf(error_buf, error_buf_size, "%s", string);
}

static char*
const_str_set_insert(const uint8 *str, int32 len, WASMModule *module,
                     char* error_buf, uint32 error_buf_size)
{
  HashMap *set = module->const_str_set;
  char *c_str = bh_malloc(len + 1), *value;

  if (!c_str) {
    set_error_buf(error_buf, error_buf_size,
                  "WASM module load failed: "
                  "alloc memory failed.");
    return NULL;
  }

  memcpy(c_str, str, len);
  c_str[len] = '\0';

  if ((value = bh_hash_map_find(set, c_str))) {
    bh_free(c_str);
    return value;
  }

  if (!bh_hash_map_insert(set, c_str, c_str)) {
    set_error_buf(error_buf, error_buf_size,
                  "WASM module load failed: "
                  "insert string to hash map failed.");
    bh_free(c_str);
    return NULL;
  }

  return c_str;
}

static bool
load_init_expr(const uint8 **p_buf, const uint8 *buf_end, InitializerExpression *init_expr,
               char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint8 flag, end_byte, *p_float;
  uint32 i;

  CHECK_BUF(p, p_end, 1);
  init_expr->init_expr_type = read_uint8(p);
  flag = init_expr->init_expr_type;

  switch (flag) {
    /* i32.const */
    case INIT_EXPR_TYPE_I32_CONST:
      read_leb_int32(p, p_end, init_expr->u.i32);
      break;
    /* i64.const */
    case INIT_EXPR_TYPE_I64_CONST:
      read_leb_int64(p, p_end, init_expr->u.i64);
      break;
    /* f32.const */
    case INIT_EXPR_TYPE_F32_CONST:
      CHECK_BUF(p, p_end, 4);
      p_float = (uint8*)&init_expr->u.f32;
      for (i = 0; i < sizeof(float32); i++)
        *p_float++ = *p++;
      break;
    /* f64.const */
    case INIT_EXPR_TYPE_F64_CONST:
      CHECK_BUF(p, p_end, 8);
      p_float = (uint8*)&init_expr->u.f64;
      for (i = 0; i < sizeof(float64); i++)
        *p_float++ = *p++;
      break;
    /* get_global */
    case INIT_EXPR_TYPE_GET_GLOBAL:
      read_leb_uint32(p, p_end, init_expr->u.global_index);
      break;
    default:
      set_error_buf(error_buf, error_buf_size, "type mismatch");
      return false;
  }
  CHECK_BUF(p, p_end, 1);
  end_byte = read_uint8(p);
  if (end_byte != 0x0b) {
      set_error_buf(error_buf, error_buf_size, "unexpected end");
      return false;
  }
  *p_buf = p;

  return true;
}

static bool
load_type_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module,
                  char *error_buf, uint32 error_buf_size)
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
      set_error_buf(error_buf, error_buf_size,
                    "Load type section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->types, 0, sizeof(WASMType*) * type_count);

    for (i = 0; i < type_count; i++) {
      CHECK_BUF(p, p_end, 1);
      flag = read_uint8(p);
      if (flag != 0x60) {
        set_error_buf(error_buf, error_buf_size,
                      "Load type section failed: "
                      "invalid type flag.");
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
      for (j = 0; j < param_count; j++) {
        CHECK_BUF(p, p_end, 1);
        type->types[j] = read_uint8(p);
      }
      read_leb_uint32(p, p_end, result_count);
      for (j = 0; j < result_count; j++) {
        CHECK_BUF(p, p_end, 1);
        type->types[param_count + j] = read_uint8(p);
      }
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load type section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load type section success.\n"); */
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
load_import_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
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
      set_error_buf(error_buf, error_buf_size,
                    "Load import section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->imports, 0, sizeof(WASMImport) * import_count);

    import = module->imports;
    for (i = 0; i < import_count; i++, import++) {
      /* load module name */
      read_leb_uint32(p, p_end, name_len);
      CHECK_BUF(p, p_end, name_len);
      if (!(import->module_name = const_str_set_insert(p, name_len, module,
                                                       error_buf, error_buf_size))) {
        return false;
      }
      p += name_len;

      /* load field name */
      read_leb_uint32(p, p_end, name_len);
      CHECK_BUF(p, p_end, name_len);
      if (!(import->field_name = const_str_set_insert(p, name_len, module,
                                                      error_buf, error_buf_size))) {
        return false;
      }
      p += name_len;

      read_leb_uint8(p, p_end, import->kind);
      switch (import->kind) {
        case IMPORT_KIND_FUNC: /* import function */
          read_leb_uint32(p, p_end, type_index);
          if (type_index >= module->type_count) {
            set_error_buf(error_buf, error_buf_size,
                          "Load import section failed: "
                          "invalid function type index.");
            return false;
          }
          import->u.function.func_type = module->types[type_index];
          module->import_function_count++;

          if (!(import->u.function.func_ptr_linked = wasm_native_func_lookup
                (import->module_name, import->field_name))) {
            set_error_buf(error_buf, error_buf_size,
                          "Load import section failed: "
                          "resolve import function failed.");
            return false;
          }
          break;

        case IMPORT_KIND_TABLE: /* import table */
          if (!load_table(&p, p_end, &import->u.table))
            return false;
          module->import_table_count++;
          if (module->import_table_count > 1) {
            set_error_buf(error_buf, error_buf_size, "multiple memories");
            return false;
          }
          break;

        case IMPORT_KIND_MEMORY: /* import memory */
          if (!load_memory(&p, p_end, &import->u.memory))
            return false;
          module->import_memory_count++;
          if (module->import_table_count > 1) {
            set_error_buf(error_buf, error_buf_size, "multiple memories");
            return false;
          }
          break;

        case IMPORT_KIND_GLOBAL: /* import global */
          read_leb_uint8(p, p_end, import->u.global.type);
          read_leb_uint8(p, p_end, mutable);
          import->u.global.is_mutable = mutable & 1 ? true : false;
          module->import_global_count++;
          if (!(wasm_native_global_lookup(import->module_name,
                                          import->field_name,
                                          &import->u.global))) {
            set_error_buf(error_buf, error_buf_size,
                          "Load import section failed: "
                          "resolve import global failed.");
            return false;
          }
          break;

        default:
          set_error_buf(error_buf, error_buf_size,
                        "Load import section failed: "
                        "invalid import type.");
          return false;
      }
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load import section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load import section success.\n"); */
  return true;
}

static bool
find_code_section(const uint8 *buf, const uint8 *buf_end, const uint8 **p_buf_code,
                  char *error_buf, uint32 error_buf_size)
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
      set_error_buf(error_buf, error_buf_size, "invalid section type");
      return false;
    }
  }

  set_error_buf(error_buf, error_buf_size,
                "WASM module load failed: "
                "find code section failed.");
  return false;
}

static bool
load_function_section(const uint8 **p_buf, const uint8 *buf_end,
                      const uint8 *buf_code, WASMModule *module,
                      char *error_buf, uint32 error_buf_size)
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
    set_error_buf(error_buf, error_buf_size,
                  "Load function section failed: "
                  "invalid function count.");
    return false;
  }

  if (func_count) {
    module->function_count = func_count;
    if (!(module->functions = bh_malloc(sizeof(WASMFunction*) * func_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load function section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->functions, 0, sizeof(WASMFunction*) * func_count);

    for (i = 0; i < func_count; i++) {
      /* Resolve function type */
      read_leb_uint32(p, p_end, type_index);
      if (type_index >= module->type_count) {
        set_error_buf(error_buf, error_buf_size,
                      "Load function section failed: "
                      "invalid function type index.");
        return false;
      }

      read_leb_uint32(p_code, p_end, code_size);
      if (code_size == 0) {
        set_error_buf(error_buf, error_buf_size,
                      "Load function section failed: "
                      "invalid function code size.");
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
        set_error_buf(error_buf, error_buf_size,
                      "Load function section failed: "
                      "alloc memory failed.");
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
    set_error_buf(error_buf, error_buf_size,
                  "Load function section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load function section success.\n"); */
  return true;
}

static bool
load_table_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, table_count, i;
  WASMTable *table;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, table_count);
  bh_assert(table_count == 1);

  if (table_count) {
    if (table_count > 1) {
      set_error_buf(error_buf, error_buf_size, "multiple memories");
      return false;
    }
    module->table_count = table_count;
    if (!(module->tables = bh_malloc(sizeof(WASMTable) * table_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load table section failed: "
                    "alloc memory failed.");
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
    set_error_buf(error_buf, error_buf_size,
                  "Load table section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load table section success.\n"); */
  return true;
}

static bool
load_memory_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, memory_count, i;
  WASMMemory *memory;

  read_leb_uint32(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, memory_count);
  bh_assert(memory_count == 1);

  if (memory_count) {
    if (memory_count > 1) {
      set_error_buf(error_buf, error_buf_size, "multiple memories");
      return false;
    }
    module->memory_count = memory_count;
    if (!(module->memories = bh_malloc(sizeof(WASMMemory) * memory_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load memory section failed: "
                    "alloc memory failed.");
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
    set_error_buf(error_buf, error_buf_size,
                  "Load memory section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load memory section success.\n"); */
  return true;
}

static bool
load_global_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, global_count, i;
  WASMGlobal *global;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, global_count);

  if (global_count) {
    module->global_count = global_count;
    if (!(module->globals = bh_malloc(sizeof(WASMGlobal) * global_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load global section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->globals, 0, sizeof(WASMGlobal) * global_count);

    global = module->globals;

    for(i = 0; i < global_count; i++, global++) {
      CHECK_BUF(p, p_end, 1);
      global->type = read_uint8(p);
      CHECK_BUF(p, p_end, 1);
      global->is_mutable = read_bool(p);

      /* initialize expression */
      if (!load_init_expr(&p, p_end, &(global->init_expr), error_buf, error_buf_size))
        return false;
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load global section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load global section success.\n"); */
  return true;
}

static bool
load_export_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module,
                    char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, export_count, i, index;
  uint8 str_len;
  WASMExport *export;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, export_count);

  if (export_count) {
    module->export_count = export_count;
    if (!(module->exports = bh_malloc(sizeof(WASMExport) * export_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load export section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->exports, 0, sizeof(WASMExport) * export_count);

    export = module->exports;
    for (i = 0; i < export_count; i++, export++) {
      read_leb_uint32(p, p_end, str_len);
      CHECK_BUF(p, p_end, str_len);
      if (!(export->name = const_str_set_insert(p, str_len, module,
                                                error_buf, error_buf_size))) {
        return false;
      }
      p += str_len;
      CHECK_BUF(p, p_end, 1);
      export->kind = read_uint8(p);
      read_leb_uint32(p, p_end, index);
      export->index = index;

      switch(export->kind) {
        /*function index*/
        case EXPORT_KIND_FUNC:
          if (index >= module->function_count + module->import_function_count) {
            set_error_buf(error_buf, error_buf_size,
                          "Load export section failed: "
                          "function index is out of range.");
            return false;
          }
          break;
        /*table index*/
        case EXPORT_KIND_TABLE:
          if (index >= module->table_count + module->import_table_count) {
            set_error_buf(error_buf, error_buf_size,
                          "Load export section failed: "
                          "table index is out of range.");
            return false;
            }
          break;
        /*memory index*/
        case EXPORT_KIND_MEMORY:
          if (index >= module->memory_count + module->import_memory_count) {
            set_error_buf(error_buf, error_buf_size,
                          "Load export section failed: "
                          "memory index is out of range.");
            return false;
            }
          break;
        /*global index*/
        case EXPORT_KIND_GLOBAL:
          if (index >= module->global_count + module->import_global_count) {
            set_error_buf(error_buf, error_buf_size,
                          "Load export section failed: "
                          "global index is out of range.");
            return false;
            }
          break;
        default:
          set_error_buf(error_buf, error_buf_size,
                        "Load export section failed: "
                        "kind flag is unexpected.");
          return false;
      }
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load export section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load export section success.\n"); */
  return true;
}

static bool
load_table_segment_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, table_segment_count, i, j, table_index, function_count, function_index;
  WASMTableSeg *table_segment;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, table_segment_count);

  if (table_segment_count) {
    module->table_seg_count = table_segment_count;
    if (!(module->table_segments = bh_malloc(sizeof(WASMTableSeg) * table_segment_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load table segment section failed: "
                    "alloc memory failed.");
      return false;
    }

    memset(module->table_segments, 0, sizeof(WASMTableSeg) * table_segment_count);

    table_segment = module->table_segments;
    for (i = 0; i < table_segment_count; i++, table_segment++) {
      read_leb_uint32(p, p_end, table_index);
      table_segment->table_index = table_index;

      /* initialize expression */
      if (!load_init_expr(&p, p_end, &(table_segment->base_offset), error_buf, error_buf_size))
        return false;

      read_leb_uint32(p, p_end, function_count);
      table_segment->function_count = function_count;
      if (!(table_segment->func_indexes = (uint32 *)bh_malloc(sizeof(uint32) * function_count))) {
        set_error_buf(error_buf, error_buf_size,
                      "Load table segment section failed: "
                      "alloc memory failed.");
        return false;
      }
      for (j = 0; j < function_count; j++) {
        read_leb_uint32(p, p_end, function_index);
        table_segment->func_indexes[j] = function_index;
      }
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load table segment section failed, "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load table segment section success.\n"); */
  return true;
}

static bool
load_data_segment_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, data_seg_count, i, mem_index, data_seg_len;
  WASMDataSeg *dataseg;
  InitializerExpression init_expr;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, data_seg_count);

  if (data_seg_count) {
    module->data_seg_count = data_seg_count;
    if (!(module->data_segments = bh_malloc(sizeof(WASMDataSeg*) * data_seg_count))) {
      set_error_buf(error_buf, error_buf_size,
                    "Load data segment section failed, "
                    "alloc memory failed.");
      return false;
    }

    memset(module->data_segments, 0, sizeof(WASMDataSeg*) * data_seg_count);

    for (i = 0; i < data_seg_count; i++) {
      read_leb_uint32(p, p_end, mem_index);

      if (!load_init_expr(&p, p_end, &init_expr, error_buf, error_buf_size))
        return false;

      read_leb_uint32(p, p_end, data_seg_len);

      if (!(dataseg = module->data_segments[i] = bh_malloc(offsetof(WASMDataSeg, data) + data_seg_len))) {
        set_error_buf(error_buf, error_buf_size,
                      "Load data segment section failed: "
                      "alloc memory failed.");
        return false;
      }

      memcpy(&dataseg->base_offset, &init_expr, sizeof(init_expr));

      dataseg->memory_index = mem_index;
      dataseg->data_length = data_seg_len;
      CHECK_BUF(p, p_end, data_seg_len);
      memcpy(dataseg->data, p, data_seg_len);
      p += data_seg_len;
    }
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load data segment section failed, "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load data segment section success.\n"); */
  return true;
}

static bool
load_code_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module,
                  char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);

  /* code has been loaded in function section, so pass it here */
  /* TODO: should check if there really have section_size code bytes */
  p += section_size;
  *p_buf = p;
  /* printf("Load code segment section success.\n"); */

  return true;
}

static bool
load_start_section(const uint8 **p_buf, const uint8 *buf_end, WASMModule *module,
                   char *error_buf, uint32 error_buf_size)
{
  const uint8 *p = *p_buf, *p_end = buf_end;
  uint32 section_size, start_function;

  read_leb_uint32(p, p_end, section_size);
  CHECK_BUF(p, p_end, section_size);
  *p_buf = p;
  read_leb_uint32(p, p_end, start_function);

  if (start_function) {
    if (start_function >= module->function_count + module->import_function_count) {
      set_error_buf(error_buf, error_buf_size,
                    "Load start section failed: "
                    "function index is out of range.");
      return false;
    }
    module->start_function = start_function;
  }

  if (section_size != (uint32)(p - *p_buf)) {
    set_error_buf(error_buf, error_buf_size,
                  "Load start section failed: "
                  "invalid section size.");
    return false;
  }

  *p_buf = p;
  /* printf("Load start section success.\n"); */
  return true;
}

static bool
load(const uint8 *buf, uint32 size, WASMModule *module, char *error_buf, uint32 error_buf_size)
{
  const uint8 *buf_end = buf + size, *buf_code;
  const uint8 *p = buf, *p_end = buf_end;
  uint32 magic_number, version, section_size;

  CHECK_BUF(p, p_end, sizeof(uint32));
  if ((magic_number = read_uint32(p)) != WASM_MAGIC_NUMBER) {
    set_error_buf(error_buf, error_buf_size, "magic header not detected");
    return false;
  }

  CHECK_BUF(p, p_end, sizeof(uint32));
  if ((version = read_uint32(p)) != WASM_CURRENT_VERSION) {
    set_error_buf(error_buf, error_buf_size, "unknown binary version");
    return false;
  }

  while (p < p_end) {
    CHECK_BUF(p, p_end, 1);
    uint8 section_type = read_uint8(p);
    /* printf("section_type: %d\n", section_type); */

    switch (section_type) {
      case SECTION_TYPE_USER:
        /* unsupported user section, ignore it. */
        read_leb_uint32(p, p_end, section_size);
        p += section_size;
        break;
      case SECTION_TYPE_TYPE:
        if (!load_type_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_IMPORT:
        if (!load_import_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_FUNC:
        if (!find_code_section(buf, buf_end, &buf_code, error_buf, error_buf_size))
          return false;
        if (!load_function_section(&p, buf_end, buf_code, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_TABLE:
        if (!load_table_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_MEMORY:
        if (!load_memory_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_GLOBAL:
        if (!load_global_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_EXPORT:
        if (!load_export_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_START:
        if (!load_start_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_ELEM:
        if (!load_table_segment_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_CODE:
        if (!load_code_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      case SECTION_TYPE_DATA:
        if (!load_data_segment_section(&p, buf_end, module, error_buf, error_buf_size))
          return false;
        break;
      default:
        set_error_buf(error_buf, error_buf_size,
                      "WASM module load failed: "
                      "unknow section type.");
        return false;
    }
  }
  /* printf("Load module success.\n"); */
  return true;
}

WASMModule*
wasm_loader_load(const uint8 *buf, uint32 size, char *error_buf, uint32 error_buf_size)
{
  WASMModule *module = bh_malloc(sizeof(WASMModule));

  if (!module) {
    set_error_buf(error_buf, error_buf_size,
                  "WASM module load failed: "
                  "alloc memory failed.");
    return NULL;
  }

  memset(module, 0, sizeof(WASMModule));

  if (!(module->const_str_set = bh_hash_map_create(32, false,
          (HashFunc)wasm_string_hash,
          (KeyEqualFunc)wasm_string_equal,
          NULL,
          bh_free)))
    goto fail;

  if (!load(buf, size, module, error_buf, error_buf_size))
    goto fail;

  return module;

fail:
  wasm_loader_unload(module);
  return NULL;
}

void
wasm_loader_unload(WASMModule *module)
{
  uint32 i;

  if (!module)
    return;

  if (module->types) {
    for (i = 0; i < module->type_count; i++) {
      if (module->types[i])
        bh_free(module->types[i]);
    }
    bh_free(module->types);
  }

  if (module->imports)
    bh_free(module->imports);

  if (module->functions) {
    for (i = 0; i < module->function_count; i++) {
      if (module->functions[i])
        bh_free(module->functions[i]);
    }
    bh_free(module->functions);
  }

  if (module->tables)
    bh_free(module->tables);

  if (module->memories)
    bh_free(module->memories);

  if (module->globals)
    bh_free(module->globals);

  if (module->exports)
    bh_free(module->exports);

  if (module->table_segments) {
    for (i = 0; i < module->table_seg_count; i++) {
      if (module->table_segments[i].func_indexes)
        bh_free(module->table_segments[i].func_indexes);
    }
    bh_free(module->table_segments);
  }

  if (module->data_segments) {
    for (i = 0; i < module->data_seg_count; i++) {
      if (module->data_segments[i])
        bh_free(module->data_segments[i]);
    }
    bh_free(module->data_segments);
  }

  if (module->const_str_set)
    bh_hash_map_destroy(module->const_str_set);

  bh_free(module);
}

typedef struct block_addr {
  uint8 block_type;
  uint8 *else_addr;
  uint8 *end_addr;
} block_addr;

bool
wasm_loader_find_block_addr(HashMap *branch_set,
                            const uint8 *start_addr,
                            const uint8 *code_end_addr,
                            uint8 block_type,
                            uint8 **p_else_addr,
                            uint8 **p_end_addr)
{
  const uint8 *p = start_addr, *p_end = code_end_addr;
  uint8 *else_addr = NULL;
  uint32 block_nested_depth = 1, count, i, u32, u64;
  uint8 opcode, u8;
  block_addr *block;

  if ((block = bh_hash_map_find(branch_set, (void*)start_addr))) {
    if (block->block_type != block_type)
      return false;
    if (block_type == BLOCK_TYPE_IF) /* if block */
      *p_else_addr = block->else_addr;
    *p_end_addr = block->end_addr;
    return true;
  }

  while (p < code_end_addr) {
    opcode = *p++;

    switch (opcode) {
      case WASM_OP_UNREACHABLE:
      case WASM_OP_NOP:
        break;

      case WASM_OP_BLOCK:
      case WASM_OP_LOOP:
      case WASM_OP_IF:
        read_leb_uint32(p, p_end, u32); /* blocktype */
        block_nested_depth++;
        break;

      case WASM_OP_ELSE:
        if (block_type == BLOCK_TYPE_IF && block_nested_depth == 1)
          else_addr = (uint8*)(p - 1);
        break;

      case WASM_OP_END:
        if (block_nested_depth == 1) {
          if (block_type == BLOCK_TYPE_IF)
            *p_else_addr = else_addr;
          *p_end_addr = (uint8*)(p - 1);

          if ((block = bh_malloc(sizeof(block_addr)))) {
            block->block_type = block_type;
            if (block_type == BLOCK_TYPE_IF)
              block->else_addr = else_addr;
            block->end_addr = (uint8*)(p - 1);

            if (!bh_hash_map_insert(branch_set, (void*)start_addr, block))
              bh_free(block);
          }

          return true;
        }
        else
          block_nested_depth--;
        break;

      case WASM_OP_BR:
      case WASM_OP_BR_IF:
        read_leb_uint32(p, p_end, u32); /* labelidx */
        break;

      case WASM_OP_BR_TABLE:
        read_leb_uint32(p, p_end, count); /* lable num */
        for (i = 0; i <= count; i++) /* lableidxs */
          read_leb_uint32(p, p_end, u32);
        break;

      case WASM_OP_RETURN:
        break;

      case WASM_OP_CALL:
        read_leb_uint32(p, p_end, u32); /* funcidx */
        break;

      case WASM_OP_CALL_INDIRECT:
        read_leb_uint32(p, p_end, u32); /* typeidx */
        read_leb_uint8(p, p_end, u8); /* 0x00 */
        break;

      case WASM_OP_DROP:
      case WASM_OP_SELECT:
        break;

      case WASM_OP_GET_LOCAL:
      case WASM_OP_SET_LOCAL:
      case WASM_OP_TEE_LOCAL:
      case WASM_OP_GET_GLOBAL:
      case WASM_OP_SET_GLOBAL:
        read_leb_uint32(p, p_end, u32); /* localidx */
        break;

      case WASM_OP_I32_LOAD:
      case WASM_OP_I64_LOAD:
      case WASM_OP_F32_LOAD:
      case WASM_OP_F64_LOAD:
      case WASM_OP_I32_LOAD8_S:
      case WASM_OP_I32_LOAD8_U:
      case WASM_OP_I32_LOAD16_S:
      case WASM_OP_I32_LOAD16_U:
      case WASM_OP_I64_LOAD8_S:
      case WASM_OP_I64_LOAD8_U:
      case WASM_OP_I64_LOAD16_S:
      case WASM_OP_I64_LOAD16_U:
      case WASM_OP_I64_LOAD32_S:
      case WASM_OP_I64_LOAD32_U:
      case WASM_OP_I32_STORE:
      case WASM_OP_I64_STORE:
      case WASM_OP_F32_STORE:
      case WASM_OP_F64_STORE:
      case WASM_OP_I32_STORE8:
      case WASM_OP_I32_STORE16:
      case WASM_OP_I64_STORE8:
      case WASM_OP_I64_STORE16:
      case WASM_OP_I64_STORE32:
        read_leb_uint32(p, p_end, u32); /* align */
        read_leb_uint32(p, p_end, u32); /* offset */
        break;

      case WASM_OP_MEMORY_SIZE:
      case WASM_OP_MEMORY_GROW:
        read_leb_uint32(p, p_end, u32); /* 0x00 */
        break;

      case WASM_OP_I32_CONST:
        read_leb_uint32(p, p_end, u32);
        break;
      case WASM_OP_I64_CONST:
        read_leb_uint64(p, p_end, u64);
        break;
      case WASM_OP_F32_CONST:
        p += sizeof(float32);
        break;
      case WASM_OP_F64_CONST:
        p += sizeof(float64);
        break;

      case WASM_OP_I32_EQZ:
      case WASM_OP_I32_EQ:
      case WASM_OP_I32_NE:
      case WASM_OP_I32_LT_S:
      case WASM_OP_I32_LT_U:
      case WASM_OP_I32_GT_S:
      case WASM_OP_I32_GT_U:
      case WASM_OP_I32_LE_S:
      case WASM_OP_I32_LE_U:
      case WASM_OP_I32_GE_S:
      case WASM_OP_I32_GE_U:
      case WASM_OP_I64_EQZ:
      case WASM_OP_I64_EQ:
      case WASM_OP_I64_NE:
      case WASM_OP_I64_LT_S:
      case WASM_OP_I64_LT_U:
      case WASM_OP_I64_GT_S:
      case WASM_OP_I64_GT_U:
      case WASM_OP_I64_LE_S:
      case WASM_OP_I64_LE_U:
      case WASM_OP_I64_GE_S:
      case WASM_OP_I64_GE_U:
      case WASM_OP_F32_EQ:
      case WASM_OP_F32_NE:
      case WASM_OP_F32_LT:
      case WASM_OP_F32_GT:
      case WASM_OP_F32_LE:
      case WASM_OP_F32_GE:
      case WASM_OP_F64_EQ:
      case WASM_OP_F64_NE:
      case WASM_OP_F64_LT:
      case WASM_OP_F64_GT:
      case WASM_OP_F64_LE:
      case WASM_OP_F64_GE:
      case WASM_OP_I32_CLZ:
      case WASM_OP_I32_CTZ:
      case WASM_OP_I32_POPCNT:
      case WASM_OP_I32_ADD:
      case WASM_OP_I32_SUB:
      case WASM_OP_I32_MUL:
      case WASM_OP_I32_DIV_S:
      case WASM_OP_I32_DIV_U:
      case WASM_OP_I32_REM_S:
      case WASM_OP_I32_REM_U:
      case WASM_OP_I32_AND:
      case WASM_OP_I32_OR:
      case WASM_OP_I32_XOR:
      case WASM_OP_I32_SHL:
      case WASM_OP_I32_SHR_S:
      case WASM_OP_I32_SHR_U:
      case WASM_OP_I32_ROTL:
      case WASM_OP_I32_ROTR:
      case WASM_OP_I64_CLZ:
      case WASM_OP_I64_CTZ:
      case WASM_OP_I64_POPCNT:
      case WASM_OP_I64_ADD:
      case WASM_OP_I64_SUB:
      case WASM_OP_I64_MUL:
      case WASM_OP_I64_DIV_S:
      case WASM_OP_I64_DIV_U:
      case WASM_OP_I64_REM_S:
      case WASM_OP_I64_REM_U:
      case WASM_OP_I64_AND:
      case WASM_OP_I64_OR:
      case WASM_OP_I64_XOR:
      case WASM_OP_I64_SHL:
      case WASM_OP_I64_SHR_S:
      case WASM_OP_I64_SHR_U:
      case WASM_OP_I64_ROTL:
      case WASM_OP_I64_ROTR:
      case WASM_OP_F32_ABS:
      case WASM_OP_F32_NEG:
      case WASM_OP_F32_CEIL:
      case WASM_OP_F32_FLOOR:
      case WASM_OP_F32_TRUNC:
      case WASM_OP_F32_NEAREST:
      case WASM_OP_F32_SQRT:
      case WASM_OP_F32_ADD:
      case WASM_OP_F32_SUB:
      case WASM_OP_F32_MUL:
      case WASM_OP_F32_DIV:
      case WASM_OP_F32_MIN:
      case WASM_OP_F32_MAX:
      case WASM_OP_F32_COPYSIGN:
      case WASM_OP_F64_ABS:
      case WASM_OP_F64_NEG:
      case WASM_OP_F64_CEIL:
      case WASM_OP_F64_FLOOR:
      case WASM_OP_F64_TRUNC:
      case WASM_OP_F64_NEAREST:
      case WASM_OP_F64_SQRT:
      case WASM_OP_F64_ADD:
      case WASM_OP_F64_SUB:
      case WASM_OP_F64_MUL:
      case WASM_OP_F64_DIV:
      case WASM_OP_F64_MIN:
      case WASM_OP_F64_MAX:
      case WASM_OP_F64_COPYSIGN:
      case WASM_OP_I32_WRAP_I64:
      case WASM_OP_I32_TRUNC_S_F32:
      case WASM_OP_I32_TRUNC_U_F32:
      case WASM_OP_I32_TRUNC_S_F64:
      case WASM_OP_I32_TRUNC_U_F64:
      case WASM_OP_I64_EXTEND_S_I32:
      case WASM_OP_I64_EXTEND_U_I32:
      case WASM_OP_I64_TRUNC_S_F32:
      case WASM_OP_I64_TRUNC_U_F32:
      case WASM_OP_I64_TRUNC_S_F64:
      case WASM_OP_I64_TRUNC_U_F64:
      case WASM_OP_F32_CONVERT_S_I32:
      case WASM_OP_F32_CONVERT_U_I32:
      case WASM_OP_F32_CONVERT_S_I64:
      case WASM_OP_F32_CONVERT_U_I64:
      case WASM_OP_F32_DEMOTE_F64:
      case WASM_OP_F64_CONVERT_S_I32:
      case WASM_OP_F64_CONVERT_U_I32:
      case WASM_OP_F64_CONVERT_S_I64:
      case WASM_OP_F64_CONVERT_U_I64:
      case WASM_OP_F64_PROMOTE_F32:
      case WASM_OP_I32_REINTERPRET_F32:
      case WASM_OP_I64_REINTERPRET_F64:
      case WASM_OP_F32_REINTERPRET_I32:
      case WASM_OP_F64_REINTERPRET_I64:
        break;

      default:
        printf("WASM loader find block addr failed: invalid opcode %02x.\n",
               opcode);
        break;
    }
  }

  (void)u32;
  (void)u64;
  (void)u8;
  return false;
}
