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

include_directories (. ../include ../applet ../../classlib/include ../platform/include ../platform/${PLATFORM} )


file (GLOB_RECURSE source_all *.c )

add_library (vmgclib ${source_all})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
 add_library (vmgclib_ut ${source_all})

 set_target_properties (vmgclib_ut PROPERTIES COMPILE_DEFINITIONS BH_TEST=1)
 # add_definitions (-DGC_VERIFY)

endif ()

