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

#include "bh_assert.h"
#include <stdio.h>
#include <stdlib.h>

void bh_assert_internal(int v, const char *file_name, int line_number, const char *expr_string)
{
  if(v) return;

  if(!file_name) file_name = "NULL FILENAME";
  if(!expr_string) expr_string = "NULL EXPR_STRING";

  printf("\nASSERTION FAILED: %s, at FILE=%s, LINE=%d\n", expr_string, file_name, line_number);
  abort();
}
