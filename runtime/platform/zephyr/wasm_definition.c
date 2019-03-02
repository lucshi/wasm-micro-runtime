/* 
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
*/

#include "wasm_platform.h"

#define RSIZE_MAX 0x7FFFFFFF
int b_memcpy_s(
   void * s1,
   unsigned int s1max,
   const void * s2,
   unsigned int n)
{
   char *dest = (char*)s1;
   char *src = (char*)s2;
   if (n == 0) {
       return 0;
   }

   if (s1 == NULL || s1max > RSIZE_MAX) {
      return -1;
   }
   if (s2 == NULL || n > s1max) {
      memset(dest, 0, s1max);
      return -1;
   }
   memcpy(dest, src, n);
   return 0;
}

int b_strcat_s(char * s1, size_t s1max, const char * s2)
{
	if (NULL == s1 || NULL == s2 || s1max < (strlen(s1) + strlen(s2) + 1) || s1max > RSIZE_MAX) {
		return -1;
	}

	strcat(s1, s2);

	return 0;
}

int b_strcpy_s(char * s1, size_t s1max, const char * s2)
{
	if (NULL == s1 || NULL == s2 || s1max < (strlen(s2) + 1) || s1max > RSIZE_MAX) {
		return -1;
	}

	strcpy(s1, s2);

	return 0;
}

