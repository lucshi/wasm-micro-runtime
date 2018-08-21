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

#ifndef _WASM_OPCODE_H
#define _WASM_OPCODE_H

#include "wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WASMOpcode {
  WASM_OP_NOP           = 0x00,

  WASM_OP_END           = 0x0b, /* end */

  WASM_OP_RETURN        = 0x0f, /* return */
  WASM_OP_CALL          = 0x10, /* call */
  WASM_OP_CALL_INDIRECT = 0x11, /* call_indirect */

  WASM_OP_GET_LOCAL     = 0x20, /* get_local */
  WASM_OP_SET_LOCAL     = 0x21, /* set_local */
  WASM_OP_TEE_LOCAL     = 0x22, /* tee_local */
  WASM_OP_GET_GLOBAL    = 0x23, /* get_global */
  WASM_OP_SET_GLOBAL    = 0x24, /* set_global */

  WASM_OP_I32_LOAD      = 0x28, /* i32.load */

  WASM_OP_I32_CONST     = 0x41, /* i32.const */

  WASM_OP_IMPDEP1       = 0xfe,
  WASM_OP_IMPDEP2       = 0xff
} WASMOpcode;

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_OPCODE_H */
