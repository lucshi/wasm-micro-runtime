#ifndef _AEE_REQUEST_H_
#define _AEE_REQUEST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct request {
  // message id
  int mid;

  // url of the request
  const char *url;

  // action of the request, can be PUT/GET/POST/DELETE
  int action;

  // payload format, currently only support attr_container_t type
  int fmt;

  // payload of the request, currently only support attr_container_t type
  void *payload;
} request_t;


// Request APIs
void request_set_handler(const char *url, void(*on_request)(request_t *));

#ifdef __cplusplus
}
#endif

#endif
