#ifndef _AEE_RESPONSE_H_
#define _AEE_RESPONSE_H_

#include "request.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct response {
  // message id
  int mid;

  // status of the response
  int status;

  // payload format, currently only support attr_container_t type
  int fmt;

  // payload of the response, currently only support attr_container_t type
  void *payload;
} response_t;


// Resposne APIs
int response_get_status(response_t *response);
response_t *response_create(request_t *request, int status);
bool response_send(response_t *response);
attr_container_t *response_get_payload(response_t *response);

#ifdef __cplusplus
}
#endif

#endif
