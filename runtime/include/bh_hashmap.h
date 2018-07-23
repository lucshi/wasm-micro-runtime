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

#ifndef _BH_HASHMAP_H
#define _BH_HASHMAP_H

#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Maximum initial size of hash map */
#define HASH_MAP_MAX_SIZE 65536

struct HashMap;
typedef struct HashMap HashMap;

/* Hash function: to get the hash value of key. */
typedef uint32 (*HashFunc)(const void *key);

/* Key equal function: to check whether two keys are equal. */
typedef bool (*KeyEqualFunc)(void *key1, void *key2);

/* Key destroy function: to destroy the key, auto called
   when an hash element is removed. */
typedef void (*KeyDestroyFunc)(void *key);

/* Value destroy function: to destroy the value, auto called
   when an hash element is removed. */
typedef void (*ValueDestroyFunc)(void *key);

/**
 * Create a hash map.
 *
 * @param size: the initial size of the hash map
 * @param use_lock whether to lock the hash map when operating on it
 * @param hash_func hash function of the key, must be specified
 * @param key_equal_func key equal function, check whether two keys
 *                       are equal, must be specified
 * @param key_destroy_func key destroy function, called when an hash element
 *                         is removed if it is not NULL
 * @param value_destroy_func value destroy function, called when an hash
 *                           element is removed if it is not NULL
 *
 * @return the hash map created, NULL if failed
 */
HashMap*
bh_hash_map_create(uint32 size, bool use_lock,
                   HashFunc hash_func,
                   KeyEqualFunc key_equal_func,
                   KeyDestroyFunc key_destroy_func,
                   ValueDestroyFunc value_destroy_func);

/**
 * Insert an element to the hash map
 *
 * @param map the hash map to insert element
 * @key the key of the element
 * @value the value of the element
 *
 * @return true if success, false otherwise
 * Note: fail if key is NULL or duplicated key exists in the hash map,
 */
bool
bh_hash_map_insert(HashMap *map, void *key, void *value);

/**
 * Find an element in the hash map
 *
 * @param map the hash map to find element
 * @key the key of the element
 *
 * @return the value of the found element if success, NULL otherwise
 */
void*
bh_hash_map_find(HashMap *map, void *key);

/**
 * Update an element in the hash map with new value
 *
 * @param map the hash map to update element
 * @key the key of the element
 * @value the new value of the element
 * @p_old_value if not NULL, copies the old value to it
 *
 * @return true if success, false otherwise
 * Note: the old value won't be destroyed by value destory function,
 *       it will be copied to p_old_value for user to process.
 */
bool
bh_hash_map_update(HashMap *map, void *key, void *value,
                   void **p_old_value);

/**
 * Remove an element from the hash map
 *
 * @param map the hash map to remove element
 * @key the key of the element
 * @p_old_key if not NULL, copies the old key to it
 * @p_old_value if not NULL, copies the old value to it
 *
 * @return true if success, false otherwise
 * Note: the old key and old value won't be destroyed by key destroy
 *       function and value destroy function, they will be copied to
 *       p_old_key and p_old_value for user to process.
 */
bool
bh_hash_map_remove(HashMap *map, void *key,
                   void **p_old_key, void **p_old_value);

/**
 * Destroy the hashmap
 *
 * @param map the hash map to destroy
 *
 * @return true if success, false otherwise
 * Note: the key destroy function and value destroy function will be
 *       called to destroy each element's key and value if they are
 *       not NULL.
 */
bool
bh_hash_map_destroy(HashMap *map);

#ifdef __cplusplus
}
#endif

#endif /* endof _BH_HASHMAP_H */

