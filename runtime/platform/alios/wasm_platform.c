#include "wasm_platform.h"
#include "ems_gc.h"


int wasm_platform_init()
{
  if (!gc_init(16 * 1024 * 1024))
    return -1;

  return 0;
}

