#pragma once

#include "IObserver.h"
#include "Event.h"

#include <vector>
#include <algorithm>

namespace dae
{
    // Mixin class: inherit from Subject to make a class observable
    class Subject
    {
        public:

        Subject() = default;
        virtual ~Subject() = default;

        Subject(const Subject&) = delete;
        Subject& operator=(const Subject&) = delete;
        Subject(Subject&&) = delete;
        Subject& operator=(Subject&&) = delete;

        void AddObserver(IObserver* observer)
        {
            if (!observer) return;
            if (std::find(m_Observers.begin(), m_Observers.end(), observer)
                != m_Observers.end()) return;

            m_Observers.push_back(observer);
        }

        void RemoveObserver(IObserver* observer)
        {
            auto it = std::find(m_Observers.begin(), m_Observers.end(), observer);
            if (it != m_Observers.end())
                m_Observers.erase(it);
        }

        protected:

        void NotifyObservers(const Event& event) const
        {
            auto observersCopy = m_Observers;
            for (auto* observer : observersCopy)
                observer->Notify(event);
        }

        private:

        std::vector<IObserver*> m_Observers;
    };
}