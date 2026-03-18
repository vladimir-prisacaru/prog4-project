#pragma once

namespace dae
{
    struct Event;

    // Pure interface that every event listener must implement
    class IObserver
    {
        public:
        virtual ~IObserver() = default;
        virtual void Notify(const Event& event) = 0;

        protected:
        IObserver() = default;
    };
}