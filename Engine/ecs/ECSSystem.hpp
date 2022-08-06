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

#include <cstdint>
#include <vector>

#include "ECSComponent.hpp"
#include "ECS.hpp"

class ECSSystem
{
  public:
    static constexpr uint8_t OPTIONAL_BIT = 1;

    ECSSystem() = default;
    virtual ~ECSSystem() = default;

    virtual void update(float delta, Entity_t entity) = 0;// how do component access ???

    [[nodiscard]] constexpr const std::vector<uint32_t>& types() const
    {
        return m_componentTypes;
    }

    [[nodiscard]] constexpr const std::vector<uint8_t>& flags() const
    {
        return m_componentFlags;
    }

    [[nodiscard]] inline bool isValid() const
    {
        for (uint8_t flag : m_componentFlags) {// need at least one non-optional component.
            if ((flag & OPTIONAL_BIT) == 0) {
                return true;
            }
        }

        return false;
    }

    constexpr void setECS(ECS* parent) { m_parentECS = parent; }

  protected:
    template<typename Component>
    [[nodiscard]] Component* get(Entity_t entity) const
    {
        if (m_parentECS == nullptr) {
            return nullptr;
        }

        return m_parentECS->get<Component>(entity);
    }

    void addComponentType(uint32_t componentID, uint8_t flags = 0)
    {
        m_componentTypes.push_back(componentID);
        m_componentFlags.push_back(flags);
    }

  private:
    std::vector<uint32_t> m_componentTypes;
    std::vector<uint8_t> m_componentFlags;

    ECS* m_parentECS = nullptr;
};
