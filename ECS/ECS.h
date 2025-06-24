//
// Created by Remus on 21/06/2025.
//

#pragma once
#include "CommonTypes.h"

#include <array>
#include <functional>
#include <typeindex>

template <class... Types>
struct TypeList {
    using TypeTuple = std::tuple<Types...>;

    template <size_t Index>
    using Get = std::tuple_element_t<Index, TypeTuple>;

    static constexpr size_t size = sizeof...(Types);
};

template <typename... TComponentTypes>
class IEntityView
{
private:
    using ComponentTypes = TypeList<TComponentTypes...>;

    template <size_t... Indices>
    auto MakeComponentTuple(EntityHandle id, std::index_sequence<Indices...>)
    {
        // Get adjacent type store
    }

    template <typename Function>
    void ForEachImpl(Function func)
    {
        // Iterate over entity list, and check against type list
    }

public:
    using ForEachFunc = std::function<void(EntityHandle, TComponentTypes&...)>;
    static std::array<std::type_index, sizeof...(TComponentTypes)> GetTypeIndices()
    {
        return { std::type_index(typeid(TComponentTypes))... };
    }

    void Each(ForEachFunc Function)
    {
        ForEachImpl(Function);
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
    IEntityView<TComponentTypes...>& GetView()
    {
        return static_cast<TDerived*>(this)->template GetView<TComponentTypes...>();
    }

};