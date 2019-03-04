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

static void*
app_instance_main(wasm_module_inst_t module_inst)
{
  const char *exception;

  wasm_application_execute_main(module_inst, app_argc, app_argv);
  if ((exception = wasm_runtime_get_exception(module_inst)))
    wasm_printf("%s\n", exception);
  return NULL;
}

void iwasm_main(void *arg1, void *arg2, void *arg3)
{
  uint8 *wasm_file_buf = NULL;
  int wasm_file_size;
  wasm_module_t wasm_module = NULL;
  wasm_module_inst_t wasm_module_inst = NULL;
  char error_buf[128];
#if WASM_ENABLE_LOG != 0
  int log_verbose_level = 1;
#endif

  (void)arg1;
  (void)arg2;
  (void)arg3;

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
                                                    8 * 1024,
                                                    error_buf,
                                                    sizeof(error_buf)))) {
    wasm_printf("%s\n", error_buf);
    goto fail2;
  }

  app_instance_main(wasm_module_inst);

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
#define DEFAULT_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(iwasm_main_thread_stack, DEFAULT_THREAD_STACKSIZE);
static struct k_thread iwasm_main_thread;

bool
iwasm_init(void)
{
  k_tid_t tid = k_thread_create(&iwasm_main_thread,
                                iwasm_main_thread_stack,
                                DEFAULT_THREAD_STACKSIZE,
                                iwasm_main, NULL, NULL, NULL,
                                DEFAULT_THREAD_PRIORITY, 0, K_NO_WAIT);
  return tid ? true : false;
}

#ifndef CONFIG_AEE
void main(void)
{
  iwasm_init();
}
#endif

