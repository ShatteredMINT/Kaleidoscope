cmake_minimum_required(VERSION 3.10)

project(Kaleidoscope)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(src)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
