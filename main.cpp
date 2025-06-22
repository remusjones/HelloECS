#include "ECS/HelloECS.h"
#include <cassert>
#include <iostream>
#include <memory>

struct Position
{
    int x = 0;
    int y = 0;

    void Print() const
    {
        std::cout << "Position: (" << x << ", " << y << ")\n";
    }
};

int main()
{
    std::unique_ptr<ECS<HelloECS>> ecs = std::make_unique<HelloECS>();

    const bool initSuccess = ecs->InitECS();
    assert(initSuccess);

    {
        std::array<EntityHandle, 1000> entityHandles{};

        // Initialize the data
        std::cout << "Creating entities and components\n";
        for (size_t i = 0; i < entityHandles.max_size(); ++i)
        {
            const EntityHandle entity = ecs->CreateEntity();
            entityHandles[i] = entity;
            Position position{static_cast<int>(i), static_cast<int>(i * 2)};
            ecs->AddComponent(entity, position);
        }

        // Print the data fetched from the ECS
        std::cout << "Printing Original Data for entities\n";
        for (size_t i = 0; i < entityHandles.max_size(); ++i)
        {
            const Position* position = ecs->GetComponent<Position>(entityHandles[i]);
            assert(position != nullptr);
            position->Print();
        }

        // Mutate the data
        std::cout << "Mutating Data\n";
        for (const auto& entity : entityHandles)
        {
            Position* position = ecs->GetComponent<Position>(entity);
            position->x *= 10;
        }

        // fetch and read the mutated data from the ecs
        std::cout << "Printing Mutated Data for entities\n";
        for (size_t i = 0; i < entityHandles.max_size(); ++i)
        {
            const Position* position = ecs->GetComponent<Position>(entityHandles[i]);
            assert(position != nullptr);
            position->Print();
        }

    }
    const bool shutdownSuccess = ecs->ShutdownECS();
    assert(shutdownSuccess);
    return 0;
}