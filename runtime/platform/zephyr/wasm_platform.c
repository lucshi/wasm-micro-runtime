#include "wasm_platform.h"
#include "ems_gc.h"

#ifndef CONFIG_AEE_ENABLE
static int
_stdout_hook_iwasm(int c)
{
  printk("%c", (char)c);
  return 1;
}

extern void __stdout_hook_install(int (*hook)(int));
#endif

int wasm_platform_init()
{
#ifndef CONFIG_AEE_ENABLE
  /* Enable printf() in Zephyr */
  __stdout_hook_install(_stdout_hook_iwasm);

  if (!gc_init(16 * 1024 * 1024))
    return -1;
#endif

  return 0;
}

