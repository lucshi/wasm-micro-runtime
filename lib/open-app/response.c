#include "attr-container.h"
#include "wasm_types.h"
#include "response.h"

int response_get_status(response_t *response)
{
  printf("response_get_status called\n");
  //TODO
  return 0;
}

response_t *
response_create(request_t *request, int status)
{
  printf("response_create called\n");
  //TODO
  return NULL;
}

bool
response_send(response_t *response)
{
  printf("response_send called\n");
  //TODO
  return false;
}

attr_container_t*
response_get_payload(response_t *response)
{
  printf("response_get_payload called\n");
  //TODO
  return NULL;
}

