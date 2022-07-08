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

#include <cstdlib>
#include "window.hpp"

#include "rendering/Device.hpp"
#include "rendering/RenderingEngine.hpp"

#include "rendering/Pipeline.hpp"

#include "rendering/Mesh.hpp"

int main()
{
    Window window(1280, 720, "Vk App");
    VkPhysicalDeviceFeatures targetFeatures{};
    Device device{ window.context(), window.surface(), targetFeatures };

    RenderingEngine engine{ window.surface(), device };

    Model monkey{ "./res/monkey3.obj" };
    monkey.finalize();
    engine.addMesh(std::make_shared<Mesh>(device, monkey.getVertices(), monkey.getIndices()));

    while (!window.shouldClose()) {
        glfwPollEvents();
        engine.render();
        engine.present();
    }

    return EXIT_SUCCESS;
}