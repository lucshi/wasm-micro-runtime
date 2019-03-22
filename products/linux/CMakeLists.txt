cmake_minimum_required (VERSION 2.8)

project (iwasm)

set (PLATFORM "linux")

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Enable repl mode if want to test spec cases
# add_definitions(-DWASM_ENABLE_REPL)

if (NOT ("$ENV{VALGRIND}" STREQUAL "YES"))
  add_definitions(-DNVALGRIND)
endif ()

# Currently build as 32-bit by default.
set (BUILD_AS_64BIT_SUPPORT "NO")

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

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wall -Wno-unused-parameter -Wno-pedantic")

include_directories (.
                     ../../runtime/include
                     ../../runtime/platform/include)

enable_language (ASM)

include (../../runtime/platform/${PLATFORM}/platform.cmake)
include (../../runtime/utils/utils.cmake)
include (../../runtime/vmcore_wasm/vmcore.cmake)

add_library (vmlib
             ${PLATFORM_LIB_SOURCE}
             ${UTILS_LIB_SOURCE}
             ${VMCORE_LIB_SOURCE})

add_executable (iwasm main.c)

target_link_libraries (iwasm vmlib -lm -ldl -lpthread)

add_library (libiwasm SHARED
             ${PLATFORM_LIB_SOURCE}
             ${UTILS_LIB_SOURCE}
             ${VMCORE_LIB_SOURCE})

set_target_properties (libiwasm PROPERTIES OUTPUT_NAME iwasm)

target_link_libraries (libiwasm -lm -ldl -lpthread)

