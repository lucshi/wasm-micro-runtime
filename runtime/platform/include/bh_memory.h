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

#ifndef _BH_MEMORY_H
#define _BH_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bh_config.h"

/** 
 * This function allocates a memory chunk from system
 * 
 * @param size bytes need allocate
 * 
 * @return the pointer to memory allocated
 */
void *bh_malloc(unsigned int size);


/** 
 * This function frees memory chunk
 * 
 * @param ptr the pointer to memory need free
 */
void bh_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* end of _BH_MEMORY_H */

