//
// Created by Remus on 21/06/2025.
//

#include "HelloECS.h"

HelloECS::HelloECS() : handleCount(0), componentCount(0), activeEntitySize(0)
{
    activeEntities.fill(INVALID_ENTITY);
}