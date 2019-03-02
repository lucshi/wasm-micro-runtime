cmake_minimum_required(VERSION 3.8.2)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

enable_language (ASM)

add_definitions (-DNVALGRIND)

set (IWASM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/iwasm)

include_directories (${IWASM_ROOT}/runtime/include
                     ${IWASM_ROOT}/runtime/platform/include
                     ${IWASM_ROOT}/runtime/platform/zephyr
                     ${IWASM_ROOT}/runtime/vmcore_wasm)

set (IWASM_SRCS ${IWASM_ROOT}/runtime/utils/wasm_hashmap.c
                ${IWASM_ROOT}/runtime/utils/wasm_log.c
                ${IWASM_ROOT}/runtime/gc/ems_alloc.c
                ${IWASM_ROOT}/runtime/gc/ems_kfc.c
                ${IWASM_ROOT}/runtime/gc/ems_hmu.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_assert.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_definition.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_memory.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_math.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_dlfcn.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_platform_log.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_time.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm_thread.c
                ${IWASM_ROOT}/runtime/platform/zephyr/wasm-native.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-application.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-import.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-interp.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-loader.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-runtime.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-thread.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/invokeNative_general.c
                ${IWASM_ROOT}/lib/lib-export.c)

target_sources(app PRIVATE ${IWASM_SRCS} src/main.c)
