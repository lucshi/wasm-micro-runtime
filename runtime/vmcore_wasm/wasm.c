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

#include "wasm.h"
#include "bh_log.h"
#include "bh_memory.h"

#define CHECK_BUF(buf, buf_end, length) do {                    \
  if (buf + length > buf_end) {                                 \
    LOG_ERROR("WASM read data failed: data out of range.\n");   \
    return false;                                               \
  }                                                             \
} while (0)

bool
read_leb(const uint8 *buf, const uint8 *buf_end,
         uint32 *p_offset, uint32 maxbits,
         bool sign, uint64 *p_result)
{
  uint64 result = 0;
  uint32 shift = 0;
  uint32 bcnt = 0;
  uint32 start_pos = *p_offset;
  uint64 byte;

  CHECK_BUF(buf, buf_end, *p_offset);
  while (true) {
    byte = buf[*p_offset];
    *p_offset += 1;
    CHECK_BUF(buf, buf_end, *p_offset);
    result |= ((byte & 0x7f) << shift);
    shift += 7;
    if ((byte & 0x80) == 0) {
      break;
    }
    bcnt += 1;
    if (bcnt > (maxbits + 7 - 1) / 7) {
      LOG_ERROR("WASM module load failed: unsigned LEB at byte %d overflow\n",
                start_pos);
      return false;
    }
  }
  if (sign && (shift < maxbits) && (byte & 0x40)) {
    /* Sign extend */
    result |= - (1 << shift);
  }
  *p_result = result;
  return true;
}

