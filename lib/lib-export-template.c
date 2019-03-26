#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib-export.h"

#ifdef WASM_ENABLE_AEE_LIB
#ifdef __ZEPHYR__
#include <autoconf.h>
#endif
#include "open-app.h"
#endif

NativeSymbol extended_native_symbol_defs[] = {
/* TODO: use macro EXPORT_WASM_API() or EXPORT_WASM_API2() to add functions to register. */
#ifdef WASM_ENABLE_AEE_LIB
  EXPORT_WASM_API(attr_container_create),
  EXPORT_WASM_API(attr_container_destroy),
  EXPORT_WASM_API(attr_container_set_short),
  EXPORT_WASM_API(attr_container_set_int),
  EXPORT_WASM_API(attr_container_set_int64),
  EXPORT_WASM_API(attr_container_set_byte),
  EXPORT_WASM_API(attr_container_set_uint16),
  EXPORT_WASM_API(attr_container_set_float),
  EXPORT_WASM_API(attr_container_set_double),
  EXPORT_WASM_API(attr_container_set_bool),
  EXPORT_WASM_API(attr_container_set_string),
  EXPORT_WASM_API(attr_container_set_bytearray),
  EXPORT_WASM_API(attr_container_get_tag),
  EXPORT_WASM_API(attr_container_get_attr_num),
  EXPORT_WASM_API(attr_container_contain_key),
  EXPORT_WASM_API(attr_container_get_as_short),
  EXPORT_WASM_API(attr_container_get_as_int),
  EXPORT_WASM_API(attr_container_get_as_int64),
  EXPORT_WASM_API(attr_container_get_as_byte),
  EXPORT_WASM_API(attr_container_get_as_uint16),
  EXPORT_WASM_API(attr_container_get_as_float),
  EXPORT_WASM_API(attr_container_get_as_double),
  EXPORT_WASM_API(attr_container_get_as_bool),
  EXPORT_WASM_API(attr_container_get_as_string),
  EXPORT_WASM_API(attr_container_get_as_bytearray),
  EXPORT_WASM_API(attr_container_get_serialize_length),
  EXPORT_WASM_API(attr_container_serialize),
  EXPORT_WASM_API(attr_container_is_constant),
  EXPORT_WASM_API(attr_container_dump),
  EXPORT_WASM_API(request_set_handler),
  EXPORT_WASM_API(response_get_status),
  EXPORT_WASM_API(response_create),
  EXPORT_WASM_API(response_send),
  EXPORT_WASM_API(response_get_payload),
  EXPORT_WASM_API(sensor_open),
  EXPORT_WASM_API(sensor_config),
  EXPORT_WASM_API(sensor_config_with_attr_container),
  EXPORT_WASM_API(sensor_close),
  EXPORT_WASM_API(timer_create),
  EXPORT_WASM_API(timer_set_interval),
  EXPORT_WASM_API(timer_cancel),
  EXPORT_WASM_API(timer_start),
  EXPORT_WASM_API(publish_event)
#endif
};
