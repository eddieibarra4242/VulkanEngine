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

#include <spdlogNoIW.h>

#include "components/Camera.hpp"
#include "components/Transform.hpp"
#include "ecs/ECSComponent.hpp"
#include "systems/FreeLook.hpp"
#include "systems/FreeMove.hpp"
#include "window.hpp"
#include "CoreEngine.hpp"

int main()
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    Window window(1920, 1080, "Vk App");
    CoreEngine engine(window, 144.0f);

    Entity_t player = engine.scene().createEntity();

    engine.scene().addComponent(player, Transform{});
    engine.scene().addComponent(player, Camera{});

    FreeLook lookSystem{ window, 5000.0f, true };
    FreeMove moveSystem{ window };

    engine.addUpdateSystem(&lookSystem);
    engine.addUpdateSystem(&moveSystem);

    engine.run();

    return EXIT_SUCCESS;
}
