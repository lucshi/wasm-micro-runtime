# Makefile for AliOS

NAME := iwasm

GLOBAL_DEFINES += NVALGRIND
GLOBAL_INCLUDES += runtime/include \
                   runtime/platform/include \
                   runtime/platform/alios \
                   runtime/vmcore

$(NAME)_SOURCES := runtime/utils/bh_hashmap.c \
                   runtime/utils/bh_log.c \
                   runtime/gc/ems_alloc.c \
                   runtime/gc/ems_kfc.c \
                   runtime/gc/ems_hmu.c \
                   runtime/platform/alios/bh_assert.c \
                   runtime/platform/alios/bh_definition.c \
                   runtime/platform/alios/bh_memory.c \
                   runtime/platform/alios/bh_math.c \
                   runtime/platform/alios/bh_dlfcn.c \
                   runtime/platform/alios/bh_platform_log.c \
                   runtime/platform/alios/bh_time.c \
                   runtime/platform/alios/bh_thread.c \
                   runtime/vmcore_wasm/wasm.c \
                   runtime/vmcore_wasm/wasm-application.c \
                   runtime/vmcore_wasm/wasm-import.c \
                   runtime/vmcore_wasm/wasm-interp.c \
                   runtime/vmcore_wasm/wasm-loader.c \
                   runtime/vmcore_wasm/wasm-native_alios.c \
                   runtime/vmcore_wasm/wasm-runtime.c \
                   runtime/vmcore_wasm/wasm-thread.c \
                   runtime/vmcore_wasm/invokeNative_general.c \
                   products/iwasm/main.c

