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

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

struct ComponentInfo
{
    uint32_t m_id;
    uint32_t m_index;
};

struct EntityDef
{
    uint32_t m_index;
    std::vector<ComponentInfo> m_infos;

    EntityDef(uint32_t index) : m_index(index) {}
};

class BaseECSComponent;
typedef EntityDef* Entity_t;
typedef void (*ECSFreeFunc_t)(BaseECSComponent*);

struct TypeInfo
{
    ECSFreeFunc_t m_freefn;
    size_t m_size;

    TypeInfo(ECSFreeFunc_t freefn, size_t size) : m_freefn(freefn), m_size(size) {}
};

class BaseECSComponent
{
  public:
    Entity_t m_entity;

    [[nodiscard]] static uint32_t registerComponentType(ECSFreeFunc_t freeFunc, size_t size);

    [[nodiscard]] inline static ECSFreeFunc_t getTypeFreeFunc(uint32_t id)
    {
        if (componentTypes.get() == nullptr || !validateType(id)) {
            return 0;
        }

        return componentTypes->at(id).m_freefn;
    }

    [[nodiscard]] inline static size_t getTypeSize(uint32_t id)
    {
        if (componentTypes.get() == nullptr || !validateType(id)) {
            return 0;
        }

        return componentTypes->at(id).m_size;
    }

    [[nodiscard]] inline static bool validateType(uint32_t id)
    {
        if (componentTypes.get() == nullptr) {
            return false;
        }

        return id < componentTypes->size();
    }

  private:
    static std::unique_ptr<std::vector<TypeInfo>> componentTypes;
};

template<typename T>
struct ECSComponent : public BaseECSComponent
{
    static const ECSFreeFunc_t FREE_FUNC;
    static const uint32_t ID;
    static const size_t SIZE;
};

template<typename Component>
[[nodiscard]] ComponentInfo copyComponent(std::vector<uint8_t>& memory, Component* component, size_t index)
{
    memory.resize(index + Component::SIZE);
    new (&memory[index]) Component(*component);
    return ComponentInfo{ Component::ID, static_cast<uint32_t>(index) };
}

template<typename Component>
void destroyComponent(BaseECSComponent* component)
{
    reinterpret_cast<Component*>(component)->~Component();
}

template<typename T>
const ECSFreeFunc_t ECSComponent<T>::FREE_FUNC{ &destroyComponent<T> };

template<typename T>
const uint32_t ECSComponent<T>::ID{ BaseECSComponent::registerComponentType(&destroyComponent<T>, sizeof(T)) };

template<typename T>
const size_t ECSComponent<T>::SIZE{ sizeof(T) };
