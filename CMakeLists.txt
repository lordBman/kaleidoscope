cmake_minimum_required(VERSION 3.10)
project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 23)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Add LLVM definitions and include directories
add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# Add compiler flags
add_compile_options(${LLVM_REQUIRED_CXX_FLAGS})

# Get LLVM libraries
llvm_map_components_to_libnames(llvm_libs core irreader analysis transformutils)

file(GLOB_RECURSE MY_SOURCES "src/*.cpp")

# Build our compiler
add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_link_libraries(${PROJECT_NAME} ${llvm_libs})