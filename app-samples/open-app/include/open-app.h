#ifndef _LIB_AEE_H_
#define _LIB_AEE_H_

#include "attr-container.h"

#ifdef __cplusplus
extern "C" {
#endif

// Request from host to AEE
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

// Response from AEE to host
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

// Sensor
typedef void *sensor_t;

// Timer
typedef void *user_timer_t;


////////////////////////////////////////////////////////////////
// APIs
////////////////////////////////////////////////////////////////

// Request APIs
void request_set_handler(const char *url, void(*on_request)(request_t *));

// Resposne APIs
int response_get_status(response_t *response);
response_t *response_create(request_t *request, int status);
bool response_send(response_t *response);
attr_container_t* response_get_payload(response_t *response);

// Sensor APIs
sensor_t sensor_open(const char* name, int index, void *user_data, void(*on_sensor_event)(sensor_t, attr_container_t *, void *));
bool sensor_config(sensor_t sensor, int freq, int bit_cfg, int delay);
bool sensor_config_with_attr_container(sensor_t sensor, attr_container_t *cfg);
bool sensor_close(sensor_t sensor);

// Timer APIs
user_timer_t timer_create(int interval, bool is_period, bool auto_start, void *user_data, void(*on_timer_update)(user_timer_t, void *));
void timer_set_interval(user_timer_t timer, int interval);
void timer_cancel(user_timer_t timer);
void timer_start(user_timer_t timer);

// Event APIs
bool publish_event(const char *url, attr_container_t *event);

#ifdef __cplusplus
}
#endif

#endif /* end of _LIB_AEE_H_ */
