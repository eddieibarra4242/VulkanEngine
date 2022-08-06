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

#include <cstddef>
#include <cstdlib>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include "window.hpp"

#include "rendering/Device.hpp"
#include "rendering/RenderingEngine.hpp"

#include "rendering/Pipeline.hpp"

#include "rendering/Mesh.hpp"

#include "ecs/ECSComponent.hpp"
#include "ecs/ECSSystem.hpp"
#include "ecs/ECS.hpp"

struct TestComponent : public ECSComponent<TestComponent>
{
    float x;
    float y;
};

class TestSystem : public ECSSystem
{
  public:
    TestSystem()
    {
        addComponentType(TestComponent::ID);
    }

    virtual void update(float delta, Entity_t entity) override
    {
        TestComponent* tc = get<TestComponent>(entity);
        spdlog::info("Test update called\n x: {} y: {} delta: {}", tc->x, tc->y, delta);
    }
};

int main()
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    Window window(1280, 720, "Vk App");
    VkPhysicalDeviceFeatures targetFeatures{};
    Device device{ window.context(), window.surface(), targetFeatures };

    RenderingEngine engine{ window.surface(), device };

    Model monkey{ "./res/monkey3.obj" };
    monkey.finalize();
    engine.addMesh(std::make_shared<Mesh>(device, monkey.getVertices(), monkey.getIndices()));

    // example ecs code
    ECS ecs;

    Entity_t e = ecs.createEntity();

    TestComponent c{ .x = 0.3f, .y = 0.15f };
    ecs.addComponent(e, c);

    TestSystem s;

    std::vector<ECSSystem*> systems;
    systems.push_back(&s);

    ecs.updateSystems(systems, 10.15f);

    while (!window.shouldClose()) {
        glfwPollEvents();
        engine.render();
        engine.present();
    }

    return EXIT_SUCCESS;
}
