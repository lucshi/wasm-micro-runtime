#ifndef _LIB_EXPORT_H_
#define _LIB_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NativeSymbol {
  const char *symbol;
  void *func_ptr;
} NativeSymbol;

#define EXPORT_WASM_API(symbol)  {#symbol, symbol}
#define EXPORT_WASM_API2(symbol) {#symbol, symbol##_wrapper}

#ifdef __cplusplus
}
#endif

#endif
