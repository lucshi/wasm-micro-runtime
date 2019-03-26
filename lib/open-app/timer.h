#ifndef _AEE_TIMER_H_
#define _AEE_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

//TODO:
#define bh_queue_t void

/* board producer define user_timer */
struct user_timer;
typedef struct user_timer *user_timer_t;

typedef struct user_timer {
  int handle;
  int interval;
  bool is_periodic;
  bool is_running;
  void (*user_timer_callback)(user_timer_t);
  bh_queue_t *queue;
} user_timer;

// Timer APIs
user_timer_t timer_create(int interval, bool is_period, bool auto_start, void *user_data, void(*on_user_timer_update)(user_timer_t, void *));
void timer_set_interval(user_timer_t timer, int interval);
void timer_cancel(user_timer_t timer);
void timer_start(user_timer_t timer);

#ifdef __cplusplus
}
#endif

#endif
