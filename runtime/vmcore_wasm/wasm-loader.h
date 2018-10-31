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
#include "bh_hashmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load a WASM module from a specified byte buffer.
 *
 * @param buf the byte buffer which contains the WASM binary data
 * @param size the size of the buffer
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return module loaded, NULL if failed
 */
WASMModule*
wasm_loader_load(const uint8 *buf, uint32 size, char *error_buf, uint32 error_buf_size);

/**
 * Unload a WASM module.
 *
 * @param module the module to be unloaded
 */
void
wasm_loader_unload(WASMModule *module);

/**
 * Find address of related else opcode and end opcode of opcode block/loop/if
 * according to the start address of opcode.
 *
 * @param branch_set the hashtable to store the else/end adress info of
 * block/loop/if opcode. The function will lookup the hashtable firstly,
 * if not found, it will then search the code from start_addr, and if success,
 * stores the result to the hashtable.
 * @param start_addr the next address of opcode block/loop/if
 * @param code_end_addr the end address of function code block
 * @param block_type the type of block, 0/1/2 denotes block/loop/if
 * @param p_else_addr returns the else addr if found
 * @param p_end_addr returns the end addr if found
 * @param error_buf returns the error log for this function
 * @param error_buf_size returns the error log string length
 *
 * @return true if success, false otherwise
 */
bool
wasm_loader_find_block_addr(HashMap *map,
                            const uint8 *start_addr,
                            const uint8 *code_end_addr,
                            uint8 block_type,
                            uint8 **p_else_addr,
                            uint8 **p_end_addr,
                            char *error_buf,
                            uint32 error_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_LOADER_H */
