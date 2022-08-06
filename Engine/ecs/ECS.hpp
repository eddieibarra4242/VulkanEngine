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

#include "ECSComponent.hpp"

#include <cstddef>
#include <unordered_map>
#include <cstdint>
#include <vector>

class ECSSystem;

class ECS
{
  public:
    ~ECS();

    // entities
    [[nodiscard]] Entity_t createEntity();
    void removeEntity(Entity_t entity);

    // components
    template<typename Component>
    void addComponent(Entity_t entity, Component& component)
    {
        auto& memory = m_components[Component::ID];
        component.m_entity = entity;
        entity->m_infos.push_back(copyComponent<Component>(memory, &component, memory.size()));
    }

    template<typename Component>
    bool removeComponent(Entity_t entity)
    {
        for (uint32_t i = 0; i < entity->m_infos.size(); i++) {
            auto& info = entity->m_infos[i];
            if (Component::ID != info.m_id) {
                continue;
            }

            auto& memory = m_components[Component::ID];
            size_t typeSize = Component::SIZE;
            size_t lastComponentIndex = memory.size() - typeSize;

            Component::FREE_FUNC(reinterpret_cast<BaseECSComponent*>(&memory[info.m_index]));// maybe unsafe ...

            if (info.m_index != lastComponentIndex) {
                Component* lastComponent = reinterpret_cast<Component*>(&memory[lastComponentIndex]);
                ComponentInfo newInfo = copyComponent<Component>(memory, lastComponent, info.m_index);

                for (ComponentInfo& lastCompEntityinfo : lastComponent->m_entity->m_infos) {
                    if (lastCompEntityinfo.m_id == newInfo.m_id) {
                        lastCompEntityinfo.m_index = newInfo.m_index;
                    }
                }
            }

            memory.resize(lastComponentIndex);

            return true;
        }

        return false;
    }

    template<typename Component>
    Component* get(Entity_t entity)
    {
        auto& memory = m_components[Component::ID];
        for (auto& info : entity->m_infos) {
            if (Component::ID == info.m_id) {
                return reinterpret_cast<Component*>(&memory[info.m_index]);
            }
        }

        return nullptr;
    }

    // systems
    void updateSystems(const std::vector<ECSSystem*>& systemList, float delta);

  private:
    std::unordered_map<uint32_t, std::vector<uint8_t>> m_components;
    std::vector<EntityDef> m_entities;

    void updateMultipleComponentSystem(ECSSystem* system, float delta, const std::vector<uint32_t>& types);
};
