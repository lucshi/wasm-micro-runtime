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
#include "wasm-runtime.h"
#include "wasm-thread.h"


/* TODO: resolve start function and execute it, check whether we
         need to call it firstly when running main function. */

static WASMFunctionInstance*
resolve_main_function(const WASMModuleInstance *module_inst)
{
  uint32 i;
  for (i = 0; i < module_inst->export_func_count; i++)
    if (!strcmp(module_inst->export_functions->name, "main"))
      return module_inst->export_functions[i].function;
  return NULL;
}

static bool
check_main_func_type(const WASMType *type)
{
  if (!(type->param_count == 0 || type->param_count == 2)
      ||type->result_count > 1)
    return false;

  if (type->param_count == 2
      && !(type->types[0] == VALUE_TYPE_I32
           && type->types[1] == VALUE_TYPE_I32))
    return false;

  if (type->result_count &&
      type->types[type->param_count] != VALUE_TYPE_I32)
    return false;

  return true;
}

bool
wasm_application_execute_main(int argc, char *argv[])
{
  WASMThread *self = wasm_runtime_get_self();
  WASMModuleInstance *module_inst = self->vm_instance->module;
  WASMFunctionInstance *func = resolve_main_function(module_inst);
  uint32 argc1 = 0, argv1[2];

  if (!func || func->is_import_func)
    return false;

  if (!check_main_func_type(func->u.func->func_type))
    return false;

  if (func->u.func->func_type->param_count) {
    argv1[0] = argc;
    argv1[1] = (uint32)argv;
  }

  wasm_runtime_call_wasm(func, argc1, argv1);
  /* TODO: check exception */
  return true;
}
