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

class IComponentInterface {
public:
    virtual ~IComponentInterface() = default;
};

template <typename TComponent>
class ComponentContainer final : public IComponentInterface
{
public:
    ComponentContainer() : size(0) {}

    TComponent& Add(const EntityHandle& entity, TComponent& component)
    {
        size_t newEntityIndex = size;

        entityToIndexMap[entity] = newEntityIndex;
        indexToEntityMap[newEntityIndex] = entity;
        components[newEntityIndex] = std::move(component);

        ++size;

        return components[newEntityIndex];
    }

    TComponent* Get(const EntityHandle& Entity)
    {
        return &components[entityToIndexMap[Entity]];
    }

    void Remove(const EntityHandle& entity)
    {
        size_t removeIndex = entityToIndexMap[entity];
        size_t lastIndex = --size;
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

class HelloECS
{
    template<typename...>
    friend class EntityView;

public:
    HelloECS();

    template <typename TComponent>
    void RegisterComponent()
    {
        ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        componentTypeMap.insert({componentTypeId, ++componentCount});
        componentContainers.insert({componentTypeId, std::make_shared<ComponentContainer<TComponent>>()});
    }

    template <typename TComponent>
    void AddComponent(const EntityHandle& entity, TComponent& component)
    {
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        if (!componentContainers.contains(componentTypeId)) RegisterComponent<TComponent>();

        auto container = GetContainer<TComponent>();
        container->Add(activeEntities[activeEntityToIndexMap[entity]], component);

        ComponentMask& componentMask = GetComponentMask(entity);
        componentMask.set(componentTypeMap[componentTypeId], true);
    }

    template<typename TComponent>
    TComponent* GetComponent(const EntityHandle& entity)
    {
        return GetContainer<TComponent>()->Get(activeEntities[activeEntityToIndexMap[entity]]);
    }

    template <typename TComponent>
    void Remove(const EntityHandle& entity)
    {
        auto container = GetContainer<TComponent>();
        container->Remove(entity);

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
        const EntityHandle newEntity = ++handleCount;
        const size_t newEntityIndex = ++activeEntitySize;

        activeEntities[newEntityIndex] = newEntity;
        componentMasks[newEntityIndex] = ComponentMask();

        activeEntityToIndexMap[newEntity] = newEntityIndex;

        return newEntity;
    }

    ComponentMask& GetComponentMask(const EntityHandle& entity)
    {
        return componentMasks[activeEntityToIndexMap[entity]];
    }

    void DestroyEntity(const EntityHandle& entity)
    {
        const size_t targetIndex = activeEntityToIndexMap[entity];
        const size_t endIndex = activeEntitySize;
        const EntityHandle swappedEntity = activeEntities[endIndex];

        // Intentionally allow swap here even if equal to avoid branching
        std::swap(activeEntities[targetIndex], activeEntities[endIndex]);
        std::swap(componentMasks[targetIndex], componentMasks[endIndex]);

        activeEntityToIndexMap[swappedEntity] = targetIndex;
        activeEntityToIndexMap.erase(entity);

        --activeEntitySize;
    }

private:
    EntityHandle handleCount;
    ComponentType componentCount;

    size_t activeEntitySize;
    std::array<EntityHandle, MAX_ENTITIES> activeEntities{};
    std::array<ComponentMask, MAX_ENTITIES> componentMasks{};

    std::unordered_map<EntityHandle, size_t> activeEntityToIndexMap;
    std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentInterface>> componentContainers{};
    std::unordered_map<ComponentTypeId, ComponentType> componentTypeMap;

    template<typename TComponent>
    std::shared_ptr<ComponentContainer<TComponent>> GetContainer()
    {
        const ComponentTypeId componentTypeId = typeid(TComponent).hash_code();
        return std::static_pointer_cast<ComponentContainer<TComponent>>(componentContainers[componentTypeId]);
    }
};


template <class... ComponentTypes>
struct ComponentTypeList {
    using ComponentTypeTuple = std::tuple<ComponentTypes...>;

    template <size_t Index>
    using Get = std::tuple_element_t<Index, ComponentTypeTuple>;

    static constexpr size_t size = sizeof...(ComponentTypes);
};

template <typename... TComponents>
class EntityView
{
public:
    EntityView(HelloECS* inECS) : ecs(inECS), viewPools{ecs->GetContainer<TComponents>().get()...}
    {
    }

private:
    friend class HelloECS;

    HelloECS* ecs;
    std::array<IComponentInterface*, sizeof...(TComponents)> viewPools;

    template<size_t Index>
    auto GetPoolAt()
    {
        using ComponentType = typename ComponentTypeList<TComponents...>::template Get<Index>;
        return static_cast<ComponentContainer<ComponentType>*>(viewPools[Index]);
    }

    template <size_t... Indices>
    auto MakeComponentTuple(EntityHandle id, std::index_sequence<Indices...>)
    {
        return std::forward_as_tuple((*GetPoolAt<Indices>()->Get(id))...);
    }

    template <typename Function>
    void ForEachImpl(Function func)
    {
        constexpr auto componentIds = std::make_index_sequence<sizeof...(TComponents)>{};
        ComponentMask viewComponentMask{};
        for (const auto& typeID : GetTypeIndices())
        {
            viewComponentMask.set(ecs->componentTypeMap[typeID], true);
        }

        for (size_t i = 0; i < ecs->activeEntitySize; ++i)
        {
            if ((ecs->componentMasks[i] & viewComponentMask) != viewComponentMask) continue;

            EntityHandle entity = ecs->activeEntities[i];
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