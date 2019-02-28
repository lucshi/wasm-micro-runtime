set (VMCORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR})

if (${BUILD_AS_64BIT_SUPPORT} STREQUAL "YES")
file (GLOB_RECURSE source_all ${VMCORE_LIB_DIR}/*.c)
else ()
file (GLOB_RECURSE source_all ${VMCORE_LIB_DIR}/*.c ${VMCORE_LIB_DIR}/*.s)
list (REMOVE_ITEM source_all ${VMCORE_LIB_DIR}/invokeNative_general.c)
endif ()

set (VMCORE_LIB_SOURCE ${source_all})

