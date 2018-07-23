include_directories (. ../../runtime/include
                     ../../runtime/platform/include
                     ../../runtime/platform/${PLATFORM}
                     ${CMAKE_CURRENT_BINARY_DIR})

add_executable (wavm main.c)

target_link_libraries (wavm platformlib vmcorelib vmutilslib -lpthread)

