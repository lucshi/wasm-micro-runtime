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

#include <stdlib.h>
#include <string.h>
#include "wasm_assert.h"
#include "wasm_log.h"
#include "wasm_platform.h"
#include "wasm_platform_log.h"
#include "wasm_thread.h"
#include "wasm-export.h"
#include "wasm_memory.h"
#include "test_wasm.h"
#include "ems_gc.h"


static int app_argc;
static char **app_argv;

void*
wsci_get_tl_root(void)
{
  return ws_tls_get(0);
}

void
wsci_set_tl_root(void *tlr)
{
  ws_tls_put(0, tlr);
}

static void
app_instance_cleanup(void)
{
}

/**
 * The start routine of the main thread of app instance.
 */
static void*
app_instance_main(void *arg)
{
  const char *exception;
  bool res;

  res = wasm_application_execute_start();
  if ((exception = wasm_runtime_get_exception()))
    wasm_printf("%s\n", exception);
  if (!res)
    return NULL;

  wasm_application_execute_main(app_argc, app_argv);
  if ((exception = wasm_runtime_get_exception()))
    wasm_printf("%s\n", exception);
  return NULL;
}

void iwasm_main(void *arg1)
{
  uint8 *wasm_file_buf = NULL;
  int wasm_file_size;
  wasm_module_t wasm_module = NULL;
  wasm_module_inst_t wasm_module_inst = NULL;
  wasm_vm_instance_t vm = NULL;
  char error_buf[128];
#if WASM_ENABLE_LOG != 0
  int log_verbose_level = 1;
#endif

  (void)arg1;

  /* initialize runtime environment */
  if (!wasm_runtime_init())
    return;

#if WASM_ENABLE_LOG != 0
  wasm_log_set_verbose_level(log_verbose_level);
#endif

  /* load WASM byte buffer from byte buffer of include file */
  wasm_file_buf = (uint8*)wasm_test_file;
  wasm_file_size = sizeof(wasm_test_file);

  /* load WASM module */
  if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                        error_buf, sizeof(error_buf)))) {
    wasm_printf("%s\n", error_buf);
    goto fail1;
  }

  /* instantiate the module */
  if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module,
                                                    0, NULL,
                                                    error_buf,
                                                    sizeof(error_buf)))) {
    wasm_printf("%s\n", error_buf);
    goto fail2;
  }

  /* create vm instance */
  if (!(vm = wasm_runtime_create_instance(wasm_module_inst,
                                          8 * 1024, /* native stack size */
                                          8 * 1024, /* wasm stack size */
                                          app_instance_main, NULL,
                                          app_instance_cleanup)))
    goto fail3;

  /* wait for the instance to terminate */
  wasm_runtime_wait_for_instance(vm, -1);

  /* destroy the instance */
  wasm_runtime_destroy_instance(vm);

fail3:
  /* destroy the module instance */
  wasm_runtime_deinstantiate(wasm_module_inst);

fail2:
  /* unload the module */
  wasm_runtime_unload(wasm_module);

fail1:
  /* destroy runtime environment */
  wasm_runtime_destroy();

#if 0   /* print the memory usage */
  int stats[GC_STAT_MAX];
  gc_heap_stats(NULL, stats, GC_STAT_MAX, MMT_INSTANCE);
  printf("heap status: total: %d, free: %d, highmark: %d\n",
         stats[GC_STAT_TOTAL], stats[GC_STAT_FREE], stats[GC_STAT_HIGHMARK]);
#endif
}

#define DEFAULT_THREAD_STACKSIZE (6 * 1024)
#define DEFAULT_THREAD_PRIORITY 50

bool
iwasm_init(void)
{
  int ret = aos_task_new("wasm-main", iwasm_main, NULL,
                         DEFAULT_THREAD_STACKSIZE);
  return ret == 0 ? true : false;
}

