#include "GridCollider.h"
#include "GridComponent.h"
#include "Physics.h"

#include <algorithm>
#include <cmath>

namespace dae
{
    void GridCollider::Register()
    {
        RegisterComponent<GridCollider>("grid_collider");

        RegisterParameter("solid_tile_ids", &GridCollider::m_SolidTileIds);
    }

    void GridCollider::OnInit(EngineCtx& ctx)
    {
        m_Grid = GetComponent<GridComponent>();

        if (m_Grid != nullptr)
            ctx.physics->AddCollider(this);
    }

    void GridCollider::OnDestroy(EngineCtx& ctx)
    {
        if (m_Grid != nullptr)
            ctx.physics->RemoveCollider(this);
    }

    RaycastHit GridCollider::Raycast(Ray ray, float maxDist)
    {
        if (m_Grid == nullptr || m_SolidTileIds.empty())
            return RaycastHit { };

        const float tileSize { m_Grid->GetTileSize() };
        const glm::vec2 gridOrigin { m_Grid->GetTransform().GetWorldPos() };
        const int rows { static_cast<int>(m_Grid->GetTiles().Rows()) };
        const int cols { static_cast<int>(m_Grid->GetTiles().Cols()) };

        // Convert ray origin to grid-local space
        const glm::vec2 localOrigin { ray.pos - gridOrigin };

        // Starting cell
        int cellCol { static_cast<int>(std::floor(localOrigin.x / tileSize)) };
        int cellRow { static_cast<int>(std::floor(localOrigin.y / tileSize)) };

        // Step direction per axis
        const int stepCol { ray.dir.x >= 0.0f ? 1 : -1 };
        const int stepRow { ray.dir.y >= 0.0f ? 1 : -1 };

        // How far along the ray (in t) to cross one full cell on each axis
        const float tDeltaCol { std::abs(tileSize / ray.dir.x) };
        const float tDeltaRow { std::abs(tileSize / ray.dir.y) };

        // t at which the ray first hits a vertical/horizontal cell boundary
        // Distance from origin to first boundary in each axis
        const float firstBoundaryCol {
            ray.dir.x >= 0.0f
                ? (std::ceil(localOrigin.x / tileSize) * tileSize - localOrigin.x)
                : (localOrigin.x - std::floor(localOrigin.x / tileSize) * tileSize)
        };
        const float firstBoundaryRow {
            ray.dir.y >= 0.0f
                ? (std::ceil(localOrigin.y / tileSize) * tileSize - localOrigin.y)
                : (localOrigin.y - std::floor(localOrigin.y / tileSize) * tileSize)
        };

        float tMaxCol { std::abs(ray.dir.x) > 1e-6f ? firstBoundaryCol / std::abs(ray.dir.x) : std::numeric_limits<float>::max() };
        float tMaxRow { std::abs(ray.dir.y) > 1e-6f ? firstBoundaryRow / std::abs(ray.dir.y) : std::numeric_limits<float>::max() };

        float t { 0.0f };

        while (t <= maxDist)
        {
            // Check current cell (skip if outside grid)
            if (cellCol >= 0 && cellCol < cols && cellRow >= 0 && cellRow < rows)
            {
                const int tileId { m_Grid->GetTileId(cellRow, cellCol) };

                if (std::find(m_SolidTileIds.begin(), m_SolidTileIds.end(), tileId)
                    != m_SolidTileIds.end())
                {
                    RaycastHit hit { };
                    hit.hit = true;
                    hit.t = t;
                    hit.pos = ray.pos + ray.dir * t;
                    hit.collider = this;
                    hit.obj = GetOwner();

                    // Surface normal points away from the entered face
                    if (tMaxCol < tMaxRow)
                        hit.normal = glm::vec2 { -static_cast<float>(stepCol), 0.0f };
                    else
                        hit.normal = glm::vec2 { 0.0f, -static_cast<float>(stepRow) };

                    return hit;
                }
            }
            else if (t > 0.0f)
            {
                // Left the grid entirely
                break;
            }

            // Advance to the next cell boundary
            if (tMaxCol < tMaxRow)
            {
                t = tMaxCol;
                tMaxCol += tDeltaCol;
                cellCol += stepCol;
            }
            else
            {
                t = tMaxRow;
                tMaxRow += tDeltaRow;
                cellRow += stepRow;
            }
        }

        return RaycastHit { };
    }
}