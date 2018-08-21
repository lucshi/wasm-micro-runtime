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

#ifndef _WASM_INTERP_H
#define _WASM_INTERP_H

#include "wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMFunctionInstance;

typedef struct WASMInterpFrame {
  /* The frame of the caller that are calling the current function. */
  struct WASMInterpFrame *prev_frame;

  /* The current WASM function. */
  struct WASMFunctionInstance *function;

  /* Instruction pointer of the bytecode array.  */
  uint8 *ip;

  /* Operand stack top pointer of the current frame.  The bottom of
     the stack is the next cell after the last local variable.  */
  uint32 *sp;

  uint32 lp[1];
} WASMInterpFrame;

/**
 * Calculate the size of interpreter area of frame of a function.
 *
 * @param all_cell_num number of all cells including local variables
 * and the working stack slots
 *
 * @return the size of interpreter area of the frame
 */
static inline unsigned
wasm_interp_interp_frame_size(unsigned all_cell_num)
{
  return align_uint(offsetof(WASMInterpFrame, lp) + all_cell_num * 5, 4);
}

void
wasm_interp_call_wasm(struct WASMFunctionInstance *function,
                      uint32 argc, uint32 argv[]);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_INTERP_H */
