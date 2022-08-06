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

#include "ECS.hpp"
#include "ECSComponent.hpp"
#include "ECSSystem.hpp"

ECS::~ECS()
{
    for (auto& entry : m_components) {
        size_t typeSize = BaseECSComponent::getTypeSize(entry.first);

        for (size_t i = 0; i < entry.second.size(); i += typeSize) {
            BaseECSComponent* component = reinterpret_cast<BaseECSComponent*>(&entry.second[i]);// maybe unused ...
            ECSFreeFunc_t freefn = BaseECSComponent::getTypeFreeFunc(entry.first);
            freefn(component);
        }
    }
}

Entity_t ECS::createEntity()
{
    size_t newEntityIndex = m_entities.size();
    m_entities.emplace_back(newEntityIndex);
    return &m_entities[newEntityIndex];
}

void ECS::removeEntity(Entity_t entity)
{
    size_t lastIndex = m_entities.size() - 1;

    if (entity->m_index != lastIndex) {
        m_entities[entity->m_index] = m_entities[lastIndex];
    }

    m_entities.pop_back();
}

void ECS::updateSystems(const std::vector<ECSSystem*>& systemList, float delta)
{
    for (auto system : systemList) {
        system->setECS(this);

        auto& types = system->types();
        if (types.size() == 1) {
            size_t typeSize = BaseECSComponent::getTypeSize(types[0]);
            auto& componentMemory = m_components[types[0]];

            for (size_t i = 0; i < componentMemory.size(); i += typeSize) {
                BaseECSComponent* comp = reinterpret_cast<BaseECSComponent*>(&componentMemory[i]);
                system->update(delta, comp->m_entity);
            }
        } else {
            updateMultipleComponentSystem(system, delta, types);
        }
    }
}

void ECS::updateMultipleComponentSystem(ECSSystem* system, float delta, const std::vector<uint32_t>& types)
{
    const auto& flags = system->flags();

    std::vector<uint32_t> nonOptionalTypes;
    std::vector<Entity_t> validEntities;

    for (size_t i = 0; i < flags.size(); i++) {
        if (flags[i] & ECSSystem::OPTIONAL_BIT) {
            continue;
        }

        nonOptionalTypes.push_back(types[i]);
    }

    for (auto& entity : m_entities) {
        bool shouldAdd = true;

        for (auto& type : nonOptionalTypes) {
            bool invalid = true;

            for (auto& info : entity.m_infos) {
                if (type == info.m_id) {
                    invalid = false;
                    break;
                }
            }

            if (invalid) {
                shouldAdd = false;
                break;
            }
        }

        if (shouldAdd) {
            validEntities.push_back(&entity);
        }
    }

    for (auto& entity : validEntities) {
        system->update(delta, entity);
    }
}
