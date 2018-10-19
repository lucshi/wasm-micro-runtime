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

#include "ems_gc_internal.h"

#if !defined(NVALGRIND)
#include <valgrind/memcheck.h>
#endif

/* shared heap data structure*/
gc_heap_t shared_heap;

#ifdef __ZEPHYR__
#ifdef CONFIG_BOARD_NUCLEO_F446RE
static char shared_heap_buf[110 * 1024 + GC_HEAD_PADDING] = { 0 };
#else
static char shared_heap_buf[512 * 1024 + GC_HEAD_PADDING] = { 0 };
#endif
#endif


#define HEAP_INC_FACTOR 1

/* Check if current platform is compatible with current GC design*/

/* Return GC_ERROR if not;*/
/* Return GC_SUCCESS otherwise.*/
int gci_check_platform()
{
#define CHECK(x, y)  do { if((x) != (y)) { LOG_ERROR("Platform checking failed on LINE %d at FILE %s.", __LINE__, __FILE__); return GC_ERROR; } } while(0)

	CHECK(8, sizeof(gc_int64));
	CHECK(4, sizeof(gc_uint32));
	CHECK(4, sizeof(gc_int32));
	CHECK(2, sizeof(gc_uint16));
	CHECK(2, sizeof(gc_int16));
	CHECK(1, sizeof(gc_int8));
	CHECK(1, sizeof(gc_uint8));
	CHECK(4, sizeof(gc_size_t));
	CHECK(4, sizeof(void *));

	return GC_SUCCESS;
}



/* Initialize a heap*/

/* @heap can not be NULL*/
/* @heap_max_size can not exceed GC_MAX_HEAP_SIZE and it should not euqal to or smaller than HMU_FC_NORMAL_MAX_SIZE.*/

/* @heap_max_size will be rounded down to page size at first.*/

/* This function will alloc resource for given heap and initalize all data structures.*/

/* Return GC_ERROR if any errors occur.*/
/* Return GC_SUCCESS otherwise.*/
BH_STATIC int init_heap(gc_heap_t *heap, gc_size_t heap_max_size)
{
	void *base_addr = NULL;
	hmu_normal_node_t *p = NULL;
	hmu_tree_node_t *root = NULL, *q = NULL;
	int i = 0;
	int ret = 0;

	bh_assert(heap);

	if(heap_max_size < 1024) {
		LOG_ERROR("[GC_ERROR]heap_init_size(%d) < 1024 ", heap_max_size);
		return GC_ERROR;
	}

	memset(heap, 0, sizeof *heap);

	ret = gct_vm_mutex_init(&heap->lock);
	if (ret != BHT_OK) {
		LOG_ERROR("[GC_ERROR]failed to init lock ");
		return GC_ERROR;
	}
		
#ifdef __ZEPHYR__
	if (heap == &shared_heap) {
		heap_max_size = sizeof(shared_heap_buf) - GC_HEAD_PADDING;
		base_addr = shared_heap_buf + GC_HEAD_PADDING;
	}
	else {
#endif

	heap_max_size = (heap_max_size + 7) & ~(unsigned int)7;

	/* alloc memory for this heap*/
	base_addr = bh_malloc(heap_max_size + GC_HEAD_PADDING);
	if(!base_addr)
	{
		LOG_ERROR("[GC_ERROR]reserve heap with size(%u) failed", heap_max_size);
		(void) gct_vm_mutex_destroy(&heap->lock);
		return GC_ERROR;
	}

	base_addr = (char*) base_addr + GC_HEAD_PADDING;

#ifdef __ZEPHYR__
	}
#endif

#ifdef BH_FOOTPRINT
	printf("\nINIT HEAP 0x%08x %d\n", base_addr, heap_max_size);
#endif

	bh_assert(((int) base_addr & 7) == 4);

	/* init all data structures*/
	heap->max_size = heap_max_size;
	heap->current_size = heap_max_size;
	heap->base_addr = (gc_uint8*)base_addr;
	heap->heap_id = (gc_handle_t)heap;

#if GC_STAT_DATA != 0
	heap->total_free_size = heap->current_size;
	heap->highmark_size = 0;
	heap->total_gc_count = 0;
	heap->total_gc_time = 0;
	heap->gc_threshold_factor = GC_DEFAULT_THRESHOLD_FACTOR;
	gc_update_threshold(heap);
#endif

	for(i = 0; i < HMU_NORMAL_NODE_CNT;i++)
	{
		/* make normal node look like a FC*/
		p = &heap->kfc_normal_list[i];
		memset(p, 0, sizeof *p);
		hmu_set_ut(&p->hmu_header, HMU_FC);
		hmu_set_size(&p->hmu_header, sizeof *p);
	}

	root = &heap->kfc_tree_root;
	memset(root, 0, sizeof *root);
	root->size = sizeof *root;
	hmu_set_ut(&root->hmu_header, HMU_FC);
	hmu_set_size(&root->hmu_header, sizeof *root);

	q = (hmu_tree_node_t *)heap->base_addr;
	memset(q, 0, sizeof *q);
	hmu_set_ut(&q->hmu_header, HMU_FC);
	hmu_set_size(&q->hmu_header, heap->current_size);

	hmu_mark_pinuse(&q->hmu_header);
	root->right = q;
	q->parent = root;
	q->size = heap->current_size;

/* #if !defined(NVALGRIND) */
/* 	VALGRIND_MAKE_MEM_NOACCESS (base_addr, heap_max_size); */
/* #endif */

	bh_assert(root->size <= HMU_FC_NORMAL_MAX_SIZE && HMU_FC_NORMAL_MAX_SIZE < q->size); /*@NOTIFY*/

#if BEIHAI_ENABLE_MEMORY_PROFILING != 0
	LOG_INFO_APP_DEV("heap is successfully initialized with max_size=%u.", heap_max_size);
#endif
	return GC_SUCCESS;
}

/* check ems_gc.h for description*/
gc_handle_t gc_init(gc_size_t heap_max_size)
{
	if(heap_max_size <= 0)
	  heap_max_size = GC_MAX_HEAP_SIZE;

	if(shared_heap.base_addr)
	{
		LOG_ERROR("Duplicated shared heap initialization");
		return NULL;
	}

	/* check system compatibility*/
	if(gci_check_platform() == GC_ERROR)
	{
		LOG_ERROR("Check platform compatibility failed");
		return NULL;
	}

#ifndef GC_NATIVE_SHARE_HEAP
	/* forward parameter checking to function init_heap*/
	if (init_heap(&shared_heap, heap_max_size) == GC_ERROR)
		return NULL;
#endif
	shared_heap.is_shared_heap = 1;

	return (gc_handle_t)&shared_heap;
}

int gc_destroy(gc_handle_t heap)
{
#ifndef GC_NATIVE_SHARE_HEAP
	if ((gc_heap_t*)heap != &shared_heap)
		return GC_ERROR;

#ifndef __ZEPHYR__
	bh_free ((gc_uint8*)shared_heap.base_addr - GC_HEAD_PADDING);
#else
    memset (&shared_heap_buf, 0, sizeof(shared_heap_buf));
#endif
	gct_vm_mutex_destroy(&shared_heap.lock);
	memset(&shared_heap, 0, sizeof (shared_heap));
#endif

	return GC_SUCCESS;
}

/* check ems_gc.h for description*/
gc_handle_t gc_init_for_instance(gc_size_t instance_heap_max_size)
{
	gc_heap_t *heap;
	int ret;

	LOG_VERBOSE("# gc_init_for_instance begin");
	gc_print_stat(0, 0);

#if defined(GC_VERIFY) && !defined(GC_NATIVE_SHARE_HEAP)
	gci_verify_heap(&shared_heap);
#endif
	heap = (gc_heap_t*)gc_alloc_vo(sizeof *heap, MMT_SHARED);
	if(!heap)
	{
		LOG_ERROR("Can not allocate space for private heap metadata.");
		return NULL;
	}

	ret = init_heap(heap, instance_heap_max_size);
	if(ret == GC_ERROR)
	{
		/* rollback*/
		gc_free((gc_object_t)heap);
		return NULL;
	}

	heap->is_shared_heap = 0;
	/* lock is not required*/

	LOG_VERBOSE("# gc_init_for_instance %x %d", heap, instance_heap_max_size);
	gc_print_stat(0, 0);	

	return (gc_handle_t)heap;
}

/* check ems_gc.h for description*/
int gc_destroy_for_instance(gc_handle_t instance_heap)
{
	gc_heap_t *heap = (gc_heap_t*)instance_heap;
        
	LOG_VERBOSE("# gc_destroy_for_instance begin");
	gc_print_stat(0, 0);

	if(!gci_is_heap_valid(heap) || heap->is_shared_heap)
	{
		LOG_ERROR("gc_destroy_instance_heap with incorrect private heap");
		return GC_ERROR;
	}

#ifdef STAT_SHOW_GC
	gc_show_stat (heap);
	gc_show_fragment (heap);
#endif

	heap->base_addr = (gc_uint8 *) heap->base_addr - GC_HEAD_PADDING;
	bh_free(heap->base_addr);

	gct_vm_mutex_destroy(&heap->lock);

	memset(heap, 0, sizeof *heap);
	gc_free(heap);

	LOG_VERBOSE("# gc_destroy_for_instance end");
	gc_print_stat(0, 0);

	return GC_SUCCESS;
}

#if GC_STAT_DATA != 0
/**
 * Set GC threshold factor
 *
 * @param heap [in] the heap to set
 * @param factor [in] the threshold size is free_size * factor / 1000
 *
 * @return GC_SUCCESS if success.
 */
int gc_set_threshold_factor(void *instance_heap, unsigned int factor)
{
	gc_heap_t *heap = (gc_heap_t*)instance_heap;

	if(!gci_is_heap_valid(heap) || heap->is_shared_heap)
	{
		LOG_ERROR("gc_destroy_instance_heap with incorrect private heap");
		return GC_ERROR;
	}

	heap->gc_threshold_factor = factor;
	gc_update_threshold(heap);
	return GC_SUCCESS;
}

#endif


#if defined(GC_VERIFY)
/* Verify heap integrity*/
/* @heap should not be NULL and it should be a valid heap*/
void gci_verify_heap(gc_heap_t *heap)
{
	hmu_t *cur = NULL, *end = NULL;

	bh_assert(heap && gci_is_heap_valid(heap));
	cur = (hmu_t *)heap->base_addr;
	end = (hmu_t *)(heap->base_addr + heap->current_size);
	while(cur < end)
	{
		hmu_verify(cur);
		cur = (hmu_t *)((gc_uint8*)cur + hmu_get_size(cur));
	}
	bh_assert(cur == end);
}
#endif

