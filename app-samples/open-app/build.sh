#
# INTEL CONFIDENTIAL
#
# Copyright 2017-2018 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials,
# and your use of them is governed by the express license under which they
# were provided to you (License). Unless the License provides otherwise, you
# may not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#

#source /home/luc/workspace/emsdk/emsdk_env.sh
emcc -Iinclude -O3 -s WASM=1 -s SIDE_MODULE=1 -s TOTAL_MEMORY=65536 -s TOTAL_STACK=65536 -s "EXPORTED_FUNCTIONS=['_on_init', '_main']" -o TestApp1.wasm main.c

