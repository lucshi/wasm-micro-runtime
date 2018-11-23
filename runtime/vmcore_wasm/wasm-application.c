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

#include <errno.h>
#ifdef WASM_ENABLE_REPL
#include <ieee754.h>
#include <math.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "wasm.h"
#include "wasm-interp.h"
#include "wasm-runtime.h"
#include "wasm-thread.h"
#include "bh_assert.h"
#include "bh_log.h"
#include "bh_memory.h"
#include "bh_platform_log.h"


bool
wasm_application_execute_start(void)
{
  WASMThread *self = wasm_runtime_get_self();
  WASMFunctionInstance *func = self->vm_instance->module->start_function;

  if (!func)
    return true;

  bh_assert(!func->is_import_func && func->param_cell_num == 0
            && func->ret_cell_num == 0);

  wasm_runtime_call_wasm(func, 0, NULL);

  return !wasm_runtime_get_exception() ? true : false;
}

static WASMFunctionInstance*
resolve_post_instantiate_function(const WASMModuleInstance *module_inst)
{
  uint32 i;
  for (i = 0; i < module_inst->export_func_count; i++)
    if (!strcmp(module_inst->export_functions[i].name, "__post_instantiate"))
      return module_inst->export_functions[i].function;
  return NULL;
}

static bool
check_post_instantiate_func_type(const WASMType *type)
{
  if (!(type->param_count == 0 && type->result_count == 0)) {
    LOG_ERROR("WASM execute application failed: "
              "invalid __post_instantiate function type.\n");
    return false;
  }
  return true;
}

static WASMFunctionInstance*
resolve_main_function(const WASMModuleInstance *module_inst)
{
  uint32 i;
  for (i = 0; i < module_inst->export_func_count; i++)
    if (!strcmp(module_inst->export_functions[i].name, "_main")
        || !strcmp(module_inst->export_functions[i].name, "main"))
      return module_inst->export_functions[i].function;

  LOG_ERROR("WASM execute application failed: main function not found.\n");
  return NULL;
}

static bool
check_main_func_type(const WASMType *type)
{
  if (!(type->param_count == 0 || type->param_count == 2)
      ||type->result_count > 1) {
    LOG_ERROR("WASM execute application failed: invalid main function type.\n");
    return false;
  }

  if (type->param_count == 2
      && !(type->types[0] == VALUE_TYPE_I32
           && type->types[1] == VALUE_TYPE_I32)) {
    LOG_ERROR("WASM execute application failed: invalid main function type.\n");
    return false;
  }

  if (type->result_count &&
      type->types[type->param_count] != VALUE_TYPE_I32) {
    LOG_ERROR("WASM execute application failed: invalid main function type.\n");
    return false;
  }

  return true;
}

bool
wasm_application_execute_main(int argc, char *argv[])
{
  WASMThread *self = wasm_runtime_get_self();
  WASMModuleInstance *module_inst = self->vm_instance->module;
  WASMFunctionInstance *func_post_instantiate =
    resolve_post_instantiate_function(module_inst);
  WASMFunctionInstance *func = resolve_main_function(module_inst);
  uint32 argc1 = 0, argv1[3] = { 0 };

  if (func_post_instantiate &&
      !check_post_instantiate_func_type(func_post_instantiate->u.func->func_type))
    return false;

  if (!func || func->is_import_func)
    return false;

  if (!check_main_func_type(func->u.func->func_type))
    return false;

  if (func->u.func->func_type->param_count) {
    argc1 = 2;
    argv1[0] = argc;
#ifdef __i386__
    argv1[1] = (uint32)argv;
#elif __x86_64__
    memcpy(argv1 + 1, &argv, 8);
#endif
  }

  if (func_post_instantiate) {
    wasm_runtime_call_wasm(func_post_instantiate, 0, NULL);
    if (wasm_runtime_get_exception())
      return false;
  }

  wasm_runtime_call_wasm(func, argc1, argv1);

  return !wasm_runtime_get_exception() ? true : false;
}

#ifdef WASM_ENABLE_REPL
static WASMFunctionInstance*
resolve_function(const WASMModuleInstance *module_inst, char *name)
{
  uint32 i;
  for (i = 0; i < module_inst->export_func_count; i++)
    if (!strcmp(module_inst->export_functions[i].name, name))
      return module_inst->export_functions[i].function;
  return NULL;
}

bool
wasm_application_execute_func(int argc, char *argv[])
{
  WASMThread *self = wasm_runtime_get_self();
  WASMModuleInstance *module_inst = self->vm_instance->module;
  WASMFunctionInstance *func;
  WASMType *type;
  uint32 argc1, *argv1;
  int32 i, p;
  const char *exception;

  bh_assert(argc >= 1);
  func = resolve_function(module_inst, argv[0]);
  if (!func || func->is_import_func)
    return false;

  type = func->u.func->func_type;
  if (type->param_count != (uint32)(argc - 1))
    return false;

  argc1 = func->param_cell_num;
  argv1 = bh_malloc(sizeof(uint32) * (argc1 > 2 ? argc1 : 2));
  if (argv1 == NULL) {
    LOG_ERROR("Wasm prepare param failed: malloc failed.\n");
    return false;
  }

  /* Parse arguments */
  for (i = 1, p = 0; i < argc; i++) {
    char *endptr;
    bh_assert(argv[i] != NULL);
    if (argv[i][0] == '\0') {
      LOG_ERROR("Wasm prepare param failed: invalid num (%s).\n", argv[i]);
      goto fail;
    }
    switch (type->types[i - 1]) {
      case VALUE_TYPE_I32:
        argv1[p++] = strtoul(argv[i], &endptr, 0);
        break;
      case VALUE_TYPE_I64:
        {
          union { uint64 val; uint32 parts[2]; } u;
          u.val = strtoull(argv[i], &endptr, 0);
          argv1[p++] = u.parts[0];
          argv1[p++] = u.parts[1];
          break;
        }
      case VALUE_TYPE_F32:
        {
          float32 f32 = strtof(argv[i], &endptr);
          if (isnan(f32)) {
            if (argv[i][0] == '-') {
              f32 = -f32;
            }
            if (endptr[0] == ':') {
              uint32 sig;
              union ieee754_float u;
              sig = strtoul(endptr + 1, &endptr, 0);
              u.f = f32;
              u.ieee.mantissa = sig;
              f32 = u.f;
            }
          }
          *(float32*)&argv1[p++] = f32;
          break;
        }
      case VALUE_TYPE_F64:
        {
          union { float64 val; uint32 parts[2]; } u;
          u.val = strtod(argv[i], &endptr);
          if (isnan(u.val)) {
            if (argv[i][0] == '-') {
              u.val = -u.val;
            }
            if (endptr[0] == ':') {
              uint64 sig;
              union ieee754_double ud;
              sig = strtoull(endptr + 1, &endptr, 0);
              ud.d = u.val;
              ud.ieee.mantissa0 = sig >> 32;
              ud.ieee.mantissa1 = sig;
              u.val = ud.d;
            }
          }
          argv1[p++] = u.parts[0];
          argv1[p++] = u.parts[1];
          break;
        }
    }
    if (*endptr != '\0' && *endptr != '_') {
      LOG_ERROR("Wasm prepare param failed: invalid num (%s).\n", argv[i]);
      goto fail;
    }
    if (errno != 0) {
      LOG_ERROR("Wasm prepare param failed: errno %d.\n", errno);
      goto fail;
    }
  }
  bh_assert(p == (int32)argc1);

  wasm_runtime_set_exception(NULL);
  wasm_runtime_call_wasm(func, argc1, argv1);
  exception = wasm_runtime_get_exception();
  if (exception) {
    bh_printf("%s\n", exception);
    goto fail;
  }
  /* print return value */
  switch (type->types[type->param_count]) {
    case VALUE_TYPE_I32:
      bh_printf("0x%x:i32", argv1[0]);
      break;
    case VALUE_TYPE_I64:
      {
        union { uint64 val; uint32 parts[2]; } u;
        u.parts[0] = argv1[0];
        u.parts[1] = argv1[1];
        bh_printf("0x%llx:i64", u.val);
        break;
      }
    case VALUE_TYPE_F32:
      bh_printf("%.7g:f32", *(float32*)argv1);
      break;
    case VALUE_TYPE_F64:
      {
        union { float64 val; uint32 parts[2]; } u;
        u.parts[0] = argv1[0];
        u.parts[1] = argv1[1];
        bh_printf("%.7g:f64", u.val);
        break;
      }
  }
  bh_printf("\n");

  bh_free(argv1);
  return true;

fail:
  bh_free(argv1);
  return false;
}
#endif /* WASM_ENABLE_REPL */
