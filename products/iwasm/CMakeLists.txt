include_directories (. ../../runtime/include
                     ../../runtime/platform/include
                     ../../runtime/platform/${PLATFORM}
                     ${CMAKE_CURRENT_BINARY_DIR})

add_executable (iwasm main.c)

target_link_libraries (iwasm platformlib vmcorelib vmutilslib platformlib -lm -ldl)

add_library (libiwasm SHARED main.c)

set_target_properties (libiwasm PROPERTIES OUTPUT_NAME iwasm)

target_link_libraries (libiwasm platformlib vmcorelib vmutilslib platformlib -lm -ldl)

