cmake_minimum_required(VERSION 3.8.2)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

enable_language (ASM)

add_definitions (-DNVALGRIND)

set (IWASM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)

include_directories (${IWASM_ROOT}/runtime/include
                     ${IWASM_ROOT}/runtime/platform/include
                     ${IWASM_ROOT}/runtime/platform/zephyr
                     ${IWASM_ROOT}/runtime/vmcore)

set (IWASM_SRCS ${IWASM_ROOT}/runtime/utils/bh_hashmap.c
                ${IWASM_ROOT}/runtime/utils/bh_log.c
                ${IWASM_ROOT}/runtime/gc/ems_alloc.c
                ${IWASM_ROOT}/runtime/gc/ems_kfc.c
                ${IWASM_ROOT}/runtime/gc/ems_hmu.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_assert.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_definition.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_memory.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_math.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_dlfcn.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_platform_log.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_time.c
                ${IWASM_ROOT}/runtime/platform/zephyr/bh_thread.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-application.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-import.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-interp.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-loader.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-native_zephyr.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-runtime.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-thread.c
                ${IWASM_ROOT}/runtime/vmcore_wasm/invokeNative_general.c
                ${IWASM_ROOT}/products/iwasm/main.c
                ${IWASM_ROOT}/lib/lib-export.c)

target_sources(app PRIVATE src/main.c ${IWASM_SRCS})
