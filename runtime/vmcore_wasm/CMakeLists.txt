include_directories (. ../include ../platform/include ../platform/${PLATFORM})

file (GLOB_RECURSE source_all *.c)

add_library (vmcorelib ${source_all})

