# Makefile for AliOS

NAME := iwasm
IWASM_ROOT := iwasm

GLOBAL_DEFINES += NVALGRIND
GLOBAL_INCLUDES += ${IWASM_ROOT}/runtime/include \
                   ${IWASM_ROOT}/runtime/platform/include \
                   ${IWASM_ROOT}/runtime/platform/alios \
                   ${IWASM_ROOT}/runtime/vmcore_wasm

$(NAME)_SOURCES := ${IWASM_ROOT}/runtime/utils/wasm_hashmap.c \
                   ${IWASM_ROOT}/runtime/utils/wasm_log.c \
                   ${IWASM_ROOT}/runtime/mem-mgt/ems_alloc.c \
                   ${IWASM_ROOT}/runtime/mem-mgt/ems_kfc.c \
                   ${IWASM_ROOT}/runtime/mem-mgt/ems_hmu.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_assert.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_definition.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_memory.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_math.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_dlfcn.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_platform.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_platform_log.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm_thread.c \
                   ${IWASM_ROOT}/runtime/platform/alios/wasm-native.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-application.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-interp.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-loader.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/wasm-runtime.c \
                   ${IWASM_ROOT}/runtime/vmcore_wasm/invokeNative_general.c \
                   src/main.c

