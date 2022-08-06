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

#include "ECSComponent.hpp"
#include <memory>

std::unique_ptr<std::vector<TypeInfo>> BaseECSComponent::componentTypes;

uint32_t BaseECSComponent::registerComponentType(ECSFreeFunc_t freeFunc, size_t size)
{
    if (componentTypes.get() == nullptr) {
        componentTypes = std::make_unique<std::vector<TypeInfo>>();
    }

    uint32_t newID = static_cast<uint32_t>(componentTypes->size());
    componentTypes->emplace_back(freeFunc, size);
    return newID;
}
