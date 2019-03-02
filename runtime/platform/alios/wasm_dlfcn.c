#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct NativeSymbol {
  const char *symbol;
  void *func_ptr;
  char *signature;
} NativeSymbol;

#define REG_SYMBOL(symbol) {#symbol, symbol, NULL }

static const NativeSymbol native_symbol_defs[] = {
  REG_SYMBOL(memcmp),
  REG_SYMBOL(memcpy),
  REG_SYMBOL(memmove),
  REG_SYMBOL(memset),
  REG_SYMBOL(putchar),
  REG_SYMBOL(snprintf),
  REG_SYMBOL(sprintf),
  REG_SYMBOL(strchr),
  REG_SYMBOL(strcmp),
  REG_SYMBOL(strcpy),
  REG_SYMBOL(strlen),
  REG_SYMBOL(strncmp),
  REG_SYMBOL(strncpy),
};

void *
wasm_dlsym(void *handle, const char *symbol)
{
  int low = 0;
  int high = sizeof(native_symbol_defs) / sizeof(NativeSymbol) - 1;
  int mid, ret;

  if (!symbol)
    return NULL;

  while (low <= high) {
    mid = (low + high) / 2;
    ret = strcmp(symbol, native_symbol_defs[mid].symbol);

    if (ret == 0)
      return native_symbol_defs[mid].func_ptr;
    else if (ret < 0)
      high = mid - 1;
    else
      low = mid + 1;
  }

  return NULL;
}

