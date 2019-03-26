#include <stdio.h>
#include <stdlib.h>

#include "open-app.h"

void on_init()
{
  attr_container_t *attr;

  request_set_handler(NULL, NULL);
  response_get_status(NULL);
  response_create(NULL, 0);
  response_send(NULL);
  response_get_payload(NULL);
  sensor_open(NULL, 0, NULL, NULL);
  sensor_config(NULL, 0, 0, 0);
  sensor_config_with_attr_container(NULL, NULL);
  sensor_close(NULL);

  timer_create(0, 0, 0, NULL, NULL);
  timer_set_interval(NULL, 0);
  timer_cancel(NULL);
  timer_start(NULL);

  /*
  attr = attr_container_create("attr");
  attr_container_destroy(attr);
  */
}
