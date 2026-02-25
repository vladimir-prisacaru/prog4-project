#pragma once

#include "GameObject.h"

namespace dae
{
    /* Rotates the object around its parent */
    class RotatorComponent : public Component
    {
        public:

        RotatorComponent(GameObject* owner) : Component(owner) { };
        virtual ~RotatorComponent() = default;



        void Update(float deltaTime) override;

        void SetAngularSpeed(float speed);
        void SetRotationRadius(float radius);

        private:

        float m_Angle { };
        float m_AngularSpeed { };
        float m_Radius { };
    };
}