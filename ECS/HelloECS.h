//
// Created by Remus on 21/06/2025.
//

#pragma once
#include "CoreTypes.h"
#include <cassert>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>

template<typename... TComponents>
class EntityView;

/*
 * Base Component Container
 * Used to store pointers, and for component ComponentContainer<T> upcasting
 */
class IComponentContainerInterface {
public:
    virtual ~IComponentContainerInterface() = default;
};

/*
 * Component Container
 * Contains sparse arrays for contiguous component arrays
 */
template <typename TComponent>
class ComponentContainer final : public IComponentContainerInterface
{
public:
    ComponentContainer() : size(0) {}

    TComponent* Add(const EntityHandle& entity, TComponent& component)
    {
        // Adds the new component to the array, and adds the lookups
        size_t newEntityIndex = size;

        entityToIndexMap[entity] = newEntityIndex;
        indexToEntityMap[newEntityIndex] = entity;
        components[newEntityIndex] = std::move(component);

        ++size;

        return &components[newEntityIndex];
    }

    TComponent* Get(const EntityHandle& Entity)
    {
        return &components[entityToIndexMap[Entity]];
    }

    void Remove(const EntityHandle& entity)
    {
        // Swaps the removed entity to the last index, and updates the maps
        size_t removeIndex = entityToIndexMap[entity];
        size_t lastIndex = size--;
        components[removeIndex] = std::move(components[lastIndex]);

        const EntityHandle swappedEntity = indexToEntityMap[lastIndex];
        entityToIndexMap[swappedEntity] = removeIndex;
        indexToEntityMap[removeIndex] = swappedEntity;

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(lastIndex);
    }

    size_t Size() const
    {
        return size;
    }

private:

    std::array<TComponent, MAX_ENTITIES> components;
    std::unordered_map<EntityHandle, size_t> entityToIndexMap;
    std::unordered_map<size_t, EntityHandle> indexToEntityMap;

    size_t size;
};

/*
 * Core ECS API
 * Combines the "Entity Manager", and the "Component Manager" concepts
 * Holds and operates on the Entity > Component Array lookups
 */
class HelloECS
{
    template<typename...>
    friend class EntityView;

public:
    HelloECS() : handleCount(0), componentCount(0), activeEntitySize(0)
    {
        activeEntities.fill(INVALID_ENTITY);
    }

    template <typename TComponent>
    void RegisterComponent()
    {
        // Gets the hash code for the provided Type, and registers the container
        ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        componentTypeMap.insert({componentTypeId, ++componentCount});
        componentContainers.insert({componentTypeId, std::make_unique<ComponentContainer<TComponent>>()});
    }

    template <typename TComponent>
    TComponent* AddComponent(const EntityHandle& entity, TComponent& component)
    {
        // Branch and register the component if the type isn't already registered
        // This could be explicit instead to avoid this runtime branch in a hotpath
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        if (!componentContainers.contains(componentTypeId)) RegisterComponent<TComponent>();

        // Add the component to the entity
        auto container = GetContainer<TComponent>();
        TComponent* newComponent = container->Add(activeEntities[activeEntityToIndexMap[entity]], component);

        // Update the component mask
        ComponentMask& componentMask = GetComponentMask(entity);
        componentMask.set(componentTypeMap[componentTypeId], true);

        return newComponent;
    }

    template<typename TComponent>
    TComponent* GetComponent(const EntityHandle& entity)
    {
        return GetContainer<TComponent>()->Get(activeEntities[activeEntityToIndexMap[entity]]);
    }

    template <typename TComponent>
    void RemoveComponent(const EntityHandle& entity)
    {
        // Removes the data to this entity
        auto container = GetContainer<TComponent>();
        container->Remove(entity);

        // Update the component mask
        ComponentMask& componentMask = GetComponentMask(entity);
        componentMask.set(componentTypeMap[typeid(TComponent).hash_code()], false);
    }

    template<typename... TComponents>
    EntityView<TComponents...> GetView()
    {
        return EntityView<TComponents...>(this);
    }

    EntityHandle CreateEntity()
    {
        // Increment sizes, and entity count
        const EntityHandle newEntity = ++handleCount;
        const size_t newEntityIndex = activeEntitySize;

        // Update arrays
        activeEntities[newEntityIndex] = newEntity;
        componentMasks[newEntityIndex] = ComponentMask();

        // Update the index to entity map
        activeEntityToIndexMap[newEntity] = newEntityIndex;

        activeEntitySize++;
        return newEntity;
    }

    ComponentMask& GetComponentMask(const EntityHandle& entity)
    {
        return componentMasks[activeEntityToIndexMap[entity]];
    }

    void DestroyEntity(const EntityHandle& entity)
    {
        // Find our entity, and our end entity
        const size_t targetIndex = activeEntityToIndexMap[entity];
        const size_t endIndex = activeEntitySize - 1;
        const EntityHandle swappedEntity = activeEntities[endIndex];

        // Swap the entity data to the end of the arrays
        // Intentionally allow swap here even if equal to avoid branching
        std::swap(activeEntities[targetIndex], activeEntities[endIndex]);
        std::swap(componentMasks[targetIndex], componentMasks[endIndex]);

        // update our swapped entity map
        activeEntityToIndexMap[swappedEntity] = targetIndex;
        activeEntityToIndexMap.erase(entity);

        --activeEntitySize;
    }

private:
    template<typename TComponent>
    ComponentContainer<TComponent>* GetContainer()
    {
        // Lookup our Component Container by using our component hash code
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        return static_cast<ComponentContainer<TComponent>*>(componentContainers[componentTypeId].get());
    }

    EntityHandle handleCount;
    ComponentType componentCount;

    size_t activeEntitySize;
    std::array<EntityHandle, MAX_ENTITIES> activeEntities{};
    std::array<ComponentMask, MAX_ENTITIES> componentMasks{};

    std::unordered_map<EntityHandle, size_t> activeEntityToIndexMap;
    std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentContainerInterface>> componentContainers{};
    std::unordered_map<ComponentTypeId, ComponentType> componentTypeMap;

};

/*
 * Helper class for managing our tuple types
 */
template <class... ComponentTypes>
struct ComponentTypeList {
    using ComponentTypeTuple = std::tuple<ComponentTypes...>;

    template <size_t Index>
    using Get = std::tuple_element_t<Index, ComponentTypeTuple>;

    static constexpr size_t size = sizeof...(ComponentTypes);
};

/*
 * Entity View
 * Used for requesting views into the ECS, such as a "matches" query which can be iterated on 'Each'
 */
template <typename... TComponents>
class EntityView
{
public:
    EntityView(HelloECS* inECS) : ecs(inECS), viewPools{ecs->GetContainer<TComponents>()...}
    {
    }

private:
    friend class HelloECS;

    HelloECS* ecs;
    std::array<IComponentContainerInterface*, sizeof...(TComponents)> viewPools;

    template<size_t Index>
    auto GetPoolAt()
    {
        // Gets the appropriate pool based on the input type index
        using ComponentType = typename ComponentTypeList<TComponents...>::template Get<Index>;
        return static_cast<ComponentContainer<ComponentType>*>(viewPools[Index]);
    }

    template <size_t... Indices>
    auto MakeComponentTuple(EntityHandle id, std::index_sequence<Indices...>)
    {
        // Combines all the component pools into a tuple for our each
        return std::forward_as_tuple((*GetPoolAt<Indices>()->Get(id))...);
    }

    template <typename Function>
    void ForEachImpl(Function func)
    {
        // Get our sequence of indices for our view pool
        constexpr auto componentIds = std::make_index_sequence<sizeof...(TComponents)>{};

        // Configure our views component mask
        ComponentMask viewComponentMask{};
        for (const auto& typeID : GetTypeIndices())
        {
            viewComponentMask.set(ecs->componentTypeMap[typeID], true);
        }

        // Iterate over all the active entities
        for (size_t i = 0; i < ecs->activeEntitySize; ++i)
        {
            // Compare the entity component mask to the view component mask, skip if it's not the same
            if ((ecs->componentMasks[i] & viewComponentMask) != viewComponentMask) continue;

            EntityHandle entity = ecs->activeEntities[i];

            // Invoke the function
            std::apply(func, std::tuple_cat(std::make_tuple(entity), MakeComponentTuple(entity, componentIds)));
        }
    }

public:

    using ForEachFunc = std::function<void(EntityHandle, TComponents &...)>;
    static std::array<ComponentTypeId, sizeof...(TComponents)> GetTypeIndices()
    {
        return { typeid(TComponents).hash_code()... };
    }

    void Each(ForEachFunc Function)
    {
        ForEachImpl(Function);
    }
};