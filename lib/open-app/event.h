#ifndef _AEE_EVENT_H_
#define _AEE_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Event APIs
bool publish_event(const char *url, attr_container_t *event);

#ifdef __cplusplus
}
#endif

#endif
