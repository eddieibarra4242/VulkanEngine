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