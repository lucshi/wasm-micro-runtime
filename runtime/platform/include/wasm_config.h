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

#ifndef _WASM_CONFIG_H
#define _WASM_CONFIG_H

#define WASM_ENABLE_LOG 1
#define WASM_ENABLE_LABELS_AS_VALUES 1

/* Enable non-syscall mode of emcc compiler (with -s SIDE_MODULE=1),
   which imports LIBC's functions */
#define WASM_ENABLE_EMCC_LIBC 1

/* Enable syscall mode of emcc compiler (without -s SIDE_MODULE=1),
   which imports syscall functions */
#define WASM_ENABLE_EMCC_SYSCALL 1

/* Enable wasmception clang compiler, which imports syscall functions */
#define WASM_ENABLE_WASMCEPTION 0

#endif /* end of _WASM_CONFIG_H */
