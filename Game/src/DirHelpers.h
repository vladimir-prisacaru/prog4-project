#pragma once

#include <glm/glm.hpp>

namespace dae
{
    inline constexpr int INVALID_DIR { -1 };
    inline constexpr int DIR_U { 0 };
    inline constexpr int DIR_R { 1 };
    inline constexpr int DIR_D { 2 };
    inline constexpr int DIR_L { 3 };

    // Returns the cardinal direction index (0=up,1=right,2=down,3=left) from a direction vector
    inline int GetDirInt(glm::vec2 dir)
    {
        if (glm::length(dir) < 0.01f)
            return DIR_R; // default: right

        dir = glm::normalize(dir);

        const glm::vec2 u { 0.0f, -1.0f };
        const glm::vec2 r { 1.0f,  0.0f };
        const glm::vec2 d { 0.0f,  1.0f };
        const glm::vec2 l { -1.0f,  0.0f };

        float bestDot { glm::dot(dir, u) };
        int bestIdx { DIR_U };

        const float dotR { glm::dot(dir, r) };
        if (dotR > bestDot) { bestDot = dotR; bestIdx = DIR_R; }

        const float dotD { glm::dot(dir, d) };
        if (dotD > bestDot) { bestDot = dotD; bestIdx = DIR_D; }

        const float dotL { glm::dot(dir, l) };
        if (dotL > bestDot) { bestIdx = DIR_L; }

        return bestIdx;
    }

    // Returns the direction index (0=up,1=right,2=down,3=left) from a row/col delta
    inline constexpr int GetDirFromDelta(int dRow, int dCol)
    {
        if (dRow == 0)
        {
            if (dCol < 0)
                return DIR_L;
            else if (dCol > 0)
                return DIR_R;
        }
        else if (dCol == 0)
        {
            if (dRow < 0)
                return DIR_U;
            else if (dRow > 0)
                return DIR_D;
        }

        return INVALID_DIR;
    }

    // Returns the opposite cardinal direction
    inline constexpr int OppositeDir(int dir)
    {
        switch (dir)
        {
            case DIR_U: return DIR_D;
            case DIR_R: return DIR_L;
            case DIR_D: return DIR_U;
            case DIR_L: return DIR_R;
        }

        return INVALID_DIR;
    }
}