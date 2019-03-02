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

#if defined(GC_VERIFY)
/* Set default value to prefix and suffix*/

/* @hmu should not be NULL and it should have been correctly initilized (except for prefix and suffix part)*/
/* @tot_size is offered here because hmu_get_size can not be used till now. @tot_size should not be smaller than OBJ_EXTRA_SIZE.*/
/*  For VO, @tot_size should be equal to object total size.*/
void hmu_init_prefix_and_suffix(hmu_t *hmu, gc_size_t tot_size, const char *file_name, int line_no)
{
	gc_object_prefix_t *prefix = NULL;
	gc_object_suffix_t *suffix = NULL;
	gc_uint32 i = 0;

	wasm_assert(hmu);
	wasm_assert(hmu_get_ut(hmu) == HMU_JO || hmu_get_ut(hmu) == HMU_VO);
	wasm_assert(tot_size >= OBJ_EXTRA_SIZE);
	wasm_assert(!(tot_size & 7));
	wasm_assert(hmu_get_ut(hmu) != HMU_VO || hmu_get_size(hmu) >= tot_size);

	prefix = (gc_object_prefix_t *)(hmu + 1);
	suffix = (gc_object_suffix_t *)((gc_uint8*)hmu + tot_size - OBJ_SUFFIX_SIZE);
	prefix->file_name = file_name;
	prefix->line_no = line_no;
	prefix->size = tot_size;
	for(i = 0;i < GC_OBJECT_PREFIX_PADDING_CNT;i++)
	{
		prefix->padding[i] = GC_OBJECT_PADDING_VALUE;
	}
	for(i = 0;i < GC_OBJECT_SUFFIX_PADDING_CNT;i++)
	{
		suffix->padding[i] = GC_OBJECT_PADDING_VALUE;
	}
}

void hmu_verify(hmu_t *hmu)
{
	gc_object_prefix_t *prefix = NULL;
	gc_object_suffix_t *suffix = NULL;
	gc_uint32 i = 0;
	hmu_type_t ut;
	gc_size_t size = 0;
	int is_padding_ok = 1;

	wasm_assert(hmu);
	ut = hmu_get_ut(hmu);
	wasm_assert(hmu_is_ut_valid(ut));

	prefix = (gc_object_prefix_t *)(hmu + 1);
        size = prefix->size;
	suffix = (gc_object_suffix_t *)((gc_uint8*)hmu + size - OBJ_SUFFIX_SIZE);

	if(ut == HMU_VO || ut == HMU_JO)
	{
		/* check padding*/
		for(i = 0;i < GC_OBJECT_PREFIX_PADDING_CNT;i++)
		{
			if(prefix->padding[i] != GC_OBJECT_PADDING_VALUE)
			{
				is_padding_ok = 0;
				break;
			}
		}
		for(i = 0;i < GC_OBJECT_SUFFIX_PADDING_CNT;i++)
		{
			if(suffix->padding[i] != GC_OBJECT_PADDING_VALUE)
			{
				is_padding_ok = 0;
				break;
			}
		}

		if(!is_padding_ok)
		{
			LOG_ERROR("Invalid padding for object created at %s:%d", (prefix->file_name ? prefix->file_name : ""), prefix->line_no);
		}
		wasm_assert(is_padding_ok);
	}
}
#endif

