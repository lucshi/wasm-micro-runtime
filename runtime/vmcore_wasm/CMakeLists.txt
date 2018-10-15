enable_language (ASM)

include_directories (. ../include ../platform/include ../platform/${PLATFORM})

file (GLOB_RECURSE source_all *.c *.s)

add_library (vmcorelib ${source_all})

