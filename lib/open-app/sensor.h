#ifndef _AEE_SENSOR_H_
#define _AEE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

//TODO:
#define bh_queue_t void

/* board producer define sensor */
struct sensor;
typedef struct sensor *sensor_t;

typedef struct sensor {
  char *name;
  int handle;
  int freq;
  int bit_cfg; 
  int delay;
  void (*sensor_callback)(sensor_t, attr_container_t);
  bh_queue_t *queue;
} sensor;

// Sensor APIs
sensor_t sensor_open(const char* name, int index, void *user_data, void(*on_sensor_event)(sensor_t, attr_container_t *, void *));
bool sensor_config(sensor_t sensor, int freq, int bit_cfg, int delay);
bool sensor_config_with_attr_container(sensor_t sensor, attr_container_t *cfg);
bool sensor_close(sensor_t sensor);

#ifdef __cplusplus
}
#endif

#endif
