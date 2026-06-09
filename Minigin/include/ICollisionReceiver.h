#pragma once

#include "ICollider.h"

namespace dae
{
    class ICollisionReceiver
    {
        public:

        virtual ~ICollisionReceiver() = default;

        virtual void OnOverlap(ICollider* other) = 0;
        virtual void OnOverlapEnd(ICollider* other) = 0;
    };
}