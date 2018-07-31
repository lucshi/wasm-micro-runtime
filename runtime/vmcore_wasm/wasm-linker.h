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

#ifndef _WASM_LINKER_H
#define _WASM_LINKER_H

#include "wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
bool
wasm_root_resolver_resolve(const char *module_name, const char *export_name,
                           uint8 obj_type, Object* p_out_obj);

Object*
wasm_root_resolver_get_stub_object(const char *export_name, uint8 obj_type);

EmscriptenInstance*
wasm_emsc_inst_instantiate (Compartment *compartment, WASMModule *module);

void
wasm_emsc_inst_init_globals(Context *context, WASMModule *module,
                            ModuleInstance *module_instance);

void
wasm_emsc_inst_inject_command_args(EmscriptenInstance *instance,
                                   const Vector *arg_strings,
                                   Vector **p_out_invoke_args);
#endif


#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_LINKER_H */
