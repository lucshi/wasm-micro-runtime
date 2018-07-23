include_directories (./include ../include ./${PLATFORM})

add_definitions (-D__POSIX__ -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=199309L)

file (GLOB_RECURSE source_all ${PLATFORM}/*.c)
add_library (platformlib ${source_all})

target_link_libraries (platformlib -pthread)

