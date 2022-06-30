#include "window.hpp"

#include "utils.hpp"
#include <spdlog/spdlog.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    [[maybe_unused]] void *pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::debug(pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info(pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::critical(pCallbackData->pMessage);
        throw std::runtime_error(fmt::format("[{}]", pCallbackData->pMessageIdName));
        break;
    default:
        break;
    }

    return VK_FALSE;
}

void setupValidationLayers([[maybe_unused]] VkInstance &context, [[maybe_unused]] VkDebugUtilsMessengerCreateInfoEXT createInfo, [[maybe_unused]] VkDebugUtilsMessengerEXT &debugMessanger)
{
#ifndef NDEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(context, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(context, &createInfo, nullptr, &debugMessanger);
    } else {
        spdlog::critical("failed to set up debug messenger!");
        throw std::runtime_error("func is nullptr");
    }
#endif
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
    Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    win->m_width = static_cast<uint32_t>(width);
    win->m_height = static_cast<uint32_t>(height);
}

Window::Window(uint32_t width, uint32_t height, const char* title) : m_width{ width }, m_height{ height }
{
    if(!glfwInit()) {
        spdlog::critical("Could not initialize GLFW!");
        return;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);

    const GLFWvidmode *vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(m_window, (vm->width - static_cast<int>(width)) / 2, (vm->height - static_cast<int>(height)) / 2);
    glfwShowWindow(m_window);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, &resizeCallback);

    //------- Vulkan Initialization --------
    VkApplicationInfo appInfo{ };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title;
    appInfo.applicationVersion = VK_API_VERSION_1_3;
    appInfo.pEngineName = "Simple Vk Engine";
    appInfo.engineVersion = VK_API_VERSION_1_3;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instInfo{ };
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

#ifndef NDEBUG
    instInfo.enabledLayerCount = validationLayers.size();
    instInfo.ppEnabledLayerNames = validationLayers.data();

    instInfo.pNext = &debugCreateInfo;
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else
    instInfo.enabledLayerCount = 0;
    instInfo.ppEnabledLayerNames = nullptr;
    instInfo.pNext = nullptr;
#endif

    instInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instInfo.ppEnabledExtensionNames = extensions.data();

    ASSERT_VK_SUCCESS(vkCreateInstance(&instInfo, nullptr, &m_context), "Failed to create VkInstance");

    setupValidationLayers(m_context, debugCreateInfo, m_debugMessanger);

    ASSERT_VK_SUCCESS(glfwCreateWindowSurface(m_context, m_window, nullptr, &m_windowSurface), "failed to create window surface!");
}

Window::~Window()
{
#ifndef NDEBUG
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_context, "vkDestroyDebugUtilsMessengerEXT"));
        
    if (func != nullptr) {
        func(m_context, m_debugMessanger, nullptr);
    }
#endif

    vkDestroySurfaceKHR(m_context, m_windowSurface, nullptr);
    vkDestroyInstance(m_context, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}