#ifndef _BH_PLATFORM_H
#define _BH_PLATFORM_H

#include "bh_config.h"
#include "bh_types.h"
#include <zephyr.h>
#include <kernel.h>
#include <inttypes.h>

#include <stdbool.h>
typedef uint64_t uint64;
typedef int64_t int64;
typedef float float32;
typedef double float64;

#ifndef NULL
#  define NULL ((void*) 0)
#endif

#define BH_PLATFORM "Zephyr"

#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Return the offset of the given field in the given type.
 *
 * @param Type the type containing the filed
 * @param field the field in the type
 *
 * @return the offset of field in Type
 */
#ifndef offsetof
#define offsetof(Type, field) ((size_t)(&((Type *)0)->field))
#endif

#define _STACK_SIZE_ADJUSTMENT (0 * 1024)

/* Stack size of applet threads's native part.  */
#define BH_APPLET_PRESERVED_NATIVE_STACK_SIZE \
    (4 * 1024 + _STACK_SIZE_ADJUSTMENT)

/* Stack size of applet threads's interpreter part.  */
#define BH_APPLET_PRESERVED_WASM_STACK_SIZE (0 * 1024)

/* Default thread priority */
#define BH_THREAD_DEFAULT_PRIORITY 7

#define BH_ROUTINE_MODIFIER

#define INVALID_THREAD_ID 0xFFFFFFFF

typedef struct k_thread korp_thread;
typedef korp_thread *korp_tid;
typedef struct k_mutex korp_mutex;
typedef struct k_sem korp_sem;

struct bh_thread_wait_node;
typedef struct bh_thread_wait_node *bh_thread_wait_list;
typedef struct korp_cond {
  struct k_mutex wait_list_lock;
  bh_thread_wait_list thread_wait_list;
} korp_cond;

typedef void* (*thread_start_routine_t)(void*);

#include <string.h>

/* The following operations declared in string.h may be defined as
   macros on Linux, so don't declare them as functions here.  */
/* memset */
/* memcpy */
/* memmove */

/* #include <stdio.h> */

/* Unit test framework is based on C++, where the declaration of
   snprintf is different.  */
#ifndef __cplusplus
int snprintf(char *buffer, size_t count, const char *format, ...);
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int memcpy_s(void * s1, unsigned int s1max, const void * s2, unsigned int n);
extern int strcpy_s(char * s1, size_t s1max, const char * s2);

/* math functions */
double sqrt(double x);
double floor(double x);
double ceil(double x);
double fmin(double x, double y);
double fmax(double x, double y);
double rint(double x);
double fabs(double x);
double trunc(double x);
int signbit(double x);
int isnan(double x);

void*
bh_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif
