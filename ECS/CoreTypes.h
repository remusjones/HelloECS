//
// Created by Remus on 21/06/2025.
//

#pragma once
#include <bitset>
#include <cstdint>

using ComponentType = uint8_t;
using ComponentTypeId = uint64_t;
using EntityHandle = uint64_t;

constexpr EntityHandle INVALID_ENTITY = 0;

/* Defines the amount of entities that can be created */
#ifndef MAX_ENTITIES
constexpr size_t MAX_ENTITIES = 5000;
#endif

/* Defines how many components the ECS can register */
#ifndef MAX_COMPONENTS
constexpr ComponentType MAX_COMPONENTS = 32;
#endif

/* Adds a conditional in the AddComponent which checks if the component container exists, if not it adds it. This will add a branch in a potential hot-path */
#ifndef AUTO_REGISTRATION
constexpr bool AUTO_REGISTRATION = false;
#endif

using ComponentMask = std::bitset<MAX_COMPONENTS>;