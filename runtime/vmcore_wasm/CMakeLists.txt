enable_language (ASM)

include_directories (. ../include ../platform/include ../platform/${PLATFORM})

file (GLOB_RECURSE source_all *.c *.s)
file (GLOB_RECURSE source_invokenative_general invokeNative_general.c)
list (REMOVE_ITEM source_all ${source_invokenative_general})

add_library (vmcorelib ${source_all})

