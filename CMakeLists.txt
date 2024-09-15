# CMakeLists.txt for RocketLink Project

cmake_minimum_required(VERSION 3.14)

# Suppress CMP0167 warning by setting the policy
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(NovaLink VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally set the build type (Debug by default if not specified)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Enable position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Find Google Test
find_package(GTest REQUIRED)

# Google Test setup using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Add the NovaLink library
add_library(NovaLink STATIC)

# Collect all source and header files for the NovaLink library
file(GLOB_RECURSE NOVALINK_SOURCES src/*.cpp)
file(GLOB_RECURSE NOVALINK_HEADERS src/*.hpp)

target_sources(NovaLink PRIVATE ${NOVALINK_SOURCES} ${NOVALINK_HEADERS})

# Specify include directories
target_include_directories(NovaLink PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Link Boost to NovaLink
target_link_libraries(NovaLink PUBLIC Boost::system)

# Set compile options **after** defining NovaLink
target_compile_options(NovaLink PRIVATE 
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror -O3>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /O2>
)

# Enable testing
enable_testing()

# Add the tests directory
add_subdirectory(tests)
