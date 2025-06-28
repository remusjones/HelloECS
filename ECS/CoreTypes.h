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
constexpr size_t MAX_ENTITIES = 5000;

constexpr ComponentType MAX_COMPONENTS = 32;

using ComponentMask = std::bitset<MAX_COMPONENTS>;