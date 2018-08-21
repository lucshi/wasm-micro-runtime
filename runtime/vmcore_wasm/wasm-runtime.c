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

#include "wasm-runtime.h"
#include "wasm-thread.h"
#include "wasm-import.h"
#include "wasm-loader.h"
#include "wasm-native.h"
#include "wasm-interp.h"
#include "bh_memory.h"


/* The supervisor VM instance. */
static WASMVmInstance *supervisor_instance;

/* The mutex for protecting the VM instance list. */
static vmci_thread_mutex_t instance_list_lock;


static bool
wasm_runtime_create_supervisor_il_env()
{
  WASMVmInstance *ilr;

  /* Ensure this is a new thread that has not been initialized. */
  bh_assert(!wasm_runtime_get_self());

  if (!(ilr = wasm_thread_create_ilr(NULL,
                                     vmci_reserved_native_stack_size,
                                     vmci_reserved_wasm_stack_size,
                                     NULL, NULL, NULL)))
    return false;

  /* Set thread local root. */
  wasm_runtime_set_tlr(&ilr->main_tlr);
  /* The current instance is the supervisor instance. */
  supervisor_instance = ilr;
  /* Initialize the circular linked list of VM instances. */
  ilr->prev = ilr->next = ilr;
  return true;
}

bool
wasm_runtime_init()
{

  if (vmci_thread_sys_init() != 0)
    return false;

  wasm_runtime_set_tlr(NULL);

  if (vmci_thread_mutex_init(&instance_list_lock, false))
    goto fail1;

  if (!wasm_runtime_create_supervisor_il_env())
    goto fail2;

  wasm_native_init();
  return true;

fail2:
  vmci_thread_mutex_destroy(&instance_list_lock);

fail1:
  vmci_thread_sys_destroy();

  return false;
}

void
wasm_runtime_destroy()
{
  wasm_thread_destroy_ilr(supervisor_instance);
  supervisor_instance = NULL;

  vmci_thread_mutex_destroy(&instance_list_lock);

  wasm_runtime_set_tlr(NULL);
  vmci_thread_sys_destroy();
}

void
wasm_runtime_call_wasm(WASMFunctionInstance *function,
                       unsigned argc, uint32 argv[])
{
  wasm_interp_call_wasm(function, argc, argv);
}

WASMModule*
wasm_runtime_load(const uint8 *buf, uint32 size)
{
  return wasm_loader_load(buf, size);
}

void
wasm_runtime_unload(WASMModule *module)
{
  wasm_loader_unload(module);
}

/**
 * Destroy memory instances.
 */
static void
memories_deinstantiate(WASMMemoryInstance **memories, uint32 count)
{
  uint32 i;
  if (memories) {
    for (i = 0; i < count; i++)
      if (memories[i])
        bh_free(memories[i]);
    bh_free(memories);
  }
}

/**
 * Instantiate memories in a module.
 */
static WASMMemoryInstance**
memories_instantiate(const WASMModule *module, uint32 global_data_size)
{
  WASMImport *import;
  uint32 mem_index = 0, i, memory_count =
    module->import_memory_count + module->memory_count;
  uint32 total_size = sizeof(WASMMemoryInstance*) * memory_count;
  WASMMemoryInstance **memories = bh_malloc(total_size), *memory;

  if (!memories) {
    printf("Instantiate memory failed: allocate memory failed.\n");
    return NULL;
  }

  memset(memories, 0, total_size);

  /* instantiate memories from import section */
  import = module->imports;
  for (i = 0; i < module->import_count; i++, import++) {
    if (import->kind == IMPORT_KIND_MEMORY) {
      total_size = offsetof(WASMMemoryInstance, base_addr) +
                   NumBytesPerPage * import->u.memory.init_page_count +
                   global_data_size;
      if (!(memory = memories[mem_index++] = bh_malloc(total_size))) {
        printf("Instantiate memory failed: allocate memory failed.\n");
        memories_deinstantiate(memories, memory_count);
        return NULL;
      }

      memset(memory, 0, total_size);
      memory->cur_page_count = import->u.memory.init_page_count;
      memory->max_page_count = import->u.memory.max_page_count;
    }
  }

  /* instantiate memories from memory section */
  for (i = 0; i < module->memory_count; i++) {
    total_size = offsetof(WASMMemoryInstance, base_addr) +
                 NumBytesPerPage * module->memories[i].init_page_count +
                 global_data_size;
    if (!(memory = memories[mem_index++] = bh_malloc(total_size))) {
      printf("Instantiate memory failed: allocate memory failed.\n");
      memories_deinstantiate(memories, memory_count);
      return NULL;
    }

    memset(memory, 0, total_size);
    memory->cur_page_count = module->memories[i].init_page_count;
    memory->max_page_count = module->memories[i].max_page_count;
  }

  bh_assert(mem_index == memory_count);
  return memories;
}

/**
 * Destroy table instances.
 */
static void
tables_deinstantiate(WASMTableInstance **tables, uint32 count)
{
  uint32 i;
  if (tables) {
    for (i = 0; i < count; i++)
      if (tables[i])
        bh_free(tables[i]);
    bh_free(tables);
  }
}

/**
 * Instantiate tables in a module.
 */
static WASMTableInstance**
tables_instantiate(const WASMModule *module)
{
  WASMImport *import;
  uint32 table_index = 0, i, table_count =
    module->import_table_count + module->table_count;
  uint32 total_size = sizeof(WASMTableInstance*) * table_count;
  WASMTableInstance **tables = bh_malloc(total_size), *table;

  if (!tables) {
    printf("Instantiate table failed: allocate memory failed.\n");
    return NULL;
  }

  memset(tables, 0, total_size);

  /* instantiate tables from import section */
  import = module->imports;
  for (i = 0; i < module->import_count; i++, import++) {
    if (import->kind == IMPORT_KIND_TABLE) {
      total_size = offsetof(WASMTableInstance, base_addr) +
                   sizeof(uint32) * import->u.table.init_size;
      if (!(table = tables[table_index++] = bh_malloc(total_size))) {
        printf("Instantiate table failed: allocate memory failed.\n");
        tables_deinstantiate(tables, table_count);
        return NULL;
      }

      memset(table, 0, total_size);
      table->cur_size = import->u.table.init_size;
      table->max_size = import->u.table.max_size;
    }
  }

  /* instantiate tables from table section */
  for (i = 0; i < module->table_count; i++) {
    total_size = offsetof(WASMTableInstance, base_addr) +
                 sizeof(uint32) * module->tables[i].init_size;
    if (!(table = tables[table_index++] = bh_malloc(total_size))) {
      printf("Instantiate table failed: allocate memory failed.\n");
      tables_deinstantiate(tables, table_count);
      return NULL;
    }

    memset(table, 0, total_size);
    table->cur_size = module->tables[i].init_size;
    table->max_size = module->tables[i].max_size;
  }

  bh_assert(table_index == table_count);
  return tables;
}

/**
 * Destroy function instances.
 */
static void
functions_deinstantiate(WASMFunctionInstance *functions)
{
  if (functions)
    bh_free(functions);
}

/**
 * Instantiate functions in a module.
 */
static WASMFunctionInstance*
functions_instantiate(const WASMModule *module)
{
  WASMImport *import;
  uint32 i, function_count =
    module->import_function_count + module->function_count;
  uint32 total_size = sizeof(WASMFunctionInstance) * function_count;
  WASMFunctionInstance *functions = bh_malloc(total_size), *function;

  if (!functions) {
    printf("Instantiate function failed: allocate memory failed.\n");
    return NULL;
  }

  memset(functions, 0, total_size);

  /* instantiate functions from import section */
  function = functions;
  import = module->imports;
  for (i = 0; i < module->import_count; i++, import++) {
    if (import->kind == IMPORT_KIND_FUNC) {
      function->is_import_func = true;
      function->u.func_import = &import->u.function;

      function->param_cell_num =
        wasm_type_param_cell_num(import->u.function.func_type);
      function->ret_cell_num =
        wasm_type_return_cell_num(import->u.function.func_type);
      function->local_cell_num = 0;

      function++;
    }
  }

  /* instantiate functions from function section */
  for (i = 0; i < module->function_count; i++) {
    function->is_import_func = false;
    function->u.func = module->functions[i];

    function->param_cell_num =
      wasm_type_param_cell_num(function->u.func->func_type);
    function->ret_cell_num =
      wasm_type_return_cell_num(function->u.func->func_type);
    function->local_cell_num =
      wasm_get_cell_num(function->u.func->local_types,
                        function->u.func->local_count);

    function++;
  }

  bh_assert((uint32)(function - functions) == function_count);
  return functions;
}

/**
 * Destroy global instances.
 */
static void
globals_deinstantiate(WASMGlobalInstance *globals)
{
  if (globals)
    bh_free(globals);
}

/**
 * Instantiate globals in a module.
 */
static WASMGlobalInstance*
globals_instantiate(const WASMModule *module,
                    uint32 *p_global_data_size)
{
  WASMImport *import;
  uint32 global_data_offset = 0;
  uint32 i, global_count =
    module->import_global_count + module->global_count;
  uint32 total_size = sizeof(WASMGlobalInstance) * global_count;
  WASMGlobalInstance *globals = bh_malloc(total_size), *global;

  if (!globals) {
    printf("Instantiate global failed: allocate memory failed.\n");
    return NULL;
  }

  memset(globals, 0, total_size);

  /* instantiate globals from import section */
  global = globals;
  import = module->imports;
  for (i = 0; i < module->import_count; i++, import++) {
    if (import->kind == IMPORT_KIND_GLOBAL) {
      WASMGlobalImport *global_import = &module->imports[i].u.global;
      global->type = global_import->type;
      global->is_mutable = global_import->is_mutable;
      global->initial_value = global_import->global_data_linked;
      global->data_offset = global_data_offset;
      global_data_offset += wasm_value_type_size(global->type);

      global++;
    }
  }

  /* instantiate globals from global section */
  for (i = 0; i < module->global_count; i++) {
    InitializerExpression *init_expr = &module->globals[i].init_expr;

    global->type = module->globals[i].type;
    global->is_mutable = module->globals[i].is_mutable;

    if (init_expr->init_expr_type == INIT_EXPR_TYPE_GET_GLOBAL) {
      bh_assert(init_expr->u.global_index < module->import_global_count);
      global->initial_value = globals[init_expr->u.global_index].initial_value;
    }
    else {
      memcpy(&global->initial_value, &init_expr->u, sizeof(int64));
    }

    global->data_offset = global_data_offset;
    global_data_offset += wasm_value_type_size(global->type);

    global++;
  }

  bh_assert((uint32)(global - globals) == global_count);
  *p_global_data_size = global_data_offset;
  return globals;
}

/**
 * Return export function count in module export section.
 */
static uint32
get_export_function_count(const WASMModule *module)
{
  WASMExport *export = module->exports;
  uint32 count = 0, i;

  for (i = 0; i < module->export_count; i++, export++)
    if (export->kind == EXPORT_KIND_FUNC)
      count++;

  return count;
}

/**
 * Destroy export function instances.
 */
static void
export_functions_deinstantiate(WASMExportFuncInstance *functions)
{
  if (functions)
    bh_free(functions);
}

/**
 * Instantiate export functions in a module.
 */
static WASMExportFuncInstance*
export_functions_instantiate(const WASMModule *module,
                             WASMModuleInstance *module_inst,
                             uint32 export_func_count)
{
  WASMExportFuncInstance *export_funcs, *export_func;
  WASMExport *export = module->exports;
  uint32 i, total_size = sizeof(WASMExportFuncInstance) * export_func_count;

  if (!(export_func = export_funcs = bh_malloc(total_size))) {
    printf("Instantiate export function failed: allocate memory failed.\n");
    return NULL;
  }

  memset(export_funcs, 0, total_size);

  for (i = 0; i < module->export_count; i++, export++)
    if (export->kind == EXPORT_KIND_FUNC) {
      bh_assert(export->index >= module->import_function_count
                && export->index < module->import_function_count
                                   + module->function_count);
      export_func->name = export->name;
      export_func->function = &module_inst->functions[export->index];
      export_func++;
    }

  bh_assert((uint32)(export_func - export_funcs) == export_func_count);
  return export_funcs;
}

void
wasm_runtime_deinstantiate(WASMModuleInstance *module_inst);

/**
 * Instantiate module
 */
WASMModuleInstance*
wasm_runtime_instantiate(const WASMModule *module)
{
  WASMModuleInstance *module_inst;
  WASMTableSeg *table_seg;
  WASMDataSeg *data_seg;
  WASMGlobalInstance *globals = NULL, *global;
  uint32 global_count, global_data_size = 0, i;
  uint8 *global_data, *global_data_end, *memory_data;
  uint32 *table_data;

  if (!module)
    return NULL;

  /* Instantiate global firstly to get the mutable data size */
  global_count = module->import_global_count + module->global_count;
  if (global_count &&
      !(globals = globals_instantiate(module, &global_data_size)))
    return NULL;

  /* Allocate the memory */
  if (!(module_inst = bh_malloc(sizeof(WASMModuleInstance)))) {
    printf("Instantiate module failed: allocate memory failed.\n");
    globals_deinstantiate(globals);
    return NULL;
  }

  memset(module_inst, 0, sizeof(WASMModuleInstance));
  module_inst->global_count = global_count;
  module_inst->globals = globals;

  module_inst->memory_count =
    module->import_memory_count + module->memory_count;
  module_inst->table_count =
    module->import_table_count + module->table_count;
  module_inst->function_count =
    module->import_function_count + module->function_count;
  module_inst->export_func_count = get_export_function_count(module);

  /* Instantiate memories/tables/functions */
  if ((module_inst->memory_count > 0
       && !(module_inst->memories = memories_instantiate(module, global_data_size)))
      || (module_inst->table_count > 0
          && !(module_inst->tables = tables_instantiate(module)))
      || (module_inst->function_count > 0
          && !(module_inst->functions = functions_instantiate(module)))
      || (module_inst->export_func_count > 0
          && !(module_inst->export_functions = export_functions_instantiate(
                    module, module_inst, module_inst->export_func_count)))) {
    wasm_runtime_deinstantiate(module_inst);
    return NULL;
  }

  if (module_inst->memory_count) {
    WASMMemoryInstance *memory;
    memory = module_inst->default_memory = module_inst->memories[0];

    /* Initialize the memory data with data segment section */
    memory_data = module_inst->default_memory->base_addr;
    for (i = 0; i < module->data_seg_count; i++) {
      data_seg = module->data_segments[i];
      bh_assert(data_seg->memory_index == 0);
      bh_assert(data_seg->base_offset.init_expr_type ==
          INIT_EXPR_TYPE_I32_CONST);
      bh_assert((uint32)data_seg->base_offset.u.i32 <
          NumBytesPerPage * module_inst->default_memory->cur_page_count);

      memcpy(memory_data + data_seg->base_offset.u.i32,
          data_seg->data, data_seg->data_length);
    }

    /* Initialize the global data */
    global_data = module_inst->global_data =
      memory->base_addr + NumBytesPerPage * memory->cur_page_count;
    global_data_end = global_data + global_data_size;
    global = globals;
    for (i = 0; i < global_count; i++, global++) {
      switch (global->type) {
        case VALUE_TYPE_I32:
        case VALUE_TYPE_F32:
          memcpy(global_data, &global->initial_value.i32, sizeof(int32));
          global_data += sizeof(int32);
          break;
        case VALUE_TYPE_I64:
        case VALUE_TYPE_F64:
          memcpy(global_data, &global->initial_value.i64, sizeof(int64));
          global_data += sizeof(int64);
          break;
        default:
          bh_assert(0);
      }
    }
    bh_assert(global_data == global_data_end);
  }

  if (module_inst->table_count) {
    module_inst->default_table = module_inst->tables[0];

    /* Initialize the table data with table segment section */
    table_data = (uint32*)module_inst->default_table->base_addr;
    table_seg = module->table_segments;
    for (i = 0; i < module->table_seg_count; i++, table_seg++) {
      bh_assert(table_seg->table_index == 0);
      bh_assert(table_seg->base_offset.init_expr_type ==
                INIT_EXPR_TYPE_I32_CONST
                || table_seg->base_offset.init_expr_type ==
                   INIT_EXPR_TYPE_GET_GLOBAL);

      if (table_seg->base_offset.init_expr_type ==
          INIT_EXPR_TYPE_GET_GLOBAL) {
        bh_assert(table_seg->base_offset.u.global_index < global_count
                  && globals[table_seg->base_offset.u.global_index].type ==
                     VALUE_TYPE_I32);
        table_seg->base_offset.u.i32 =
          globals[table_seg->base_offset.u.global_index].initial_value.i32;
      }

      bh_assert((uint32)table_seg->base_offset.u.i32 <
                module_inst->default_table->cur_size);
      memcpy(table_data + table_seg->base_offset.u.i32,
             table_seg->func_indexes,
             sizeof(uint32) * table_seg->function_count);
    }
  }

  if (module->start_function) {
    bh_assert(module->start_function >= module->import_function_count);
    module_inst->start_function =
      &module_inst->functions[module->start_function];
  }

  return module_inst;
}

void
wasm_runtime_deinstantiate(WASMModuleInstance *module_inst)
{
  if (!module_inst)
    return;

  memories_deinstantiate(module_inst->memories, module_inst->memory_count);
  tables_deinstantiate(module_inst->tables, module_inst->table_count);
  functions_deinstantiate(module_inst->functions);
  globals_deinstantiate(module_inst->globals);
  export_functions_deinstantiate(module_inst->export_functions);
  bh_free(module_inst);
}

static void*
wasm_thread_start(void *arg)
{
  return NULL;
}

static void
app_instance_cleanup(WASMVmInstance *ilr)
{
  void (*cleanup_routine)();

  /* Remove vm from the VM instance list before it's actually
     destroyed to avoid operations by other threads after it's
     destroyed.  */
  vmci_thread_mutex_lock(&instance_list_lock);
  ilr->prev->next = ilr->next;
  ilr->next->prev = ilr->prev;
  ilr->next = ilr->prev = NULL;
  vmci_thread_mutex_unlock(&instance_list_lock);

  /* Set the cleanup routine.  */
  cleanup_routine = ilr->cleanup_routine;

  /* Release resources in vm and set it to destroyed state (whose
     main_file is set to NULL). */
  wasm_thread_destroy_ilr(ilr);

  /* Call the cleanup routine at the last step after unlock.  */
  if (cleanup_routine)
    (*cleanup_routine)();
}

/**
 * Entry point of the main thread of VM instance.
 *
 * @param arg pointer to the current VM instance
 *
 * @return return value of the start routine (pointed to by
 * start_routine) of the instance or NULL if initialization fails
 */
static void* vmci_thread_start_routine_modifier
app_instance_start (void *arg)
{
  void *retval = NULL;
  WASMVmInstance *ilr = (WASMVmInstance*)arg;
  WASMThread *self = &ilr->main_tlr;
  vmci_thread_t handle = self->handle;

  /* This must be a new thread that has not been initialized. */
  bh_assert(!wasm_runtime_get_self());

  /* Set the native stack boundary for this thread. */
  wasm_thread_set_native_stack_boundary(self);

  /* Set the thread local root. */
  wasm_runtime_set_tlr(self);

  /* Run the start routine. */
  retval = (*ilr->start_routine)(ilr->start_routine_arg);

  /* WASM stack must be empty.  */
  bh_assert(self->wasm_stack.s.top == self->wasm_stack.s.bottom);

  /* Change to ZOMBIE state before dying (and locks being destroyed)
     so that possible destroying thread won't be blocked.  If the
     destroying thread find that the main thread of an instance is in
     ZOMBIE state, it should't cancel the threads of that instance
     though ZOMBIE is a safe state because in this case that "ZOMBIE"
     thread is doing cleanup, which shall not be killed.  */
  /*TODO: modify to wasm_runtime_change_state(WASM_THREAD_ZOMBIE);*/
  self->state = WASM_THREAD_ZOMBIE;

  /* Cleanup app instance */
  app_instance_cleanup(ilr);

  /* Release system resource by ourselves when exit normally. */
  vmci_thread_detach(handle);

  /* Call exit explicitly because some systems may need to do
     something in the exit function.  */
  vmci_thread_exit(retval);
  return NULL;
}

WASMVmInstance*
wasm_runtime_create_instance(WASMModuleInstance *module_inst,
                             uint32 native_stack_size,
                             uint32 wasm_stack_size,
                             void *(*start_routine)(void*), void *arg,
                             void (*cleanup_routine)(void))
{
  WASMVmInstance *ilr;
  uint32 native_stack_size1 = native_stack_size +
                              vmci_reserved_native_stack_size;
  uint32 wasm_stack_size1 = wasm_stack_size +
                            vmci_reserved_wasm_stack_size;

  if (!(ilr = wasm_thread_create_ilr(module_inst,
                                     native_stack_size1, wasm_stack_size1,
                                     start_routine, arg, cleanup_routine)))
    return NULL;

  /* Set this to make the main thread to be a WASM thread. */
  ilr->main_tlr.start_routine = wasm_thread_start;

  if (vmci_thread_create(&ilr->main_tlr.handle,
                         &app_instance_start, ilr,
                         ilr->native_stack_size)) {
    wasm_thread_destroy_ilr(ilr);
    return NULL;
  }

  /* Insert the new VM instance to the VM instance list. */
  vmci_thread_mutex_lock(&instance_list_lock);
  supervisor_instance->next->prev = ilr;
  ilr->next = supervisor_instance->next;
  ilr->prev = supervisor_instance;
  supervisor_instance->next = ilr;
  vmci_thread_mutex_unlock(&instance_list_lock);

  return ilr;
}

void
wasm_runtime_destroy_instance(WASMVmInstance *ilr)
{
  WASMThread *self = wasm_runtime_get_self();
  WASMVmInstance *self_ilr = self->vm_instance;

  /* We cannot destroy ourselves. */
  bh_assert(self_ilr != ilr);

  (void)self_ilr;
}

void
wasm_runtime_wait_for_instance(WASMVmInstance *ilr, int mills)
{
  wasm_thread_wait_for_instance(ilr, mills);
}

