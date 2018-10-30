
cmake_minimum_required (VERSION 2.8)

project (wasm)

set (PLATFORM "linux")

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if ("${INSTRUMENT_TEST_ENABLED}" STREQUAL "YES")
  add_definitions(-DINSTRUMENT_TEST_ENABLED)
endif ()

if (NOT ("$ENV{VALGRIND}" STREQUAL "YES"))
  add_definitions(-DNVALGRIND)
endif ()

# Currently only support 32-bit platforms.
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_definitions (-m32)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
endif ()

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=free")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -ffunction-sections -fdata-sections -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-variadic-macros")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -Wall -pedantic -Wno-variadic-macros")

set (MEMORYCHECK_COMMAND_OPTIONS "--suppressions=${CMAKE_SOURCE_DIR}/tools/vimash/valgrind.supp")

include (CTest)

# Disable code optimization when collecting code coverage.
if ("${NO_OPT}" STREQUAL "YES")
  add_definitions (-O0)
endif ()

# Add subdirectories:
add_subdirectory (runtime)
add_subdirectory (products)
#add_subdirectory (tools)
