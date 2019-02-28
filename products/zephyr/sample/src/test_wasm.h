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

unsigned char wasm_test_file[] = {
  0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x06, 0x64, 0x79, 0x6C, 0x69, 0x6E, 0x6B, 0xC0, 0x80, 0x04,
  0x04, 0x00, 0x00, 0x01, 0x13, 0x04, 0x60, 0x01, 0x7F, 0x00, 0x60, 0x01, 0x7F, 0x01, 0x7F, 0x60, 0x02, 0x7F, 0x7F, 0x01,
  0x7F, 0x60, 0x00, 0x00, 0x02, 0x58, 0x06, 0x03, 0x65, 0x6E, 0x76, 0x05, 0x5F, 0x66, 0x72, 0x65, 0x65, 0x00, 0x00, 0x03,
  0x65, 0x6E, 0x76, 0x07, 0x5F, 0x6D, 0x61, 0x6C, 0x6C, 0x6F, 0x63, 0x00, 0x01, 0x03, 0x65, 0x6E, 0x76, 0x07, 0x5F, 0x70,
  0x72, 0x69, 0x6E, 0x74, 0x66, 0x00, 0x02, 0x03, 0x65, 0x6E, 0x76, 0x05, 0x5F, 0x70, 0x75, 0x74, 0x73, 0x00, 0x01, 0x03,
  0x65, 0x6E, 0x76, 0x0D, 0x5F, 0x5F, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 0x5F, 0x62, 0x61, 0x73, 0x65, 0x03, 0x7F, 0x00,
  0x03, 0x65, 0x6E, 0x76, 0x06, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 0x02, 0x00, 0x01, 0x03, 0x04, 0x03, 0x02, 0x03, 0x03,
  0x06, 0x10, 0x03, 0x7F, 0x01, 0x41, 0x00, 0x0B, 0x7F, 0x01, 0x41, 0x00, 0x0B, 0x7F, 0x00, 0x41, 0x1B, 0x0B, 0x07, 0x33,
  0x04, 0x12, 0x5F, 0x5F, 0x70, 0x6F, 0x73, 0x74, 0x5F, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74, 0x69, 0x61, 0x74, 0x65,
  0x00, 0x06, 0x05, 0x5F, 0x6D, 0x61, 0x69, 0x6E, 0x00, 0x04, 0x0B, 0x72, 0x75, 0x6E, 0x50, 0x6F, 0x73, 0x74, 0x53, 0x65,
  0x74, 0x73, 0x00, 0x05, 0x04, 0x5F, 0x73, 0x74, 0x72, 0x03, 0x03, 0x0A, 0xBA, 0x01, 0x03, 0x9E, 0x01, 0x01, 0x01, 0x7F,
  0x23, 0x01, 0x21, 0x00, 0x23, 0x01, 0x41, 0x10, 0x6A, 0x24, 0x01, 0x20, 0x00, 0x41, 0x08, 0x6A, 0x21, 0x02, 0x23, 0x00,
  0x41, 0x1B, 0x6A, 0x10, 0x03, 0x1A, 0x41, 0x80, 0x08, 0x10, 0x01, 0x21, 0x01, 0x20, 0x01, 0x04, 0x7F, 0x20, 0x00, 0x20,
  0x01, 0x36, 0x02, 0x00, 0x23, 0x00, 0x20, 0x00, 0x10, 0x02, 0x1A, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x0D, 0x3A, 0x00,
  0x00, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x0E, 0x3A, 0x00, 0x01, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x0F, 0x3A, 0x00,
  0x02, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x10, 0x3A, 0x00, 0x03, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x11, 0x3A, 0x00,
  0x04, 0x20, 0x01, 0x23, 0x00, 0x2C, 0x00, 0x12, 0x3A, 0x00, 0x05, 0x20, 0x02, 0x20, 0x01, 0x36, 0x02, 0x00, 0x23, 0x00,
  0x41, 0x13, 0x6A, 0x20, 0x02, 0x10, 0x02, 0x1A, 0x20, 0x01, 0x10, 0x00, 0x20, 0x00, 0x24, 0x01, 0x41, 0x00, 0x05, 0x23,
  0x00, 0x41, 0x28, 0x6A, 0x10, 0x03, 0x1A, 0x20, 0x00, 0x24, 0x01, 0x41, 0x7F, 0x0B, 0x0B, 0x03, 0x00, 0x01, 0x0B, 0x14,
  0x00, 0x23, 0x00, 0x41, 0x40, 0x6B, 0x24, 0x01, 0x23, 0x01, 0x41, 0x80, 0x80, 0x04, 0x6A, 0x24, 0x02, 0x10, 0x05, 0x0B,
  0x0B, 0x3F, 0x01, 0x00, 0x23, 0x00, 0x0B, 0x39, 0x62, 0x75, 0x66, 0x20, 0x70, 0x74, 0x72, 0x3A, 0x20, 0x25, 0x70, 0x0A,
  0x00, 0x31, 0x32, 0x33, 0x34, 0x0A, 0x00, 0x62, 0x75, 0x66, 0x3A, 0x20, 0x25, 0x73, 0x00, 0x48, 0x65, 0x6C, 0x6C, 0x6F,
  0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x00, 0x6D, 0x61, 0x6C, 0x6C, 0x6F, 0x63, 0x20, 0x62, 0x75, 0x66, 0x20, 0x66,
  0x61, 0x69, 0x6C, 0x65, 0x64, 0x00, 0x50, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0x01, 0x49, 0x07, 0x00, 0x05, 0x5F, 0x66, 0x72,
  0x65, 0x65, 0x01, 0x07, 0x5F, 0x6D, 0x61, 0x6C, 0x6C, 0x6F, 0x63, 0x02, 0x07, 0x5F, 0x70, 0x72, 0x69, 0x6E, 0x74, 0x66,
  0x03, 0x05, 0x5F, 0x70, 0x75, 0x74, 0x73, 0x04, 0x05, 0x5F, 0x6D, 0x61, 0x69, 0x6E, 0x05, 0x0B, 0x72, 0x75, 0x6E, 0x50,
  0x6F, 0x73, 0x74, 0x53, 0x65, 0x74, 0x73, 0x06, 0x12, 0x5F, 0x5F, 0x70, 0x6F, 0x73, 0x74, 0x5F, 0x69, 0x6E, 0x73, 0x74,
  0x61, 0x6E, 0x74, 0x69, 0x61, 0x74, 0x65, 0x00, 0x20, 0x10, 0x73, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x4D, 0x61, 0x70, 0x70,
  0x69, 0x6E, 0x67, 0x55, 0x52, 0x4C, 0x0E, 0x61, 0x2E, 0x6F, 0x75, 0x74, 0x2E, 0x77, 0x61, 0x73, 0x6D, 0x2E, 0x6D, 0x61,
  0x70
};
