set(CPP_SOURCES 
  main.cpp 
  window.cpp 
  CoreEngine.cpp
  rendering/Device.cpp
  rendering/Image.cpp
  rendering/SwapChain.cpp
  rendering/BasicRasterPipeline.cpp
  rendering/Buffer.cpp
  rendering/Mesh.cpp
  rendering/Descriptors.cpp
  rendering/RenderingEngine.cpp
  ecs/ECS.cpp
  ecs/ECSComponent.cpp
  systems/FreeLook.cpp
  systems/FreeMove.cpp)

include_directories(../NoImplementationWarnings)
include_directories(../submodules/glm)
include_directories(../submodules/stb)

# register keyword no longer exists in c++17 and beyond. GLM still uses this keyword :(
add_compile_definitions(register= )

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
        glfw
        tinyobjloader
        spdlog::spdlog
        )

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
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VkApp>/res/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:VkApp>/res"

    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/res/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/res"
            "${PROJECT_BINARY_DIR}/res"
)
