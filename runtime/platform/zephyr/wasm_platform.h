#ifndef _WASM_PLATFORM_H
#define _WASM_PLATFORM_H

#include "wasm_config.h"
#include "wasm_types.h"
#include <autoconf.h>
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

#define WASM_PLATFORM "Zephyr"

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

typedef struct k_thread korp_thread;
typedef korp_thread *korp_tid;
typedef struct k_mutex korp_mutex;

int wasm_platform_init();

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
wasm_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif
