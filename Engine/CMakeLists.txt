set(CPP_SOURCES 
  main.cpp 
  window.cpp 
  rendering/Device.cpp)

add_library(vulkan SHARED IMPORTED)
set_target_properties(vulkan PROPERTIES 
  IMPORTED_LOCATION "/usr/lib/libvulkan.so" 
  INTERFACE_INCLUDE_DIRECTORIES "/usr/include/vulkan/")

# Generic test that uses conan libs
add_executable(VkApp ${CPP_SOURCES})
target_link_libraries(
  VkApp
  PRIVATE project_options
          project_warnings
          vulkan
          CONAN_PKG::fmt
          CONAN_PKG::spdlog
          CONAN_PKG::glfw
          CONAN_PKG::glm
          CONAN_PKG::stb)

set(GLSL_VALIDATOR "glslangValidator")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(VkApp Shaders)

add_custom_command(TARGET VkApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VkApp>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:VkApp>/shaders"
)