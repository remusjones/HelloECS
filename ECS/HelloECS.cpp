//
// Created by Remus on 21/06/2025.
//

#include "HelloECS.h"

uint64_t HelloECS::handleCount = 1;

bool HelloECS::InitECS()
{
    return true;
}

bool HelloECS::ShutdownECS()
{
    componentContainers.clear();
    return true;
}

EntityHandle HelloECS::CreateEntity()
{
    EntityHandle newEntityHandle = handleCount++;
    return entities.emplace_back(newEntityHandle);
}