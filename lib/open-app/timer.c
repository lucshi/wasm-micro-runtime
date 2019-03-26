#include "attr-container.h"
#include "wasm_types.h"
#include "timer.h"

user_timer_t
timer_create(int interval, bool is_period, bool auto_start, void *user_data, void(*on_timer_update)(user_timer_t, void *))
{
  printf("timer_create called\n");
  //TODO
  return NULL;
}

void
timer_set_interval(user_timer_t timer, int interval)
{
  printf("timer_set_interval called\n");
  //TODO
}

void
timer_cancel(user_timer_t timer)
{
  printf("timer_cancel called\n");
  //TODO
}

void
timer_start(user_timer_t timer)
{
  printf("timer_start called\n");
  //TODO
}

