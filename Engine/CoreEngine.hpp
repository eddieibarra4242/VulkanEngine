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

#include "ecs/ECS.hpp"
#include "ecs/ECSSystem.hpp"
#include "window.hpp"
#include "rendering/Device.hpp"
#include "rendering/RenderingEngine.hpp"

class CoreEngine
{
  public:
    CoreEngine(Window& window, float fixedFPS = 60.0f, VkPhysicalDeviceFeatures targetFeatures = {}) : m_window(window),
                                                                                                       m_device{ window.context(), window.surface(), targetFeatures },
                                                                                                       m_renderingEngine{ window.surface(), m_device },
                                                                                                       m_frameTime{ 1.0f / fixedFPS } {}

    void run();

    constexpr ECS& scene() { return m_scene; }
    constexpr const ECS& scene() const { return m_scene; }

    inline void addUpdateSystem(ECSSystem* system) { m_updateSystems.push_back(system); }
    inline void addRenderSystem(ECSSystem* system) { m_renderSystems.push_back(system); }

  private:
    const Window& m_window;
    ECS m_scene;

    std::vector<ECSSystem*> m_updateSystems;
    std::vector<ECSSystem*> m_renderSystems;

    Device m_device;
    RenderingEngine m_renderingEngine;

    float m_frameTime;
};
