#pragma once

#include <vulkan/vulkan.h>
#include <string_view>
#include <spdlog/spdlog.h>

#define ASSERT_VK_SUCCESS(res, msg) __vk_util_assert(__FILE__, __LINE__, res, msg)

constexpr void __vk_util_assert(const std::string &file, const int &line, VkResult res, const std::string_view& msg)
{
    if(res == VK_SUCCESS)
        return;

    spdlog::critical("[{}:{}]\n{}\nVkResult: {}", file, line, msg.cbegin(), res);
    throw std::runtime_error("Bad Result");
}