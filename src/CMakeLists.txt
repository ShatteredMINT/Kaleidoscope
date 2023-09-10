find_package(LLVM 18 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
file(GLOB_RECURSE SRC *.cpp)
add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC include include/ven)


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core orcjit native)

# Link against LLVM libraries
target_link_libraries(${PROJECT_NAME} ${llvm_libs})
