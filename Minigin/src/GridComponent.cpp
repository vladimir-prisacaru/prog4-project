#include "GridComponent.h"

namespace dae
{
    void GridComponent::Register()
    {
        RegisterComponent<GridComponent>("grid_component");

        RegisterParameter("tiles", &GridComponent::m_Tiles);
        RegisterParameter("tile_size", &GridComponent::m_TileSize);
    }

    TileCoords GridComponent::GetTileCoords(glm::vec2 pos) const
    {
        const glm::vec2 gridPos { GetTransform().GetWorldPos() };

        const int rows { static_cast<int>(m_Tiles.Rows()) };
        const int cols { static_cast<int>(m_Tiles.Cols()) };

        const glm::vec2 localPos { pos - gridPos };

        const int col { static_cast<int>(std::floor(localPos.x / m_TileSize)) };
        const int row { static_cast<int>(std::floor(localPos.y / m_TileSize)) };

        if (col < 0 || col >= cols || row < 0 || row >= rows)
        {
            return INVALID_TILE_COORD;
        }

        return { row, col };
    }

    int GridComponent::GetTileId(int row, int col) const
    {
        if (row < 0 || row >= m_Tiles.Rows() || col < 0 || col >= m_Tiles.Cols())
            return INVALID_TILE_ID;

        return m_Tiles(row, col);
    }

    int GridComponent::GetTileId(TileCoords coords) const
    {
        return GetTileId(coords.first, coords.second);
    }

    int GridComponent::GetTileId(glm::vec2 pos) const
    {
        const auto coords { GetTileCoords(pos) };

        if (coords == INVALID_TILE_COORD)
            return INVALID_TILE_ID;

        return GetTileId(coords);
    }

    void GridComponent::SetTileId(int row, int col, int value)
    {
        if (row < 0 || row >= m_Tiles.Rows() || col < 0 || col >= m_Tiles.Cols())
            return;

        m_Tiles(row, col) = value;
    }

    void GridComponent::SetTileId(TileCoords coords, int value)
    {
        SetTileId(coords.first, coords.second, value);
    }
}