#include "lib-export-template.c"

void *get_extended_symbol_ptr(int *p_size)
{
  *p_size = sizeof(extended_native_symbol_defs) / sizeof(NativeSymbol);
  return extended_native_symbol_defs;
}
