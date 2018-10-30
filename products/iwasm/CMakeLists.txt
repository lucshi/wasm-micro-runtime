include_directories (. ../../runtime/include
                     ../../runtime/platform/include
                     ../../runtime/platform/${PLATFORM}
                     ${CMAKE_CURRENT_BINARY_DIR})

add_executable (iwasm main.cpp)

target_link_libraries (iwasm platformlib vmcorelib vmutilslib platformlib -lm -ldl)

