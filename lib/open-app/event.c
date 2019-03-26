#include "attr-container.h"
#include "wasm_types.h"
#include "event.h"

bool
publish_event(const char *url, attr_container_t *event)
{
  (void)url;
  (void)event;

  printf("publish_event called\n");

  return false;
}
