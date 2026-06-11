#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <optional>

#include "ICollider.h"



namespace dae
{
    struct Path
    {
        std::vector<glm::vec2> nodes;
    };

    class Graph final : public ICollider
    {
        public:

        Graph() = default;
        ~Graph() = default;

        bool IsEnabled() override { return true; /* always on */ };
        void SetEnabled(bool) override { /* do nothing */ };

        RaycastHit Raycast(Ray ray, float maxDist) override;
        bool CheckOverlap(ICollider* other, bool& supported) override;

        /* Adds a node at pos, returns the index of the new (or existing) node */
        int AddNode(glm::vec2 pos);
        /* Removes the node at index and all connections to it */
        void RemoveNode(int index);
        /* Adds a bidirectional connection between nodes a and b */
        void AddConnection(int a, int b);
        /* Removes the connection between nodes a and b */
        void RemoveConnection(int a, int b);
        /* Returns the index of the closest node within radius of pos, or -1 if none */
        int FindNode(glm::vec2 pos, float radius = 0.5f) const;

        /* Finds shortest path from startPos to endPos,
           path.nodes does NOT include startPos, but DOES include endPos */
        Path FindPath(glm::vec2 startPos, glm::vec2 endPos);
        /* Makes the longest path from connected nodes up to a certain depth,
           that includes the closest node to startPos */
        Path GetConnectedPath(glm::vec2 startPos, int depth);

        const std::vector<glm::vec2>& GetNodes() const { return m_Nodes; }
        const std::vector<std::pair<int, int>>& GetConnections() const { return m_Connections; }

        private:

        /* Returns t along the ray at which it intersects segment [a, b] */
        static std::optional<float> RaySegmentIntersect(
            Ray ray, float maxDist, glm::vec2 a, glm::vec2 b, float& outS);

        std::vector<glm::vec2> m_Nodes { };
        std::vector<std::pair<int, int>> m_Connections { };
    };
}