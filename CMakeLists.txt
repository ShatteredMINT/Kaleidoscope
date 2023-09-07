cmake_minimum_required(VERSION 3.10)

project(Kaleidoscope)

add_subdirectory(src)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
