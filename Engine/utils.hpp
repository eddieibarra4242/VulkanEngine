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

#include <vulkan/vulkan.h>
#include <string_view>
#include <spdlog/spdlog.h>

#define DELETE_COPY(Class) Class(const Class&) = delete; \
void operator=(const Class&) = delete

#define DELETE_MOVE(Class) Class(Class&&) = delete; \
void operator=(Class&&) = delete

#define DELETE_COPY_AND_MOVE(Class) Class(const Class&) = delete; \
void operator=(const Class&) = delete; \
Class(Class&&) = delete; \
void operator=(Class&&) = delete

#define ASSERT_VK_SUCCESS(res, msg) __vk_util_assert(__FILE__, __LINE__, res, msg)

constexpr void __vk_util_assert(const std::string &file, const int &line, VkResult res, const std::string_view& msg)
{
    if(res == VK_SUCCESS)
        return;

    spdlog::critical("[{}:{}]\n{}\nVkResult: {}", file, line, msg.cbegin(), res);
    throw std::runtime_error("Bad Result");
}

template<typename T>
constexpr size_t elemSize(size_t arraySize) {
    return arraySize / sizeof(T);
}