#include "attr-container.h"
#include "wasm_types.h"
#include "sensor.h"

sensor_t
sensor_open(const char* name, int index, void *user_data, void(*on_sensor_event)(sensor_t, attr_container_t *, void *))
{
  printf("sensor_open called\n");
  //TODO
  return NULL;
}

bool
sensor_config(sensor_t sensor, int freq, int bit_cfg, int delay)
{
  printf("sensor_config called\n");
  //TODO
  return false;
}

bool
sensor_config_with_attr_container(sensor_t sensor, attr_container_t *cfg)
{
  printf("sensor_config_with_attr_container called\n");
  //TODO
  return false;
}

bool
sensor_close(sensor_t sensor)
{
  printf("sensor_close called\n");
  //TODO
  return false;
}
