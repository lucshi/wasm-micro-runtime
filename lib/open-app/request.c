#include "attr-container.h"
#include "wasm_types.h"
#include "request.h"

void
request_set_handler(const char *url, void(*on_request)(request_t *))
{
  printf("request_set_handler called\n");
  //TODO
}
