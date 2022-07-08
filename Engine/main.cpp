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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int main()
{
    Window window(1280, 720, "Vk App");
    VkPhysicalDeviceFeatures targetFeatures{};
    Device device{ window.context(), window.surface(), targetFeatures };
    
    RenderingEngine engine { window.surface(), device };

    Model triangle;

    triangle.addFace(
        triangle.addVertex(glm::vec3(-1, 1, 0)),  
        triangle.addVertex(glm::vec3(0, 1, 0)),
        triangle.addVertex(glm::vec3(-.5f, -1, 0)));

    triangle.finalize();

    engine.addMesh(std::make_shared<Mesh>(device, triangle.getVertices(), triangle.getIndices()));

    Model triangle2;

    triangle2.addFace(
        triangle2.addVertex(glm::vec3(0, 1, 0)),  
        triangle2.addVertex(glm::vec3(1, 1, 0)),
        triangle2.addVertex(glm::vec3(.5f, -1, 0)));

    triangle2.finalize();
    
    engine.addMesh(std::make_shared<Mesh>(device, triangle2.getVertices(), triangle2.getIndices()));

    while (!window.shouldClose()) {
        glfwPollEvents();
        engine.render();
        engine.present();
    }

    return EXIT_SUCCESS;
}