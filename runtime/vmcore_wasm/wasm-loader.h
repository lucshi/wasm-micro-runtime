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

#ifndef _WASM_LOADER_H
#define _WASM_LOADER_H

#include "wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load WASM module from specified byte buffer.
 *
 * @param buf the byte buffer contains the WASM binary data
 * @param size the size of the buffer
 *
 * @return return module loaded, NULL if failed
 */
WASMModule*
wasm_wasm_module_load(const uint8 *buf, uint32 size);

/**
 * Unload WASM module.
 *
 * @param module the module to be unloaded
 *
 * @return true if success, false otherwise
 */
bool
wasm_wasm_module_unload(WASMModule *module);


#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_LOADER_H */
