/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010, 2011 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors. Title to the Material
 * remains with Intel Corporation or its suppliers and licensors. The
 * Material contains trade secrets and proprietary and confidential
 * information of Intel or its suppliers and licensors. The Material
 * is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way without Intel's prior express
 * written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 */

#include <stdlib.h>
#include "bh_memory.h"

void* __real_malloc(size_t size);
void* __real_calloc(size_t nmemb, size_t size);
void __real_free(void *ptr);

void *
__wrap_malloc(size_t size)
{
  return __real_malloc(size);
}

void *
__wrap_calloc(size_t nmemb, size_t size)
{
  return __real_calloc(nmemb, size);
}

void
__wrap_free(void *ptr)
{
  __real_free(ptr);
}

void*
bh_malloc(unsigned int size)
{
  return __real_malloc(size);
}

void bh_free(void *ptr)
{
  if (ptr)
    __real_free(ptr);
}

