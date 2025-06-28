#include "ECS/HelloECS.h"
#include <cassert>
#include <iostream>
#include <memory>

struct Position
{
    int x = 0;
    int y = 0;

    void Print(const EntityHandle& ID) const
    {
        std::cout << ID << " - Position: (" << x << ", " << y << ")\n";
    }
};

struct Rotation
{
    int facing = 0;

    void Print(const EntityHandle& ID) const
    {
        std::cout << ID << " - Facing: (" << facing << ")\n";
    }
};

int main()
{
    std::unique_ptr<HelloECS> ecs = std::make_unique<HelloECS>();
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

            if (i % 3 == 0)
            {
                Rotation rotation {static_cast<int>(i * 100)};
                ecs->AddComponent(entity, rotation);
            }
        }

        // Print the data fetched from the ECS
        std::cout << "Printing Original Data for entities\n";
        for (size_t i = 0; i < entityHandles.max_size(); ++i)
        {
            const Position* position = ecs->GetComponent<Position>(entityHandles[i]);
            assert(position != nullptr);
            position->Print(entityHandles[i]);
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
            position->Print(entityHandles[i]);
        }

        // print our position view
        std::cout << "Printing View Data for entities\n";
        auto positionView = ecs->GetView<Position>();
        positionView.Each([&](const EntityHandle& entity, const Position& position) {
            position.Print(entity);
        });

        // print our position + rotation view, and mutate the state
        auto positionRotationView = ecs->GetView<Position, Rotation>();
        positionRotationView.Each([&](const EntityHandle& entity, Position& position, Rotation& rotation)
        {
            position.Print(entity);
            rotation.Print(entity);

            rotation.facing = -100 * static_cast<int>(entity);
            position.x = -1 * static_cast<int>(entity);
        });

        // print our updated Position and Rotation
        auto positionRotationView2 = ecs->GetView<Position, Rotation>();
        positionRotationView2.Each([&](const EntityHandle& entity, const Position& position, const Rotation& rotation)
        {
            position.Print(entity);
            rotation.Print(entity);
        });

    }
    return 0;
}