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

#include "FreeMove.hpp"

#include "../components/Transform.hpp"
#include "../components/Camera.hpp"
#include <GLFW/glfw3.h>

static glm::vec3 RIGHT{ 1, 0, 0 };
static glm::vec3 FORWARD{ 0, 0, 1 };

FreeMove::FreeMove(Window& window, float speed) : m_inputContext(window), m_speed(speed)
{
    addComponentType(Transform::ID);
    addComponentType(Camera::ID);
}

void FreeMove::update(float delta, Entity_t entity)
{
    Transform& transform = *get<Transform>(entity);

    if (glfwGetKey(m_inputContext.glfwWindowPtr(), GLFW_KEY_W) == GLFW_PRESS) {
        transform.m_position += glm::normalize(transform.m_orientation * -FORWARD) * delta * m_speed;
    }

    if (glfwGetKey(m_inputContext.glfwWindowPtr(), GLFW_KEY_S) == GLFW_PRESS) {
        transform.m_position += glm::normalize(transform.m_orientation * FORWARD) * delta * m_speed;
    }

    if (glfwGetKey(m_inputContext.glfwWindowPtr(), GLFW_KEY_A) == GLFW_PRESS) {
        transform.m_position += glm::normalize(transform.m_orientation * -RIGHT) * delta * m_speed;
    }

    if (glfwGetKey(m_inputContext.glfwWindowPtr(), GLFW_KEY_D) == GLFW_PRESS) {
        transform.m_position += glm::normalize(transform.m_orientation * RIGHT) * delta * m_speed;
    }
}
