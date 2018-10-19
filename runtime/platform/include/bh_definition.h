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
#ifndef _BH_DEFINITION_H
#define _BH_DEFINITION_H

#include "bh_config.h"

typedef enum {
    BH_FAILED = -100,
    BH_UNKOWN = -99,
    BH_MAGIC_UNMATCH = -12,
    BH_UNIMPLEMENTED = -11,
    BH_INTR = -10,
    BH_CLOSED = -9,
    BH_BUFFER_OVERFLOW = -8,  /* TODO: no used error, should remove*/
    BH_NOT_SUPPORTED = -7,
    BH_WEAR_OUT_VIOLATION = -6,
    BH_NOT_FOUND = -5,
    BH_INVALID_PARAMS = -4,
    BH_ACCESS_DENIED = -3,
    BH_OUT_OF_MEMORY = -2,
    BH_INVALID = -1,
    BH_SUCCESS = 0,
    BH_TIMEOUT = 2
} bh_status;


#endif
