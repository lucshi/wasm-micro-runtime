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

#include "bh_hashmap.h"
#include "bh_thread.h"
#include "bh_memory.h"


typedef struct HashMapElem {
  void *key;
  void *value;
  struct HashMapElem *next;
} HashMapElem;

struct HashMap {
  /* size of element array */
  uint32 size;
  /* lock for elements */
  korp_mutex *lock;
  /* hash function of key */
  HashFunc hash_func;
  /* key equal function */
  KeyEqualFunc key_equal_func;
  KeyDestroyFunc key_destroy_func;
  ValueDestroyFunc value_destroy_func;
  HashMapElem *elements[1];
};

HashMap*
bh_hash_map_create(uint32 size, bool use_lock,
                   HashFunc hash_func,
                   KeyEqualFunc key_equal_func,
                   KeyDestroyFunc key_destroy_func,
                   ValueDestroyFunc value_destroy_func)
{
  HashMap *map;
  uint32 total_size;

  if (size > HASH_MAP_MAX_SIZE) {
    printf("HashMap create failed: size is too large.\n");
    return NULL;
  }

  if (!hash_func || !key_equal_func) {
    printf("HashMap create failed: hash function or key equal function "
           " is NULL.\n");
    return NULL;
  }

  total_size = offsetof(HashMap, elements) +
               sizeof(HashMapElem) * size +
               (use_lock ? sizeof(korp_mutex) : 0);

  if (!(map = bh_malloc(total_size))) {
    printf("HashMap create failed: alloc memory failed.\n");
    return NULL;
  }

  memset(map, 0, total_size);

  if (use_lock) {
    map->lock = (korp_mutex*)
      ((uint8*)map + offsetof(HashMap, elements) + sizeof(HashMapElem) * size);
    if (vm_mutex_init(map->lock, false)) {
      printf("HashMap create failed: init map lock failed.\n");
      bh_free(map);
      return NULL;
    }
  }

  map->size = size;
  map->hash_func = hash_func;
  map->key_equal_func = key_equal_func;
  map->key_destroy_func = key_destroy_func;
  map->value_destroy_func = value_destroy_func;
  return map;
}

bool
bh_hash_map_insert(HashMap *map, void *key, void *value)
{
  uint32 index;
  HashMapElem *elem;

  if (!map || !key) {
    printf("HashMap insert elem failed: map or key is NULL.\n");
    return false;
  }

  if (map->lock) {
    vm_mutex_lock(map->lock);
  }

  index = map->hash_func(key) % map->size;
  elem = map->elements[index];
  while (elem) {
    if (map->key_equal_func(elem->key, key)) {
      printf("HashMap insert elem failed: duplicated key found.\n");
      goto fail;
    }
    elem = elem->next;
  }

  if (!(elem = bh_malloc(sizeof(HashMapElem)))) {
    printf("HashMap insert elem failed: alloc memory failed.\n");
    goto fail;
  }

  elem->key = key;
  elem->value = value;
  elem->next = map->elements[index];
  map->elements[index] = elem;

  if (map->lock) {
    vm_mutex_unlock(map->lock);
  }
  return true;

fail:
  if (map->lock) {
    vm_mutex_unlock(map->lock);
  }
  return false;
}

void*
bh_hash_map_find(HashMap *map, void *key)
{
  uint32 index;
  HashMapElem *elem;
  void *value;

  if (!map || !key) {
    printf("HashMap find elem failed: map or key is NULL.\n");
    return NULL;
  }

  if (map->lock) {
    vm_mutex_lock(map->lock);
  }

  index = map->hash_func(key) % map->size;
  elem = map->elements[index];

  while (elem) {
    if (map->key_equal_func(elem->key, key)) {
      value = elem->value;
      if (map->lock) {
        vm_mutex_unlock(map->lock);
      }
      return value;
    }
    elem = elem->next;
  }

  if (map->lock) {
    vm_mutex_unlock(map->lock);
  }
  return NULL;
}

bool
bh_hash_map_update(HashMap *map, void *key, void *value,
                   void **p_old_value)
{
  uint32 index;
  HashMapElem *elem;

  if (!map || !key) {
    printf("HashMap update elem failed: map or key is NULL.\n");
    return false;
  }

  if (map->lock) {
    vm_mutex_lock(map->lock);
  }

  index = map->hash_func(key) % map->size;
  elem = map->elements[index];

  while (elem) {
    if (map->key_equal_func(elem->key, key)) {
      if (p_old_value)
        *p_old_value = elem->value;
      elem->value = value;
      if (map->lock) {
        vm_mutex_unlock(map->lock);
      }
      return true;
    }
    elem = elem->next;
  }

  if (map->lock) {
    vm_mutex_unlock(map->lock);
  }
  return false;
}

bool
bh_hash_map_remove(HashMap *map, void *key,
                   void **p_old_key, void **p_old_value)
{
  uint32 index;
  HashMapElem *elem, *prev;

  if (!map || !key) {
    printf("HashMap remove elem failed: map or key is NULL.\n");
    return false;
  }

  if (map->lock) {
    vm_mutex_lock(map->lock);
  }

  index = map->hash_func(key) % map->size;
  prev = elem = map->elements[index];

  while (elem) {
    if (map->key_equal_func(elem->key, key)) {
      if (p_old_key)
        *p_old_key = elem->key;
      if (p_old_value)
        *p_old_value = elem->value;

      if (elem == map->elements[index])
        map->elements[index] = elem->next;
      else
        prev->next = elem->next;

      bh_free(elem);

      if (map->lock) {
        vm_mutex_unlock(map->lock);
      }
      return true;
    }

    prev = elem;
    elem = elem->next;
  }

  if (map->lock) {
    vm_mutex_unlock(map->lock);
  }
  return false;
}

bool
bh_hash_map_destroy(HashMap *map)
{
  uint32 index;
  HashMapElem *elem, *next;

  if (!map) {
    printf("HashMap destroy failed: map is NULL.\n");
    return false;
  }

  if (map->lock) {
    vm_mutex_lock(map->lock);
  }

  for (index = 0; index < map->size; index++) {
    elem = map->elements[index];
    while (elem) {
      next = elem->next;

      if (map->key_destroy_func) {
        map->key_destroy_func(elem->key);
      }
      if (map->value_destroy_func) {
        map->value_destroy_func(elem->value);
      }
      bh_free(elem);

      elem = next;
    }
  }

  if (map->lock) {
    vm_mutex_unlock(map->lock);
    vm_mutex_destroy(map->lock);
  }
  bh_free(map);
  return true;
}
