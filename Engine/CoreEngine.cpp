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

#include "CoreEngine.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>
#include <bits/chrono.h>
#include <chrono>
#include <memory>
#include <ratio>
#include <spdlog/spdlog.h>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::system_clock::time_point Time;

constexpr float toSeconds(std::chrono::nanoseconds d)
{
    return static_cast<float>(d.count()) / 1000000000.0f;
}

CoreEngine::CoreEngine(Window& window, float fixedFPS, VkPhysicalDeviceFeatures targetFeatures) : m_window(window),
                                                                                                  m_device{ window.context(), window.surface(), targetFeatures },
                                                                                                  m_renderingEngine{ window.surface(), m_device },
                                                                                                  m_frameTime{ 1.0f / fixedFPS },
                                                                                                  m_cameraScrapper{ std::make_unique<CameraScrapper>(m_renderingEngine) }
{
    m_renderSystems.push_back(m_cameraScrapper.get());
}

void CoreEngine::run()
{
    bool isRunning = true;

    Time startTime = Clock::now();

    float unprocessedTime = 0.0f;
    float frameCountTime = 0.0f;

    int unprocessedFrames = 0;
    int updateFrames = 0;
    int renderFrames = 0;

    while (isRunning) {
        bool render = false;

        Time lastTime = Clock::now();
        float passedTime = toSeconds(lastTime - startTime);
        startTime = lastTime;

        unprocessedTime += passedTime;
        frameCountTime += passedTime;

        glfwPollEvents();

        while (unprocessedTime > m_frameTime) {
            if (m_window.shouldClose()) {
                isRunning = false;
            }

            m_scene.updateSystems(m_updateSystems, m_frameTime);

            unprocessedTime -= m_frameTime;
            render = true;
            updateFrames++;
        }

        if (render) {
            m_scene.updateSystems(m_renderSystems, m_frameTime);
            m_renderingEngine.render();
            m_renderingEngine.present();
            renderFrames++;
        } else {
            unprocessedFrames++;
        }

        if (frameCountTime >= 1.0f) {
            spdlog::debug("Unprocessed Frames: {}, Update Frames: {}, Render Frames: {}", unprocessedFrames, updateFrames, renderFrames);
            unprocessedFrames = 0;
            updateFrames = 0;
            renderFrames = 0;
            frameCountTime = 0.0f;
        }
    }
}
