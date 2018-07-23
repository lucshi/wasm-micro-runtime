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

#ifndef _BH_VECTOR_H
#define _BH_VECTOR_H

#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vector {
  /* size of each element */
  uint32 size_elem;
  /* max element number */
  uint32 max_elements;
  /* current element num */
  uint32 num_elements;
  /* vector data allocated */
  uint8 *data;
} Vector;

/**
 * Initialize vector
 *
 * @param vector the vector to init
 * @param init_length the initial length of the vector
 * @param size_elem size of each element
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_init(Vector *vector, uint32 init_length, uint32 size_elem);

/**
 * Set element of vector
 *
 * @param vector the vector to set
 * @param index the index of the element to set
 * @param elem_buf the element buffer which stores the element data
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_set(Vector *vector, uint32 index, const void *elem_buf);

/**
 * Get element of vector
 *
 * @param vector the vector to get
 * @param index the index of the element to get
 * @param elem_buf the element buffer to store the element data,
 *                 whose length must be no less than element size
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_get(const Vector *vector, uint32 index, void *elem_buf);

/**
 * Insert element of vector
 *
 * @param vector the vector to insert
 * @param index the index of the element to insert
 * @param elem_buf the element buffer which stores the element data
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_insert(Vector *vector, uint32 index, const void *elem_buf);

/**
 * Append element to the end of vector
 *
 * @param vector the vector to append
 * @param elem_buf the element buffer which stores the element data
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_append(Vector *vector, const void *elem_buf);

/**
 * Remove element from vector
 *
 * @param vector the vector to remove element
 * @param index the index of the element to remove
 * @param old_elem_buf if not NULL, copies the element data to the buffer
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_remove(Vector *vector, uint32 index, void *old_elem_buf);

/**
 * Return the size of the vector
 *
 * @param vector the vector to get size
 *
 * @return return the size of the vector
 */
uint32
bh_vector_size(const Vector *vector);

/**
 * Destroy the vector
 *
 * @param vector the vector to destroy
 *
 * @return true if success, false otherwise
 */
bool
bh_vector_destroy(Vector *vector);

#ifdef __cplusplus
}
#endif

#endif /* endof _BH_VECTOR_H */

