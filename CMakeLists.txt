cmake_minimum_required(VERSION 3.15)

# Set the project name to your project name, my project isn't very descriptive
project(VulkanEngine CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

add_custom_target(format
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/format.sh
  USES_TERMINAL
)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} glfw/3.3.4)
set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} glm/0.9.9.8)
set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} stb/cci.20210910)

include(cmake/Conan.cmake)
run_conan()

add_subdirectory(Engine)