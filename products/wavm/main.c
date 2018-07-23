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

#include <stdio.h>
#include "bh_platform.h"
#include "wasm-export.h"
#include "bh_memory.h"


static int app_argc;
static char **app_argv;

static int
print_help()
{
  printf("Usage: wavm [-options] wasm_file [args...]\n");
  printf("options:\n");
  printf("  -f|--function name     Specify function name to run in module rather than main\n");

  return 1;
}

int
main(int argc, char *argv[])
{
  char *wasm_file = NULL;
  const char *func_name = NULL;
  uint8 *wasm_file_buf = NULL;
  int wasm_file_size;
  struct WASMModule *wasm_module = NULL;

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
    else
      return print_help();
  }

  if (argc == 0)
    return print_help();

  wasm_file = argv[0];
  app_argc = argc - 1;
  app_argv = argv + 1;

  if (!(wasm_file_buf = (uint8*)
        bh_read_file_to_buffer(wasm_file, &wasm_file_size))) {
    return -1;
  }

  if (!(wasm_module = wasm_wasm_module_load(wasm_file_buf, wasm_file_size))) {
    goto fail1;
  }

  /* TODO: link module */

  wasm_wasm_module_unload(wasm_module);

fail1:
  bh_free(wasm_file_buf);

  (void)func_name;
  return 0;
}

