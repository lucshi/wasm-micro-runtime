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

#ifndef _WASM_NATIVE_H
#define _WASM_NATIVE_H

#include "wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lookup native function implementation of a given import function.
 *
 * @param module_name the module name of the import function
 * @param func_name the function name of the import function
 *
 * @return return the native function pointer if success, NULL otherwise
 */
void*
wasm_native_func_lookup(const char *module_name, const char *func_name);

/**
 * Lookup global variable of a given import global
 *
 * @param module_name the module name of the import global
 * @param global_name the global name of the import global
 *
 * @param return pointer point to the import global if success, NULL otherwise
 */
void*
wasm_native_global_lookup(const char *module_name, const char *global_name);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_NATIVE_H */
