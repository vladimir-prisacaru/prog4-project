#include "Graph.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>

namespace dae
{
    RaycastHit Graph::Raycast(Ray ray, float maxDist)
    {
        RaycastHit best { };
        best.hit = false;
        best.t = std::numeric_limits<float>::max();

        for (const auto& [iA, iB] : m_Connections)
        {
            const glm::vec2 a { m_Nodes[iA] };
            const glm::vec2 b { m_Nodes[iB] };

            float s { };
            auto tOpt { RaySegmentIntersect(ray, maxDist, a, b, s) };

            if (!tOpt.has_value())
                continue;

            const float t { tOpt.value() };

            if (t >= best.t)
                continue;

            best.hit = true;
            best.t = t;
            best.pos = ray.pos + ray.dir * t;
            best.collider = this;
            best.obj = nullptr;

            // Edge tangent and its perpendicular, oriented toward the ray origin
            const glm::vec2 edge { b - a };
            const glm::vec2 perp { -edge.y, edge.x };   // rotate 90 degrees CCW
            const glm::vec2 toRay { ray.pos - best.pos };
            best.normal = glm::dot(perp, toRay) >= 0.0f
                ? glm::normalize(perp)
                : glm::normalize(-perp);
        }

        return best;
    }

    bool Graph::CheckOverlap(ICollider*, bool& supported)
    {
        supported = false;
        return false;
    }

    Path Graph::FindPath(glm::vec2 startPos, glm::vec2 endPos)
    {
        // Copy to not modify original graph
        std::vector<glm::vec2> nodes { m_Nodes };
        std::vector<std::pair<int, int>> connections { m_Connections };

        // Inject temporary nodes
        const int startIdx { static_cast<int>(nodes.size()) };
        nodes.push_back(startPos);

        const int endIdx { static_cast<int>(nodes.size()) };
        nodes.push_back(endPos);

        auto connectTempNode = [&] (int tempIdx)
            {
                const glm::vec2 pos { nodes[tempIdx] };

                int bestIdx { -1 };
                float bestDist { std::numeric_limits<float>::max() };

                for (int i = 0; i < startIdx; ++i)
                {
                    const float dist { glm::length(nodes[i] - pos) };

                    if (dist < bestDist)
                    {
                        bestDist = dist;
                        bestIdx = i;
                    }
                }

                if (bestIdx != -1)
                    connections.emplace_back(tempIdx, bestIdx);
            };

        connectTempNode(startIdx);
        connectTempNode(endIdx);

        // Build adjacency list from connections
        const int nodeCount { static_cast<int>(nodes.size()) };
        std::vector<std::vector<int>> adj(nodeCount);

        for (const auto& [a, b] : connections)
        {
            if (a < nodeCount && b < nodeCount)
            {
                adj[a].push_back(b);
                adj[b].push_back(a);
            }
        }

        // BFS
        std::vector<int> parent(nodeCount, -1);
        std::vector<bool> visited(nodeCount, false);

        std::queue<int> queue;
        queue.push(startIdx);
        visited[startIdx] = true;

        while (!queue.empty())
        {
            const int current { queue.front() };

            queue.pop();

            if (current == endIdx)
                break;

            for (const int neighbor : adj[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    queue.push(neighbor);
                }
            }
        }

        // Reconstruct path
        Path result { };

        if (!visited[endIdx])
            return result;  // No path found

        for (int cur = endIdx; cur != startIdx; cur = parent[cur])
        {
            result.nodes.push_back(nodes[cur]);
        }

        std::reverse(result.nodes.begin(), result.nodes.end());

        return result;
    }

    Path Graph::GetConnectedPath(glm::vec2 startPos, int depth)
    {
        if (m_Nodes.empty() || depth < 1)
            return { };

        // --- Find seed node ---
        const int seedIdx { FindNode(startPos, std::numeric_limits<float>::max()) };

        if (seedIdx == -1)
            return { };

        // --- Build adjacency list (full graph, indices stable) ---
        const int nodeCount { static_cast<int>(m_Nodes.size()) };
        std::vector<std::vector<int>> adj(nodeCount);

        for (const auto& [a, b] : m_Connections)
        {
            adj[a].push_back(b);
            adj[b].push_back(a);
        }

        // --- BFS from seed, up to depth hops ---
        // visited[i] = hop distance from seed, or -1 if not visited
        std::vector<int> hopDist(nodeCount, -1);
        std::queue<int> queue;

        hopDist[seedIdx] = 0;
        queue.push(seedIdx);

        while (!queue.empty())
        {
            const int cur { queue.front() };
            queue.pop();

            if (hopDist[cur] >= depth)
                continue;

            for (const int nb : adj[cur])
            {
                if (hopDist[nb] == -1)
                {
                    hopDist[nb] = hopDist[cur] + 1;
                    queue.push(nb);
                }
            }
        }

        // Collect visited node indices
        std::vector<int> visited;
        visited.reserve(nodeCount);
        for (int i = 0; i < nodeCount; ++i)
        {
            if (hopDist[i] != -1)
                visited.push_back(i);
        }

        if (visited.size() < 2)
            return { };

        // Find the pair of visited nodes furthest apart in world space
        int farA { visited[0] };
        int farB { visited[1] };
        float maxDist2 { 0.0f };

        for (int i = 0; i < static_cast<int>(visited.size()); ++i)
        {
            for (int j = i + 1; j < static_cast<int>(visited.size()); ++j)
            {
                const glm::vec2 delta { m_Nodes[visited[i]] - m_Nodes[visited[j]] };
                const float dist2 { glm::dot(delta, delta) };

                if (dist2 > maxDist2)
                {
                    maxDist2 = dist2;
                    farA = visited[i];
                    farB = visited[j];
                }
            }
        }

        // BFS from farA to farB, restricted to the visited set
        std::vector<bool> inSet(nodeCount, false);
        for (const int idx : visited)
            inSet[idx] = true;

        std::vector<int> parent(nodeCount, -1);
        std::vector<bool> seen(nodeCount, false);
        std::queue<int> pathQueue;

        seen[farA] = true;
        pathQueue.push(farA);

        while (!pathQueue.empty())
        {
            const int cur { pathQueue.front() };
            pathQueue.pop();

            if (cur == farB)
                break;

            for (const int nb : adj[cur])
            {
                if (!seen[nb] && inSet[nb])
                {
                    seen[nb] = true;
                    parent[nb] = cur;
                    pathQueue.push(nb);
                }
            }
        }

        // Reconstruct path farA -> farB
        Path result { };

        if (!seen[farB])
            return result;  // No path

        for (int cur = farB; cur != farA; cur = parent[cur])
            result.nodes.push_back(m_Nodes[cur]);

        result.nodes.push_back(m_Nodes[farA]);
        std::reverse(result.nodes.begin(), result.nodes.end());

        return result;
    }


    int Graph::AddNode(glm::vec2 pos)
    {
        const int existing { FindNode(pos) };

        if (existing != -1)
            return existing;

        m_Nodes.push_back(pos);

        return static_cast<int>(m_Nodes.size()) - 1;
    }

    void Graph::RemoveNode(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_Nodes.size()))
            return;

        m_Nodes.erase(m_Nodes.begin() + index);

        // Remove all connections that reference this node and re-index
        m_Connections.erase(
            std::remove_if(m_Connections.begin(), m_Connections.end(),
                [index] (const std::pair<int, int>& c)
                {
                    return c.first == index || c.second == index;
                }),
            m_Connections.end()
        );

        // Shift indices above 'index' down by one
        for (auto& [a, b] : m_Connections)
        {
            if (a > index)
                a--;

            if (b > index)
                b--;
        }
    }

    void Graph::AddConnection(int a, int b)
    {
        if (a == b)
            return;

        // Normalize order so (a,b) == (b,a)
        if (a > b) std::swap(a, b);

        for (const auto& con : m_Connections)
        {
            const auto [conA, conB] { con };
            auto nodeA { conA }, nodeB { conB };

            if (nodeA > nodeB)
                std::swap(nodeA, nodeB);

            if (nodeA == a && nodeB == b)
                return;
        }

        m_Connections.emplace_back(a, b);
    }

    void Graph::RemoveConnection(int a, int b)
    {
        m_Connections.erase(
            std::remove_if(m_Connections.begin(), m_Connections.end(),
                [a, b] (const std::pair<int, int>& c)
                {
                    return (c.first == a && c.second == b)
                        || (c.first == b && c.second == a);
                }),
            m_Connections.end()
        );
    }

    int Graph::FindNode(glm::vec2 pos, float radius) const
    {
        int bestIdx { -1 };
        float bestDist { radius };

        for (int i = 0; i < static_cast<int>(m_Nodes.size()); ++i)
        {
            const float dist { glm::length(m_Nodes[i] - pos) };

            if (dist <= bestDist)
            {
                bestDist = dist;
                bestIdx = i;
            }
        }

        return bestIdx;
    }

    std::optional<float> Graph::RaySegmentIntersect(
        Ray ray, float maxDist, glm::vec2 a, glm::vec2 b, float& outS)
    {
        const glm::vec2 e { b - a };
        const glm::vec2 f { a - ray.pos };

        const float det { e.x * ray.dir.y - ray.dir.x * e.y };

        // Parallel (or degenerate), no intersection
        if (std::abs(det) < 1e-8f)
            return std::nullopt;

        const float invDet { 1.0f / det };

        const float t { (-f.x * e.y + e.x * f.y) * invDet };
        const float s { (ray.dir.x * f.y - f.x * ray.dir.y) * invDet };

        if (t < 0.0f || t > maxDist)
            return std::nullopt;

        if (s < 0.0f || s > 1.0f)
            return std::nullopt;

        outS = s;

        return t;
    }
}