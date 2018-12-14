#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NativeSymbol {
  const char *symbol;
  void *func_ptr;
} NativeSymbol;

#define REG_SYMBOL(symbol) {#symbol, symbol}

static NativeSymbol extended_native_symbol_defs[] = {
/* TODO: use macro REG_SYMBOL() to add functions to register. */
};

int get_extended_symbol_count()
{
  return sizeof(extended_native_symbol_defs) / sizeof(NativeSymbol);
}

void* get_extended_symbol_ptr()
{
  return extended_native_symbol_defs;
}
