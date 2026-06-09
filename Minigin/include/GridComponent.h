#pragma once

#include <glm/glm.hpp>

#include "Array2d.h"
#include "GameObject.h"

namespace dae
{
    // Encoded as { row, column }
    using TileCoords = std::pair<int, int>;

    static constexpr int INVALID_TILE_ID { -1 };
    static constexpr TileCoords INVALID_TILE_COORD { -1, -1 };

    class GridComponent : public Component, public Registrar<GridComponent>
    {
        public:

        static void Register();

        GridComponent(GameObject* owner) : Component(owner) { };
        virtual ~GridComponent() = default;

        Array2d<int>& GetTiles() { return m_Tiles; }
        const Array2d<int>& GetTiles() const { return m_Tiles; }

        float GetTileSize() const { return m_TileSize; }
        void SetTileSize(float tileSize) { m_TileSize = tileSize; }

        TileCoords GetTileCoords(glm::vec2 pos) const;

        int GetTileId(int row, int col) const;
        int GetTileId(TileCoords coords) const;
        int GetTileId(glm::vec2 pos) const;

        void SetTileId(int row, int col, int value);
        void SetTileId(TileCoords coords, int value);

        private:

        Array2d<int> m_Tiles { };

        float m_TileSize { 32.0f };
    };
}