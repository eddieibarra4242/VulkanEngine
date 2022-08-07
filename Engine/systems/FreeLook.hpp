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

#include "../ecs/ECSSystem.hpp"
#include "../window.hpp"

class FreeLook : public ECSSystem
{
  public:
    FreeLook(Window& window, float sensitivity = 50.0f, bool invertY = false);

    virtual void update(float delta, Entity_t entity) override;

  private:
    Window& m_inputContext;

    bool m_locked;
    float m_sensitivity;

    bool m_invertY;

    inline void center()
    {
        glfwSetCursorPos(m_inputContext.glfwWindowPtr(), static_cast<double>(m_inputContext.width()) / 2.0, static_cast<double>(m_inputContext.height()) / 2.0);
    }
};
