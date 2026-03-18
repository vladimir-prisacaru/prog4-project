#pragma once

#include <cstdint>

namespace dae
{
    // Scoped enum for all game events
    enum class GameEvent : uint32_t
    {
        PlayerDied,
        PointsGained,
        EnemyDied,
        PickupCollected,
        GameOver,
        ScoreThresholdReached,
    };

    // Generic event payload sent to observers
    struct Event
    {
        GameEvent id;
        int value1 { 0 }; // Optional payload
        int value2 { 0 }; // Optional payload

        explicit Event(GameEvent eventId, int payload1 = 0, int payload2 = 0)
            : id(eventId), value1(payload1), value2(payload2)
        { }
    };
}