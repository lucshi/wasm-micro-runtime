#ifndef _BH_PLATFORM_H
#define _BH_PLATFORM_H

#include "bh_config.h"
#include "bh_types.h"

#include <inttypes.h>
#include <stdbool.h>
typedef uint64_t uint64;
typedef int64_t int64;
typedef float float32;
typedef double float64;

#ifndef NULL
#  define NULL ((void*) 0)
#endif

#define BH_PLATFORM "Linux"

#include <stdarg.h>
#include <ctype.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
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

/* Default thread priority */
#define BH_THREAD_DEFAULT_PRIORITY 0

#define INVALID_THREAD_ID 0xFFFFFFFF

typedef pthread_t       korp_tid;
typedef pthread_mutex_t korp_mutex;
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

/* #include <math.h> */

double fmod(double x, double y);
float fmodf(float x, float y);

extern int memcpy_s(void * s1, unsigned int s1max, const void * s2, unsigned int n);
extern int strcpy_s(char * s1, size_t s1max, const char * s2);
#include <stdio.h>
extern int fopen_s(FILE ** pFile, const char *filename, const char *mode);

char*
bh_read_file_to_buffer(const char *filename, int *ret_size);

#ifdef __cplusplus
}
#endif

#endif
