# Set minimum CMake version
cmake_minimum_required(VERSION 3.25)
project(remove-fps-limit LANGUAGES C VERSION 5.19)

# Use MultiThreadedDLL as MSVC runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

# Only allow MSVC compiler
if(NOT MSVC)
    message(FATAL_ERROR "${PROJECT_NAME} should be built with MSVC")
endif()

# Add MinHook library from subdirectory
add_subdirectory(vendor/minhook)

# Create shared library (DLL)
add_library(${PROJECT_NAME} SHARED
    src/fps.c
)

# Set output DLL name to dinput8.dll
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "dinput8"
    SUFFIX ".dll"
)

# Set warning level to W3
target_compile_options(${PROJECT_NAME} PRIVATE /W3)

# Link against MinHook library
target_link_libraries(${PROJECT_NAME} PRIVATE
    minhook
)

# Disable security warnings and unnecessary system macros
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)
