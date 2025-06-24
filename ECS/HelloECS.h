//
// Created by Remus on 21/06/2025.
//

#pragma once
#include "CommonTypes.h"
#include "ECS.h"

#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

class IComponentInterface {
public:
    virtual ~IComponentInterface() = default;
};

template <typename TComponent>
class ComponentContainer final : public IComponentInterface
{
public:
    ComponentContainer() = default;

    TComponent& AddComponent(const EntityHandle& entity, TComponent& component)
    {
        entityComponentMap[entity] = static_cast<unsigned int>(components.size());
        components.push_back(std::move(component));
        entities.push_back(entity);
        return components.back();
    }

    TComponent* Get(const EntityHandle& Entity) {

        return Contains(Entity) ? &components[entityComponentMap[Entity]] : nullptr;
    }

    bool Contains(const EntityHandle& entity) const
    {
        return entityComponentMap.find(entity) != entityComponentMap.end();
    }

    void Remove(const EntityHandle& entity)
    {
        int cID = entityComponentMap[entity];

        components[cID] = std::move(components.back());
        entities[cID] = entities.back();
        entityComponentMap[entities.back()] = cID;

        entityComponentMap.erase(entity);
        components.pop_back();
        entities.pop_back();
    }

    void Clear()
    {
        entityComponentMap.clear();
        components.clear();
        entities.clear();
    }

    size_t Size() const
    {
        return entityComponentMap.size();
    }

private:
    std::vector<EntityHandle> entities;
    std::vector<TComponent> components;

    // todo: this is here until sparse impl
    std::unordered_map<EntityHandle, ComponentHandle> entityComponentMap;
};

class HelloECS final : public ECS<HelloECS>
{
    static uint64_t handleCount;

public:
    bool InitECS();
    bool ShutdownECS();

    template<typename TComponent>
    void AddComponent(EntityHandle entity, TComponent& component)
    {
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        if (componentContainers.find(componentTypeId) == componentContainers.end())
        {
            componentContainers[componentTypeId] = std::make_unique<ComponentContainer<TComponent>>();
        }

        static_cast<ComponentContainer<TComponent>*>(componentContainers[componentTypeId].get())->AddComponent(entity, component);
    }

    template<typename TComponent>
    TComponent* GetComponent(EntityHandle entity)
    {
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        return static_cast<ComponentContainer<TComponent>*>(componentContainers[componentTypeId].get())->Get(entity);
    }

    template<typename... TComponentTypes>
    inline IEntityView<TComponentTypes...>& GetView()
    {
        IEntityView<TComponentTypes...> view;
        for (const auto& typeIndex : view.GetTypeIndices())
        {
            // Todo: Implement view
        }

        return view;
    }

    EntityHandle CreateEntity();

private:
    std::vector<EntityHandle> entities;

    // todo: this is here until sparse impl
    std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentInterface>> componentContainers;
};

