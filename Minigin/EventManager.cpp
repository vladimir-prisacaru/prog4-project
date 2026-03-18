#include "EventManager.h"

#include <algorithm>

namespace dae
{
    void EventManager::AddListener(GameEvent eventType, IObserver* observer)
    {
        if (!observer) return;

        auto& list = GetOrCreateList(eventType);

        if (std::find(list.begin(), list.end(), observer) == list.end())
            list.push_back(observer);
    }

    void EventManager::RemoveListener(GameEvent eventType, IObserver* observer)
    {
        const auto key = static_cast<uint32_t>(eventType);
        auto it = m_Listeners.find(key);
        if (it == m_Listeners.end()) return;

        auto& list = it->second;
        auto  pos = std::find(list.begin(), list.end(), observer);
        if (pos != list.end())
            list.erase(pos);
    }

    void EventManager::NotifyObservers(const Event& event)
    {
        DispatchToListeners(event);
    }

    void EventManager::QueueEvent(const Event& event)
    {
        m_EventQueue.push(event);
    }

    void EventManager::FlushEvents()
    {
        std::queue<Event> toProcess;
        std::swap(toProcess, m_EventQueue);

        while (!toProcess.empty())
        {
            DispatchToListeners(toProcess.front());
            toProcess.pop();
        }
    }

    EventManager::ObserverList& EventManager::GetOrCreateList(GameEvent type)
    {
        return m_Listeners[static_cast<uint32_t>(type)];
    }

    void EventManager::DispatchToListeners(const Event& event)
    {
        const auto key = static_cast<uint32_t>(event.id);

        auto it = m_Listeners.find(key);
        if (it != m_Listeners.end())
        {
            auto listCopy = it->second;
            for (auto* observer : listCopy)
                observer->Notify(event);
        }
    }
}