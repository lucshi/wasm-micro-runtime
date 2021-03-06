# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required (VERSION 2.8.2)
message ("lvgl_native_ui_app...")
project (lvgl_native_ui_app)

#################################################################

# Currently build as 64-bit by default. Set to "NO" to build 32-bit binaries.
set (BUILD_AS_64BIT_SUPPORT "YES")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
   if (${BUILD_AS_64BIT_SUPPORT} STREQUAL "YES")
     # Add -fPIC flag if build as 64-bit
     set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
     set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -fPIC")
   else ()
     add_definitions (-m32)
     set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
     set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
   endif ()
endif ()

set(WAMR_DEPS_DIR ../../../core/deps)
set(LVGL_SOURCE_DIR ${WAMR_DEPS_DIR}/lvgl)
set(LVGL_DRIVER_DIR ${WAMR_DEPS_DIR}/lv_drivers)

#################################

add_definitions(-DLV_CONF_INCLUDE_SIMPLE)
 
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${WAMR_DEPS_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../lv_config)


file(GLOB_RECURSE INCLUDES "${LVGL_DRIVER_DIR}/*.h" "${LVGL_SOURCE_DIR}/*.h"  "./*.h" )
file(GLOB_RECURSE SOURCES  "${LVGL_DRIVER_DIR}/*.c" "${LVGL_SOURCE_DIR}/*.c" )

add_executable(lvgl_native_ui_app main.c get_time.c ${SOURCES} ${INCLUDES})
target_link_libraries(lvgl_native_ui_app PRIVATE SDL2 )

