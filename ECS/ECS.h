//
// Created by Remus on 21/06/2025.
//

#pragma once
#include "CommonTypes.h"

#include <array>
#include <functional>
#include <typeindex>

template <typename... TComponentTypes>
class IEntityView
{
public:
    static std::array<std::type_index, sizeof...(TComponentTypes)> GetTypeIndices()
    {
        return { std::type_index(typeid(TComponentTypes))... };
    }

    void Each(const std::function<void(TComponentTypes&...)>& func)
    {
        // Todo: Implement view iteration logic
    }
};

template<typename TDerived>
class ECS
{
public:
    ~ECS() = default;

    bool InitECS()              { return static_cast<TDerived*>(this)->InitECS(); }
    bool ShutdownECS()          { return static_cast<TDerived*>(this)->ShutdownECS(); }
    EntityHandle CreateEntity() { return static_cast<TDerived*>(this)->CreateEntity(); }

    template<typename TComponent>
    void AddComponent(EntityHandle entity, TComponent& component)
    {
        static_cast<TDerived*>(this)->AddComponent(entity, component);
    }

    template<typename TComponent>
    TComponent* GetComponent(EntityHandle entity)
    {
        return static_cast<TDerived*>(this)->template GetComponent<TComponent>(entity);
    }

    template<typename... TComponentTypes>
    inline IEntityView<TComponentTypes...>& GetView()
    {
        return static_cast<TDerived*>(this)->template GetView<TComponentTypes...>();
    }

};