cd ../

clang-format -i ./Engine/*.hpp
clang-format -i ./Engine/*.cpp

clang-format -i ./Engine/rendering/*.hpp
clang-format -i ./Engine/rendering/*.cpp

clang-format -i ./Engine/ecs/*.hpp
clang-format -i ./Engine/ecs/*.cpp

clang-format -i ./Engine/components/*.hpp
clang-format -i ./Engine/components/*.cpp

clang-format -i ./Engine/systems/*.hpp
clang-format -i ./Engine/systems/*.cpp
