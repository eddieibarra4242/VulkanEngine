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

#include "FreeLook.hpp"

#include "../components/Transform.hpp"
#include "../components/Camera.hpp"

#include <glm/glm.hpp>

static constexpr int MOUSE_LEFT = 0;
static constexpr int MOUSE_RIGHT = 1;

static glm::vec3 UP{ 0, 1, 0 };
static glm::vec3 RIGHT{ 1, 0, 0 };

FreeLook::FreeLook(Window& window, float sensitivity, bool invertY) : m_inputContext(window), m_locked(false), m_sensitivity(sensitivity), m_invertY(invertY)
{
    addComponentType(Transform::ID);
    addComponentType(Camera::ID);
}

void FreeLook::update(float delta, Entity_t entity)
{
    using vec2d = glm::dvec2;

    Transform& transform = *get<Transform>(entity);

    if (glfwGetMouseButton(m_inputContext.glfwWindowPtr(), MOUSE_LEFT) && !m_locked) {
        glfwSetInputMode(m_inputContext.glfwWindowPtr(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        center();
        m_locked = true;
    }

    if (glfwGetMouseButton(m_inputContext.glfwWindowPtr(), MOUSE_RIGHT) && m_locked) {
        glfwSetInputMode(m_inputContext.glfwWindowPtr(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_locked = false;
    }

    if (m_locked) {
        vec2d diff;
        glfwGetCursorPos(m_inputContext.glfwWindowPtr(), &diff.x, &diff.y);

        // normalize diff
        diff *= vec2d{ 2.0 / static_cast<double>(m_inputContext.width()), 2.0 / static_cast<double>(m_inputContext.height()) };

        diff -= 1.0;

        // apply sensitivity
        diff *= -m_sensitivity * delta;

        if (m_invertY) {
            diff.y *= -1;
        }

        if (diff.x != 0) {
            transform.m_orientation = glm::normalize(glm::angleAxis(static_cast<float>(diff.x), UP) * transform.m_orientation);
        }

        if (diff.y != 0) {
            glm::vec3 right = transform.m_orientation * RIGHT;
            transform.m_orientation = glm::normalize(glm::angleAxis(static_cast<float>(diff.y), right) * transform.m_orientation);
        }

        center();
    }
}
