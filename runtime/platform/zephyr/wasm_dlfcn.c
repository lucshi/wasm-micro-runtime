#include <autoconf.h>
#include <zephyr.h>
#include <kernel.h>
#include <misc/printk.h>
#include <gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool sort_flag = false;

typedef struct NativeSymbol {
  const char *symbol;
  void *func_ptr;
} NativeSymbol;

#define REG_SYMBOL(symbol) {#symbol, symbol}

static int
wasm_putchar(int c)
{
  printk("%c", (char)c);
  return 1;
}

static NativeSymbol native_symbol_defs[] = {
  REG_SYMBOL(memcmp),
  REG_SYMBOL(memcpy),
  REG_SYMBOL(memmove),
  REG_SYMBOL(memset),
  { "putchar", wasm_putchar },
  REG_SYMBOL(snprintf),
  REG_SYMBOL(sprintf),
  REG_SYMBOL(strchr),
  REG_SYMBOL(strcmp),
  REG_SYMBOL(strcpy),
  REG_SYMBOL(strlen),
  REG_SYMBOL(strncmp),
  REG_SYMBOL(strncpy),
};

extern void *get_extended_symbol_ptr(int *p_size);

static bool
sort_symbol_ptr(NativeSymbol *ptr, int len)
{
  int i, j;
  NativeSymbol temp;

  for (i = 0; i < len - 1; ++i) {
    for (j = i + 1; j < len; ++j) {
      if (strcmp((ptr+i)->symbol, (ptr+j)->symbol) > 0) {
        temp = ptr[i];
        ptr[i] = ptr[j];
        ptr[j] = temp;
      }
    }
  }

  return true;
}

static void *
lookup_symbol(NativeSymbol *ptr, int len, const char *symbol)
{
  int low = 0, mid, ret;
  int high = len - 1;

  while (low <= high) {
    mid = (low + high) / 2;
    ret = strcmp(symbol, ptr[mid].symbol);

    if (ret == 0)
      return ptr[mid].func_ptr;
    else if (ret < 0)
      high = mid - 1;
    else
      low = mid + 1;
  }

  return NULL;
}

void *
wasm_dlsym(void *handle, const char *symbol)
{
  int len;
  NativeSymbol *ext_native_symbol_defs = get_extended_symbol_ptr(&len);
  void *ret;

  if (!sort_flag) {
    sort_symbol_ptr(native_symbol_defs, sizeof(native_symbol_defs) / sizeof(NativeSymbol));
    sort_symbol_ptr(ext_native_symbol_defs, len);
    sort_flag = true;
  }

  if (!symbol)
    return NULL;

  ret = lookup_symbol(native_symbol_defs, sizeof(native_symbol_defs) / sizeof(NativeSymbol), symbol);
  if (ret)
    return ret;
  ret = lookup_symbol(ext_native_symbol_defs, len, symbol);
  if (ret)
    return ret;

  return NULL;
}
