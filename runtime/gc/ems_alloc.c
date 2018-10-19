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



static int hmu_is_in_heap(gc_heap_t* heap, hmu_t* hmu)
{
	return heap && hmu && (gc_uint8*)hmu >= heap->base_addr && (gc_uint8*)hmu < heap->base_addr + heap->current_size;
}

/* Remove a node from the tree it belongs to*/

/* @p can not be NULL*/
/* @p can not be the ROOT node*/

/* Node @p will be removed from the tree and left,right,parent pointers of node @p will be*/
/*  set to be NULL. Other fields will not be touched.*/
/* The tree will be re-organized so that the order conditions are still satisified.*/
BH_STATIC void remove_tree_node(hmu_tree_node_t *p)
{
	hmu_tree_node_t *q = NULL, **slot = NULL;

	bh_assert(p);
	bh_assert(p->parent); /* @p can not be the ROOT node*/

	/* get the slot which holds pointer to node p*/
	if(p == p->parent->right)
	{
		slot = &p->parent->right;
	}
	else
	{
		bh_assert(p == p->parent->left); /* @p should be a child of its parent*/
		slot = &p->parent->left;
	}

	/* algorithms used to remove node p*/
	/* case 1: if p has no left child, replace p with its right child*/
	/* case 2: if p has no right child, replace p with its left child*/
	/* case 3: otherwise, find p's predecessor, remove it from the tree and replace p with it.*/
	/*         use predecessor can keep the left <= root < right condition.*/

	if(!p->left)
	{
		/* move right child up*/
		*slot = p->right;
		if(p->right) p->right->parent = p->parent;

		p->left = p->right = p->parent = NULL;
		return;
	}

	if(!p->right)
	{
		/* move left child up*/
		*slot = p->left;
		p->left->parent = p->parent; /* p->left can never be NULL.*/

		p->left = p->right = p->parent = NULL;
		return;
	}

	/* both left & right exist, find p's predecessor at first*/
	q = p->left;
	while(q->right) q = q->right;
	remove_tree_node(q); /* remove from the tree*/

	*slot = q;
	q->parent = p->parent;
	q->left = p->left;
	q->right = p->right;
	if(q->left) q->left->parent = q;
	if(q->right) q->right->parent = q;

	p->left = p->right = p->parent = NULL;
}

static void unlink_hmu(gc_heap_t *heap, hmu_t *hmu) {
	gc_size_t size;

	bh_assert(gci_is_heap_valid(heap));
	bh_assert(hmu && (gc_uint8*)hmu >= heap->base_addr && (gc_uint8*)hmu < heap->base_addr + heap->current_size);
	bh_assert(hmu_get_ut(hmu) == HMU_FC);

	size = hmu_get_size(hmu);

	if(HMU_IS_FC_NORMAL(size)) {
		int node_idx = size >> 3;
		hmu_normal_node_t* node = heap->kfc_normal_list[node_idx].next;
		hmu_normal_node_t** p = &(heap->kfc_normal_list[node_idx].next);
		while(node) {
			if((hmu_t*) node == hmu) {
				*p = node->next;
				break;
			}
			p = &(node->next);
			node = node->next;
		}

		if(!node) {
			LOG_ERROR("[GC_ERROR]couldn't find the node in the normal list");
		}
	} else {
		remove_tree_node((hmu_tree_node_t *)hmu);
	}
}

static void hmu_set_free_size(hmu_t *hmu)
{
	gc_size_t size;
	bh_assert(hmu && hmu_get_ut(hmu) == HMU_FC);

	size = hmu_get_size(hmu);
	*((int*)((char*)hmu + size) - 1 ) = size;
}

/* Add free chunk back to KFC*/

/* @heap should not be NULL and it should be a valid heap*/
/* @hmu should not be NULL and it should be a HMU of length @size inside @heap*/
/* @hmu should be aligned to 8*/
/* @size should be positive and multiple of 8*/

/* @hmu with size @size will be added into KFC as a new FC.*/
void gci_add_fc(gc_heap_t *heap, hmu_t *hmu, gc_size_t size)
{
	hmu_normal_node_t *np = NULL;
	hmu_tree_node_t *root = NULL, *tp = NULL, *node=NULL;
	int node_idx;

	bh_assert(gci_is_heap_valid(heap));
	bh_assert(hmu && (gc_uint8*)hmu >= heap->base_addr && (gc_uint8*)hmu < heap->base_addr + heap->current_size);
	bh_assert(((gc_uint32)hmu_to_obj(hmu) & 7) == 0);
	bh_assert(size > 0 && ((gc_uint8*)hmu) + size <= heap->base_addr + heap->current_size);
	bh_assert(!(size & 7));

	hmu_set_ut(hmu, HMU_FC);
	hmu_set_size(hmu, size);
	hmu_set_free_size(hmu);

	if(HMU_IS_FC_NORMAL(size))
	{
		np = (hmu_normal_node_t*)hmu;

		node_idx = size >> 3;
		np->next = heap->kfc_normal_list[node_idx].next;
		heap->kfc_normal_list[node_idx].next = np;
		return;
	}

	/* big block*/
	node = (hmu_tree_node_t*)hmu;
	node->size = size;
	node->left = node->right = node->parent = NULL;

	/* find proper node to link this new node to*/
	root =  &heap->kfc_tree_root;
	tp = root;
	bh_assert(tp->size < size);
	while(1)
	{
		if(tp->size < size)
		{
			if(!tp->right)
			{
				tp->right = node;
				node->parent = tp;
				break;
			}
			tp = tp->right;
		}
		else /* tp->size >= size*/
		{
			if(!tp->left)
			{
				tp->left = node;
				node->parent = tp;
				break;
			}
			tp = tp->left;
		}
	}
}


/* Find a proper hmu for required memory size*/

/* @heap should not be NULL and it should be a valid heap*/
/* @size should cover the header and it should be 8 bytes aligned*/

/* GC will not be performed here.*/
/* Heap extension will not be performed here.*/

/* A proper HMU will be returned. This HMU can include the header and given size. The returned HMU will be aligned to 8 bytes.*/
/* NULL will be returned if there are no proper HMU.*/
BH_STATIC hmu_t *alloc_hmu(gc_heap_t *heap, gc_size_t size)
{
	hmu_normal_node_t *node = NULL, *p = NULL;
	int node_idx = 0, init_node_idx = 0;
	hmu_tree_node_t *root = NULL, *tp = NULL, *last_tp = NULL;
	hmu_t *next, *rest;

	bh_assert(gci_is_heap_valid(heap));
	bh_assert(size > 0 && !(size & 7));

	if (size < GC_SMALLEST_SIZE)
		size = GC_SMALLEST_SIZE;

	/* check normal list at first*/
	if (HMU_IS_FC_NORMAL(size))
	{
		/* find a non-empty slot in normal_node_list with good size*/
		init_node_idx = (int)(size >> 3);
		for(node_idx = init_node_idx; node_idx < HMU_NORMAL_NODE_CNT;node_idx++)
		{
			node = heap->kfc_normal_list + node_idx;
			if(node->next) break;
			node = NULL;
		}

		/* not found in normal list*/
		if(node) {
			bh_assert(node_idx >= init_node_idx);

			p = node->next;
			node->next = p->next;
			bh_assert(((gc_int32)hmu_to_obj(p) & 7)==0);

			if((gc_size_t)node_idx != init_node_idx && ((gc_size_t)node_idx << 3) >= size + GC_SMALLEST_SIZE) {  /* with bigger size*/
				rest = (hmu_t*)(((char *)p) + size);
				gci_add_fc( heap, rest, (node_idx << 3) - size);
				hmu_mark_pinuse(rest);
			} else { 
				size = node_idx << 3;
				next = (hmu_t*)((char*)p + size);
				if(hmu_is_in_heap(heap, next))
					hmu_mark_pinuse(next);
			}

#if GC_STAT_DATA != 0
			heap->total_free_size -= size;
			if((heap->current_size - heap->total_free_size) > heap->highmark_size)
				heap->highmark_size = heap->current_size - heap->total_free_size;
#endif

			hmu_set_size((hmu_t*) p, size);
			return (hmu_t*)p;
		}
	}

	/* need to find a node in tree*/
	root = &heap->kfc_tree_root;

	/* find the best node*/
	bh_assert(root);
	tp = root->right;
	while(tp)
	{
		if(tp->size < size)
		{
			tp = tp->right;
			continue;
		}

		/* record the last node with size equal to or bigger than given size*/
		last_tp = tp;
		tp = tp->left;
	}

	if(last_tp)
	{
		bh_assert(last_tp->size >= size);

		/* alloc in last_p*/

		/* remove node last_p from tree*/
		remove_tree_node(last_tp);

		if(last_tp->size >= size + GC_SMALLEST_SIZE) {
			rest = (hmu_t*)((char*)last_tp + size);
			gci_add_fc(heap, rest, last_tp->size - size);
			hmu_mark_pinuse(rest);
		} else {
			size = last_tp->size;
			next = (hmu_t*)((char*)last_tp + size);
			if(hmu_is_in_heap(heap, next))
				hmu_mark_pinuse(next);
		}

#if GC_STAT_DATA != 0
                heap->total_free_size -= size;
		if((heap->current_size - heap->total_free_size) > heap->highmark_size)
			heap->highmark_size = heap->current_size - heap->total_free_size;
#endif
		hmu_set_size((hmu_t*) last_tp, size);
		return (hmu_t*)last_tp;
	}

	return NULL;
}

/* Find a proper HMU for given size*/

/* @heap should not be NULL and it should be a valid heap*/
/* @size should cover the header and it should be 8 bytes aligned*/

/* This function will try several ways to satisfy the allocation request.*/
/*  1. Find a proper on available HMUs.*/
/*  2. GC will be triggered if 1 failed.*/
/*  3. Find a proper on available HMUS.*/
/*  4. Return NULL if 3 failed*/

/* A proper HMU will be returned. This HMU can include the header and given size. The returned HMU will be aligned to 8 bytes.*/
/* NULL will be returned if there are no proper HMU.*/
BH_STATIC hmu_t* alloc_hmu_ex(gc_heap_t *heap, gc_size_t size)
{
	hmu_t *ret = NULL;

	bh_assert(gci_is_heap_valid(heap));
	bh_assert(size > 0 && !(size & 7));

	if(&shared_heap == heap)
		return alloc_hmu(heap, size);

#ifdef GC_IN_EVERY_ALLOCATION
	gci_gc_heap(heap);
	ret = alloc_hmu(heap, size);
#else

# if GC_STAT_DATA != 0
	if (heap->gc_threshold < heap->total_free_size)
		ret = alloc_hmu(heap, size);
# else
	ret = alloc_hmu(heap, size);
# endif

	if(ret)
		return ret;

	/*gci_gc_heap(heap);*//* disable gc claim currently */
	ret = alloc_hmu(heap, size);
#endif
	return ret;
}

gc_object_t _gc_alloc_vo_i_heap(void *vheap, gc_size_t size ALLOC_EXTRA_PARAMETERS)
{
	gc_heap_t* heap = (gc_heap_t*)vheap;
	hmu_t *hmu = NULL;
	gc_object_t ret = (gc_object_t)NULL;
	gc_size_t tot_size = 0;

	/* align size*/
	tot_size = GC_ALIGN_8(size + HMU_SIZE + OBJ_PREFIX_SIZE + OBJ_SUFFIX_SIZE); /* hmu header, prefix, suffix*/
	if(tot_size < size) 
		return NULL;

#ifdef GC_NATIVE_SHARE_HEAP
	if (vheap == NULL || vheap == &shared_heap)
		return bh_malloc (size);
#endif

        /* FIXME: remove this after refactor.  */
	if(heap == NULL)
		heap = &shared_heap;


	gct_vm_mutex_lock(&heap->lock);

	hmu = alloc_hmu_ex(heap, tot_size);
	if(!hmu) 
		goto FINISH;

	hmu_set_ut(hmu, HMU_VO);
	hmu_unfree_vo(hmu);

#if defined(GC_VERIFY)
	hmu_init_prefix_and_suffix(hmu, tot_size, file_name, line_number);
#endif

	ret = hmu_to_obj(hmu);

#if !defined(NVALGRIND)
        if (heap == &shared_heap)
	VALGRIND_MALLOCLIKE_BLOCK (ret, size, 16, 0);	
        /* VALGRIND_PRINTF ("XXX %x %d\n", ret, size); */
#endif

#if BEIHAI_ENABLE_MEMORY_PROFILING != 0
    LOG_PROFILE_HEAP_ALLOC ((unsigned)heap, tot_size);
#endif

FINISH:
	gct_vm_mutex_unlock(&heap->lock);

	return ret;
}

/* see ems_gc.h for description*/
gc_object_t _gc_alloc_vo_i(gc_size_t size, gc_mm_t mmt ALLOC_EXTRA_PARAMETERS)
{
  (void)mmt;
    bh_assert (mmt == MMT_SHARED);
    return gc_alloc_vo_i_heap (&shared_heap, size ALLOC_EXTRA_ARGUMENTS);
}

/* see ems_gc.h for description*/
gc_object_t _gc_alloc_jo_i_heap(void *vheap, gc_size_t size ALLOC_EXTRA_PARAMETERS)
{
	gc_heap_t* heap = (gc_heap_t*)vheap;
	gc_object_t ret = (gc_object_t)NULL;
	hmu_t *hmu = NULL;
	gc_size_t tot_size = 0;

	bh_assert(!heap->is_shared_heap);
	bh_assert(gci_is_heap_valid(heap));

	/* align size*/
	tot_size = GC_ALIGN_8(size + HMU_SIZE + OBJ_PREFIX_SIZE + OBJ_SUFFIX_SIZE); /* hmu header, prefix, suffix*/
	if(tot_size < size) 
		return NULL;

	hmu = alloc_hmu_ex(heap, tot_size);
	if(!hmu) goto FINISH;

	/* reset all fields*/
	memset((char*)hmu + sizeof(*hmu), 0, tot_size - sizeof(*hmu));

	/* hmu->header = 0; */
	hmu_set_ut(hmu, HMU_JO);
	hmu_unmark_jo(hmu);

#if defined(GC_VERIFY)
	hmu_init_prefix_and_suffix(hmu, tot_size, file_name, line_number);
#endif
	ret = hmu_to_obj(hmu);

#if !defined(NVALGRIND)
        if (heap == &shared_heap)
	VALGRIND_MALLOCLIKE_BLOCK (ret, size, 16, 0);
        /* VALGRIND_PRINTF ("jjj %x %d\n", ret, size); */
#endif

#if BEIHAI_ENABLE_MEMORY_PROFILING != 0
    LOG_PROFILE_HEAP_NEW ((unsigned)heap, tot_size);
#endif

FINISH:

	return ret;
}

/* see ems_gc.h for description*/
gc_object_t gc_alloc_jo_i(void* cls,  gc_size_t size, gc_mm_t mmt ALLOC_EXTRA_PARAMETERS)
{
  (void)cls;
  (void)size;
  (void)mmt;
    bh_assert (0);
    return NULL;
}

/* Do some checking to see if given pointer is a possible valid heap*/

/* Return GC_TRUE if all checking passed*/
/* Return GC_FALSE otherwise*/
int gci_is_heap_valid(gc_heap_t *heap)
{
    if(!heap) return GC_FALSE;
    if(heap->heap_id != (gc_handle_t)heap) return GC_FALSE;

    return GC_TRUE;
}

int gc_free_i_heap(void *vheap, gc_object_t obj ALLOC_EXTRA_PARAMETERS)
{
    gc_heap_t* heap = (gc_heap_t*)vheap;
    hmu_t *hmu = NULL;
    hmu_t *prev = NULL;
    hmu_t *next = NULL;
    gc_size_t size = 0;
    hmu_type_t ut;
    int ret = GC_SUCCESS;

    if(!obj)
    {
        return GC_SUCCESS;
    }

#ifdef GC_NATIVE_SHARE_HEAP
    if (vheap == NULL || vheap == &shared_heap) {
        bh_free (obj);
        return GC_SUCCESS;
    }
#endif


    hmu = obj_to_hmu(obj);

    if (heap == NULL) {
        heap = &shared_heap;
    }

    gct_vm_mutex_lock(&heap->lock);

    if((gc_uint8 *)hmu >= heap->base_addr && (gc_uint8 *)hmu < heap->base_addr + heap->current_size)
    {
#ifdef GC_VERIFY
        hmu_verify(hmu);
#endif 
        ut = hmu_get_ut(hmu);
        if(ut == HMU_VO)
        {
            if(hmu_is_vo_freed(hmu)) {
                bh_assert(0);
                ret = GC_ERROR;
                goto out;
            }

            size = hmu_get_size (hmu);

#if GC_STAT_DATA != 0
            heap->total_free_size += size;
#endif
#if BEIHAI_ENABLE_MEMORY_PROFILING != 0
            LOG_PROFILE_HEAP_FREE ((unsigned)heap, size);
#endif

            if(!hmu_get_pinuse(hmu)) {
                prev = (hmu_t*) ((char*) hmu - *((int*)hmu - 1 ));

                if(hmu_is_in_heap(heap, prev) && hmu_get_ut(prev) == HMU_FC) {
                    size += hmu_get_size(prev);
                    hmu = prev;
                    unlink_hmu(heap, prev);
                }
            }

            next = (hmu_t*) ((char*) hmu + size);
            if(hmu_is_in_heap(heap, next)) {
                if(hmu_get_ut(next) == HMU_FC) {
                    size += hmu_get_size(next);
                    unlink_hmu(heap, next);
                    next = (hmu_t*) ((char*) hmu + size);
                } 
            }

            gci_add_fc(heap, hmu, size);

            if(hmu_is_in_heap(heap, next)) {
                hmu_unmark_pinuse(next);
            }


#if !defined(NVALGRIND)
        if (heap == &shared_heap)
            VALGRIND_FREELIKE_BLOCK (obj, 0);
            /* VALGRIND_PRINTF ("ZZZ %x\n", obj); */
#endif
        }
        else
        {
            ret = GC_ERROR;
            goto out;
        }
        ret = GC_SUCCESS;
        goto out;
    }

out:
    gct_vm_mutex_unlock(&heap->lock);
    return ret;
}

/* see ems_gc.h for description*/
int gc_free_i(gc_object_t obj ALLOC_EXTRA_PARAMETERS)
{
	return gc_free_i_heap(NULL, obj ALLOC_EXTRA_ARGUMENTS);
}


#ifdef GC_TEST

void gci_dump(char* buf, gc_heap_t *heap)
{
	hmu_t *cur = NULL, *end = NULL;
	hmu_type_t ut;
	gc_size_t size;
	int i = 0;
	int p;
	char inuse;
	int mark;

	cur = (hmu_t*)heap->base_addr;
	end = (hmu_t*)((char*)heap->base_addr + heap->current_size);

	while(cur < end)
	{
		ut = hmu_get_ut(cur);
		size = hmu_get_size(cur);
		p = hmu_get_pinuse(cur);
		mark = hmu_is_jo_marked (cur);
		
		if(ut == HMU_VO)
			inuse = 'V';
		else if(ut == HMU_JO)
			inuse = hmu_is_jo_marked(cur) ? 'J' : 'j';
		else if(ut == HMU_FC)
			inuse = 'F';

		bh_assert(size > 0);

		buf += sprintf(buf, "#%d %08x %x %x %d %c %d\n", i, (char*) cur - (char*) heap->base_addr, ut, p, mark, inuse, hmu_obj_size(size));
		
		cur = (hmu_t*)((char *)cur + size);
		i++;
	}

	bh_assert(cur == end);
}

#endif
