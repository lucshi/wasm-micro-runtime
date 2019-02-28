# Makefile for AliOS

NAME := iwasm
IWASM_ROOT := iwasm

GLOBAL_DEFINES += NVALGRIND
GLOBAL_INCLUDES += ${IWASM_ROOT}/runtime/include \
                   ${IWASM_ROOT}/runtime/platform/include \
                   ${IWASM_ROOT}/runtime/platform/alios \
                   ${IWASM_ROOT}/runtime/vmcore_wasm

$(NAME)_SOURCES := ${IWASM_ROOT}/runtime/utils/bh_hashmap.c \
                   ${IWASM_ROOT}/runtime/utils/bh_log.c \
                   ${IWASM_ROOT}/runtime/gc/ems_alloc.c \
                   ${IWASM_ROOT}/runtime/gc/ems_kfc.c \
                   ${IWASM_ROOT}/runtime/gc/ems_hmu.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_assert.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_definition.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_memory.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_math.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_dlfcn.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_platform_log.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_time.c \
                   ${IWASM_ROOT}/runtime/platform/alios/bh_thread.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm-native.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-application.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-import.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-interp.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-loader.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-runtime.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-thread.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/invokeNative_general.c \
                   src/main.c

