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

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "bh_assert.h"
#include "bh_log.h"
#include "bh_platform.h"
#include "bh_platform_log.h"
#include "bh_thread.h"
#include "wasm-export.h"
#include "bh_memory.h"


static int app_argc;
static char **app_argv;

void*
vmci_get_tl_root(void)
{
  return vm_tls_get(0);
}

void
vmci_set_tl_root(void *tlr)
{
  vm_tls_put(0, tlr);
}

static int
print_help()
{
  bh_printf("Usage: iwasm [-options] wasm_file [args...]\n");
  bh_printf("options:\n");
  bh_printf("  -f|--function name     Specify function name to run "
            "in module rather than main\n");
#if WASM_ENABLE_LOG != 0
  bh_printf("  -v=X                   Set log verbose level (0 to 2, default is 1), larger level with more log\n");
#endif
#ifdef WASM_ENABLE_REPL
  bh_printf("  --repl                 Start a very simple REPL (read-eval-print-loop) mode \n"
            "                         that runs commands in the form of `FUNC ARG...`\n");
#endif

  return 1;
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
  wasm_application_execute_main(app_argc, app_argv);
  if ((exception = wasm_runtime_get_exception()))
    bh_printf("%s", exception);
  return NULL;
}

#ifdef WASM_ENABLE_REPL
/**
 * Split a space separated strings into an array of strings
 * Returns NULL on failure
 * Memory must be freed by caller
 * Based on: http://stackoverflow.com/a/11198630/471795
 */
static char **
split_string(char *str, int *count)
{
  char **res = NULL;
  char *p;
  int idx = 0;

  /* split string and append tokens to 'res' */
  do {
    p = strtok(str, " ");
    str = NULL;
    res = realloc(res, sizeof(char*) * (idx + 1));
    if (res == NULL) {
      return NULL;
    }
    res[idx++] = p;
  } while (p);

  if (count) { *count = idx - 1; }
  return res;
}

static void*
app_instance_func(void *arg)
{
  char *cmd = NULL;
  size_t len = 0;
  ssize_t n;

  while ((bh_printf("webassembly> "), n = getline(&cmd, &len, stdin)) != -1) {
    bh_assert(n > 0);
    if (cmd[n - 1] == '\n') {
      if (n == 1)
        continue;
      else
        cmd[n - 1] = '\0';
    }
    app_argv = split_string(cmd, &app_argc);
    if (app_argv == NULL) {
      LOG_ERROR("Wasm prepare param failed: split string failed.\n");
      break;
    }
    if (app_argc != 0) {
      wasm_application_execute_func(app_argc, app_argv);
      /* TODO: check exception */
    }
    free(app_argv);
  }
  free(cmd);
  return NULL;
}
#endif /* WASM_ENABLE_REPL */

int
main(int argc, char *argv[])
{
  char *wasm_file = NULL;
  const char *func_name = NULL;
  uint8 *wasm_file_buf = NULL;
  int wasm_file_size;
  wasm_module_t wasm_module = NULL;
  wasm_module_inst_t wasm_module_inst = NULL;
  wasm_vm_instance_t vm = NULL;
  char error_buf[64];
#if WASM_ENABLE_LOG != 0
  int log_verbose_level = 1;
#endif
#ifdef WASM_ENABLE_REPL
  bool is_repl_mode = false;
#endif

  /* Process options.  */
  for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
    if (!strcmp(argv[0], "-f")
        || !strcmp(argv[0], "--function")) {
       argc--, argv++;
       if (argc < 2) {
         print_help();
         return 0;
       }
       func_name = argv[0];
    }
#if WASM_ENABLE_LOG != 0
    else if (!strncmp (argv[0], "-v=", 3)) {
      log_verbose_level = atoi(argv[0] + 3);
      if (log_verbose_level < 0 || log_verbose_level > 2)
        return print_help ();
    }
#endif
#ifdef WASM_ENABLE_REPL
    else if (!strcmp(argv[0], "--repl"))
      is_repl_mode = true;
#endif
    else
      return print_help();
  }

  if (argc == 0)
    return print_help();

  wasm_file = argv[0];
  app_argc = argc - 1;
  app_argv = argv + 1;

  /* initialize runtime environment */
  if (!wasm_runtime_init())
    return -1;

  bh_log_set_verbose_level(log_verbose_level);

  /* load WASM byte buffer from WASM bin file */
  if (!(wasm_file_buf = (uint8*)
        bh_read_file_to_buffer(wasm_file, &wasm_file_size)))
    goto fail1;

  /* load WASM module */
  if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                        error_buf, sizeof(error_buf)))) {
    bh_printf("%s\n", error_buf);
    goto fail2;
  }

  /* instantiate the module */
  if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module,
                                                    error_buf,
                                                    sizeof(error_buf)))) {
    bh_printf("%s\n", error_buf);
    goto fail3;
  }

#ifdef WASM_ENABLE_REPL
  if (is_repl_mode) {
    /* create vm instance */
    if (!(vm = wasm_runtime_create_instance(wasm_module_inst,
                                            1024 * 1024, /* TODO, define macro */
                                            1024 * 1024, /* TODO, define macro */
                                            app_instance_func, NULL,
                                            app_instance_cleanup)))
      goto fail4;
  } else
#endif
  {
    /* create vm instance */
    if (!(vm = wasm_runtime_create_instance(wasm_module_inst,
                                            32 * 1024, /* TODO, define macro */
                                            32 * 1024, /* TODO, define macro */
                                            app_instance_main, NULL,
                                            app_instance_cleanup)))
      goto fail4;
  }

  /* wait for the instance to terminate */
  wasm_runtime_wait_for_instance(vm, -1);

  /* destroy the instance */
  wasm_runtime_destroy_instance(vm);

fail4:
  /* destroy the module instance */
  wasm_runtime_deinstantiate(wasm_module_inst);

fail3:
  /* unload the module */
  wasm_runtime_unload(wasm_module);

fail2:
  /* free the file buffer */
  bh_free(wasm_file_buf);

fail1:
  /* destroy runtime environment */
  wasm_runtime_destroy();

  (void)func_name;
  return 0;
}

