#pragma once

#include "Singleton.h"
#include "Event.h"
#include "IObserver.h"

#include <vector>
#include <queue>
#include <unordered_map>

namespace dae
{
    class EventManager final : public Singleton<EventManager>
    {
        public:

        ~EventManager() override = default;

        // Subscribe to a specific event type.
        void AddListener(GameEvent eventType, IObserver* observer);

        // Unsubscribe from a specific event type.
        void RemoveListener(GameEvent eventType, IObserver* observer);

        // Dispatch immediately (synchronous)
        void NotifyObservers(const Event& event);

        // Buffer for later, dispatched when FlushEvents() is called
        void QueueEvent(const Event& event);

        // Called once per frame to drain the queue
        void FlushEvents();

        private:

        using ObserverList = std::vector<IObserver*>;

        std::unordered_map<uint32_t, ObserverList> m_Listeners;
        std::queue<Event> m_EventQueue;

        ObserverList& GetOrCreateList(GameEvent type);
        void DispatchToListeners(const Event& event);
    };
}