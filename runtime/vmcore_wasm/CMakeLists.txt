enable_language (ASM)

include_directories (. ../include ../platform/include ../platform/${PLATFORM})

file (GLOB_RECURSE source_all *.c *.s)
file (GLOB_RECURSE source_invokenative_general invokeNative_general.c)
list (REMOVE_ITEM source_all ${source_invokenative_general})

file (GLOB_RECURSE source_wasm_native_all wasm-native_*.c)
file (GLOB_RECURSE source_wasm_native_linux wasm-native_linux.c)
list (REMOVE_ITEM source_all ${source_wasm_native_all})
list (APPEND source_all ${source_wasm_native_linux})


add_library (vmcorelib ${source_all})

