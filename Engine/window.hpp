/*
   Copyright 2022 Eduardo Ibarra

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>

constexpr std::array<const char*, 1> validationLayers{ "VK_LAYER_KHRONOS_validation" };

class Window {
public:
    Window(uint32_t width, uint32_t height, const char* title);
    ~Window();

    bool shouldClose() const;

    [[nodiscard]] constexpr GLFWwindow* glfwWindowPtr() { return m_window; }

    [[nodiscard]] constexpr uint32_t width() const { return m_width; }
    [[nodiscard]] constexpr uint32_t height() const { return m_height; }
    [[nodiscard]] constexpr float aspect_ratio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

    [[nodiscard]] constexpr const VkInstance& context() const { return m_context; }
    [[nodiscard]] constexpr const VkSurfaceKHR& surface() const { return m_windowSurface; }

private:
    GLFWwindow* m_window;

    uint32_t m_width;
    uint32_t m_height;

    VkInstance m_context;
    VkDebugUtilsMessengerEXT m_debugMessanger;
    VkSurfaceKHR m_windowSurface;

    friend void resizeCallback(GLFWwindow* window, int width, int height);
};