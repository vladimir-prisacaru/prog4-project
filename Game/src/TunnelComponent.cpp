#include "TunnelComponent.h"
#include "GridComponent.h"
#include "SceneManager.h"
#include "Player.h"
#include "Physics.h"

#include <array>



namespace dae
{
    TunnelComponent::~TunnelComponent() { };

    void TunnelComponent::Register()
    {
        RegisterComponent<TunnelComponent>("tunnel_component");

        RegisterParameter("dugout_tile_ids", &TunnelComponent::m_DugoutTileIds);
        RegisterParameter("dugout_levels", &TunnelComponent::m_DugoutLevels);
        RegisterParameter("corner_tile_ids", &TunnelComponent::m_CornerTileIds);
        RegisterParameter("tunnel_tile_ids", &TunnelComponent::m_TunnelTileIds);
        RegisterParameter("edge_tile_ids", &TunnelComponent::m_EdgeTileIds);
        RegisterParameter("empty_tile_id", &TunnelComponent::m_EmptyTileId);
        RegisterParameter("full_tile_id", &TunnelComponent::m_FullTileId);
    }

    void TunnelComponent::OnInit(EngineCtx& ctx)
    {
        m_Grid = GetComponent<GridComponent>();

        m_Players = ctx.sceneManager->GetAllComponentsByType<Player>();

        // Set initial player state
        for (auto& player : m_Players)
        {
            m_LastPlayerTile[player] = INVALID_TILE_COORD;
            m_PlayerDigState[player] = false;
        }

        // Create graph
        m_Graph = std::make_unique<Graph>();
        RebuildGraph();
        ctx.physics->AddCollider(m_Graph.get());
    }

    void TunnelComponent::Update(EngineCtx& ctx)
    {
        if (m_Grid == nullptr)
            return;

        for (auto* player : m_Players)
        {
            const auto oldCoords { m_LastPlayerTile[player] };
            const auto newCoords { m_Grid->GetTileCoords(player->GetTransform().GetWorldPos()) };

            // Skip invalid coords
            if (newCoords == INVALID_TILE_COORD)
            {
                logWarning(
                    "Player (id={}) placed at invalid tile coords in the grid!",
                    player->GetId());

                continue;
            }

            // Set initial state
            if (oldCoords == INVALID_TILE_COORD)
            {
                m_LastPlayerTile[player] = newCoords;

                continue;
            }

            // If coords didn't change
            if (oldCoords == newCoords)
            {
                if (int dir { }, dugoutLevel { };
                    IsTileDugout(m_Grid->GetTileId(oldCoords), dir, dugoutLevel))
                {
                    m_PlayerDigState[player] = true;

                    const glm::vec2 dirVectors[4] {
                        {  0.0f, -1.0f }, // DIR_U
                        {  1.0f,  0.0f }, // DIR_R
                        {  0.0f,  1.0f }, // DIR_D
                        { -1.0f,  0.0f }  // DIR_L
                    };

                    const glm::vec2 digAxis { dirVectors[dir] };
                    const glm::vec2 tileCenter { GetTileCenter(oldCoords) };
                    const float tileSize { m_Grid->GetTileSize() };
                    const glm::vec2 playerPos { player->GetTransform().GetWorldPos() };

                    const float entryFaceProj {
                        glm::dot(tileCenter, digAxis) - tileSize * 0.5f };

                    const float playerProj { glm::dot(playerPos,  digAxis) };

                    const float progress { (playerProj - entryFaceProj) / tileSize };
                    const float clamped { progress < 0.0f ? 0.0f : (progress > 1.0f ? 1.0f : progress) };

                    const int newLevel { static_cast<int>(clamped * static_cast<float>(m_DugoutLevels)) };
                    const int levelClamped { newLevel < m_DugoutLevels ? newLevel : m_DugoutLevels - 1 };

                    // Only update the tile if the level increased
                    if (levelClamped > dugoutLevel)
                        m_Grid->SetTileId(oldCoords, GetDugoutTileId(dir, levelClamped));
                }
                else
                {
                    m_PlayerDigState[player] = false;
                }

                continue;
            }

            const auto [oldRow, oldCol] { oldCoords };
            const auto [newRow, newCol] { newCoords };

            // Calculate difference
            const int dRow { newRow - oldRow };
            const int dCol { newCol - oldCol };

            // Skip diagonal movement
            if (dRow != 0 && dCol != 0)
            {
                logWarning(
                    "Diagonal player movement on the grid is not allowed: dRow={}, dCol={}",
                    dRow, dCol);

                continue;
            }

            const int direction { GetDir(dRow, dCol) };

            // Resolve tiles
            ResolveOldTile(direction, oldCoords);
            ResolveNewTile(direction, newCoords);

            // Update tile coords
            m_LastPlayerTile[player] = newCoords;
        }
    }

    void TunnelComponent::OnDestroy(EngineCtx& ctx)
    {
        ctx.physics->RemoveCollider(m_Graph.get());
    }

    bool TunnelComponent::IsPlayerDigging(Player* player)
    {
        auto it { m_PlayerDigState.find(player) };

        if (it != m_PlayerDigState.end())
            return m_PlayerDigState[player];

        return false;
    }

    Graph* TunnelComponent::GetNavigationGraph()
    {
        return m_Graph.get();
    }



    // -----------------------
    // --- Tile resolution ---
    // -----------------------

    void TunnelComponent::ResolveOldTile(int outDir, TileCoords coords)
    {
        const int oldTileId { m_Grid->GetTileId(coords) };

        // Going from tunnel
        if (int dir { }; IsTileTunnel(oldTileId, dir))
        {
            // If not just passing through
            if (dir != outDir && dir != OppositeDir(outDir))
            {
                m_Grid->SetTileId(coords, GetEdgeTileId(outDir));
                UpdateTileInGraph(coords);
            }

            return;
        }

        // Going from corner
        if (int dir1 { }, dir2 { }; IsTileCorner(oldTileId, dir1, dir2))
        {
            // If not just passing through
            if (dir1 != outDir && dir2 != outDir)
            {
                if (outDir == OppositeDir(dir1))
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir2));
                    UpdateTileInGraph(coords);
                }
                else if (outDir == OppositeDir(dir2))
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir1));
                    UpdateTileInGraph(coords);
                }
            }

            return;
        }

        // Going from dugout
        if (int dir { }, dugoutLevel { }; IsTileDugout(oldTileId, dir, dugoutLevel))
        {
            // If going forward
            if (outDir == dir)
            {
                m_Grid->SetTileId(coords, GetTunnelTileId(dir));
                UpdateTileInGraph(coords);

                return;
            }

            // If going sideways
            if (outDir != OppositeDir(dir))
            {
                m_Grid->SetTileId(coords, GetCornerTileId(outDir, OppositeDir(dir)));
                UpdateTileInGraph(coords);
            }

            return;
        }

        // Going from edge
        if (int dir { }; IsTileEdge(oldTileId, dir))
        {
            // If going through
            if (outDir == OppositeDir(dir))
            {
                m_Grid->SetTileId(coords, GetEmptyTileId());
                UpdateTileInGraph(coords);
            }

            return;
        }
    }

    void TunnelComponent::ResolveNewTile(int inDir, TileCoords coords)
    {
        const int newTileId { m_Grid->GetTileId(coords) };

        // Going into full
        if (IsTileFull(newTileId))
        {
            m_Grid->SetTileId(coords, GetDugoutTileId(inDir, 0));

            // Add the new tile to the graph and connect it to its neighbours
            UpdateTileInGraph(coords);
        }

        // Going into tunnel
        if (int dir { }; IsTileTunnel(newTileId, dir))
        {
            // If not just passing through
            if (dir != inDir && dir != OppositeDir(inDir))
            {
                m_Grid->SetTileId(coords, GetEdgeTileId(OppositeDir(inDir)));
                UpdateTileInGraph(coords);
            }
        }

        // Going into edge
        if (int dir { }; IsTileEdge(newTileId, dir))
        {
            if (inDir == dir)
            {
                m_Grid->SetTileId(coords, GetEmptyTileId());
                UpdateTileInGraph(coords);
            }
        }

        // Going into corner
        if (int dir1 { }, dir2 { }; IsTileCorner(newTileId, dir1, dir2))
        {
            // If not just passing through
            if (OppositeDir(dir1) != inDir && OppositeDir(dir2) != inDir)
            {
                if (inDir == dir1)
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir2));
                    UpdateTileInGraph(coords);
                }
                else if (inDir == dir2)
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir1));
                    UpdateTileInGraph(coords);
                }
            }

            return;
        }

        // Going into dugout
        if (int dir { }, dugoutLevel { }; IsTileDugout(newTileId, dir, dugoutLevel))
        {
            // If going forward
            if (inDir == OppositeDir(dir))
            {
                m_Grid->SetTileId(coords, GetTunnelTileId(OppositeDir(dir)));
                UpdateTileInGraph(coords);

                return;
            }

            // If going sideways
            if (inDir != dir)
            {
                m_Grid->SetTileId(coords, GetCornerTileId(OppositeDir(inDir), OppositeDir(dir)));
                UpdateTileInGraph(coords);
            }

            return;
        }
    }



    // -------------------------
    // --- Direction helpers ---
    // -------------------------

    constexpr int TunnelComponent::GetDir(int dRow, int dCol)
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

    constexpr int TunnelComponent::OppositeDir(int dir)
    {
        switch (dir)
        {
            case DIR_U:
                return DIR_D;
            case DIR_R:
                return DIR_L;
            case DIR_D:
                return DIR_U;
            case DIR_L:
                return DIR_R;
        }

        return INVALID_DIR;
    }



    // --------------------
    // --- Tile Helpers ---
    // --------------------

    bool TunnelComponent::IsTileFull(int id) const
    {
        return id == m_FullTileId;
    }

    bool TunnelComponent::IsTileEmpty(int id) const
    {
        return id == m_EmptyTileId;
    }

    bool TunnelComponent::IsTileDugout(int id, int& dir, int& dugoutLevel) const
    {
        auto it { std::find(m_DugoutTileIds.begin(), m_DugoutTileIds.end(), id) };

        // Skip if not found
        if (it == m_DugoutTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_DugoutTileIds.begin()) };

        dir = idInVector / m_DugoutLevels;
        dugoutLevel = idInVector % m_DugoutLevels;

        return true;
    }

    bool TunnelComponent::IsTileTunnel(int id, int& dir) const
    {
        auto it { std::find(m_TunnelTileIds.begin(), m_TunnelTileIds.end(), id) };

        // Skip if not found
        if (it == m_TunnelTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_TunnelTileIds.begin()) };
        dir = idInVector;

        return true;
    }

    bool TunnelComponent::IsTileEdge(int id, int& dir) const
    {
        auto it { std::find(m_EdgeTileIds.begin(), m_EdgeTileIds.end(), id) };

        // Skip if not found
        if (it == m_EdgeTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_EdgeTileIds.begin()) };
        dir = idInVector;

        return true;
    }

    bool TunnelComponent::IsTileCorner(int id, int& dir1, int& dir2) const
    {
        auto it { std::find(m_CornerTileIds.begin(), m_CornerTileIds.end(), id) };

        // Skip if not found
        if (it == m_CornerTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_CornerTileIds.begin()) };
        dir1 = idInVector;

        switch (dir1)
        {
            case DIR_U:
                dir2 = DIR_R;
                break;
            case DIR_R:
                dir2 = DIR_D;
                break;
            case DIR_D:
                dir2 = DIR_L;
                break;
            case DIR_L:
                dir2 = DIR_U;
                break;
        }

        return true;
    }

    int TunnelComponent::GetFullTileId()
    {
        return m_FullTileId;
    }

    int TunnelComponent::GetEmptyTileId()
    {
        return m_EmptyTileId;
    }

    int TunnelComponent::GetDugoutTileId(int dir, int dugoutLevel)
    {
        return m_DugoutTileIds[(dir * m_DugoutLevels) + dugoutLevel];
    }

    int TunnelComponent::GetTunnelTileId(int dir)
    {
        return m_TunnelTileIds[dir];
    }

    int TunnelComponent::GetEdgeTileId(int dir)
    {
        return m_EdgeTileIds[dir];
    }

    int TunnelComponent::GetCornerTileId(int dir1, int dir2)
    {
        if (dir1 == DIR_U)
        {
            if (dir2 == DIR_R)
                return m_CornerTileIds[DIR_U];
            else if (dir2 == DIR_L)
                return m_CornerTileIds[DIR_L];
        }
        else if (dir1 == DIR_R)
        {
            if (dir2 == DIR_D)
                return m_CornerTileIds[DIR_R];
            else if (dir2 == DIR_U)
                return m_CornerTileIds[DIR_U];
        }
        else if (dir1 == DIR_D)
        {
            if (dir2 == DIR_R)
                return m_CornerTileIds[DIR_R];
            else if (dir2 == DIR_L)
                return m_CornerTileIds[DIR_D];
        }
        else if (dir1 == DIR_L)
        {
            if (dir2 == DIR_U)
                return m_CornerTileIds[DIR_L];
            else if (dir2 == DIR_D)
                return m_CornerTileIds[DIR_D];
        }

        return INVALID_TILE_ID;
    }



    // ---------------------
    // --- Graph Helpers ---
    // ---------------------

    glm::vec2 TunnelComponent::GetTileCenter(TileCoords coords) const
    {
        const glm::vec2 gridOrigin { m_Grid->GetTransform().GetWorldPos() };

        const float tileSize { m_Grid->GetTileSize() };

        return glm::vec2 {
            gridOrigin.x + (static_cast<float>(coords.second) + 0.5f) * tileSize,
            gridOrigin.y + (static_cast<float>(coords.first) + 0.5f) * tileSize
        };
    }

    bool TunnelComponent::IsTilePassable(TileCoords coords) const
    {
        const int id { m_Grid->GetTileId(coords) };

        if (id == INVALID_TILE_ID)
            return false;

        return !IsTileFull(id);
    }

    bool TunnelComponent::IsTileOpenToward(int tileId, int dir) const
    {
        if (IsTileFull(tileId))
            return false;

        if (IsTileEmpty(tileId))
            return true;

        if (int dugDir { }, level { }; IsTileDugout(tileId, dugDir, level))
            return dir == OppositeDir(dugDir);

        if (int tunnelDir { }; IsTileTunnel(tileId, tunnelDir))
            return dir == tunnelDir || dir == OppositeDir(tunnelDir);

        if (int edgeDir { }; IsTileEdge(tileId, edgeDir))
            return dir != OppositeDir(edgeDir);

        if (int dir1 { }, dir2 { }; IsTileCorner(tileId, dir1, dir2))
            return dir == dir1 || dir == dir2;

        return false; // unrecognised tile
    }

    bool TunnelComponent::AreTilesConnected(TileCoords coords, TileCoords nb, int dirAtoB) const
    {
        const int idA { m_Grid->GetTileId(coords) };
        const int idB { m_Grid->GetTileId(nb) };

        if (idA == INVALID_TILE_ID || idB == INVALID_TILE_ID)
            return false;

        return IsTileOpenToward(idA, dirAtoB) && IsTileOpenToward(idB, OppositeDir(dirAtoB));
    }

    void TunnelComponent::UpdateTileInGraph(TileCoords coords)
    {
        if (m_Graph == nullptr)
            return;

        const glm::vec2 center { GetTileCenter(coords) };

        // Ensure a node exists for this tile (needed for newly dug tiles)
        const int nodeIdx { m_Graph->AddNode(center) };

        // The four orthogonal neighbours with the direction from coords to each
        const std::array<std::pair<TileCoords, int>, 4> neighbours { {
            { { coords.first - 1, coords.second     }, DIR_U },
            { { coords.first,     coords.second + 1 }, DIR_R },
            { { coords.first + 1, coords.second     }, DIR_D },
            { { coords.first,     coords.second - 1 }, DIR_L }
        } };

        for (const auto& [nb, dir] : neighbours)
        {
            const glm::vec2 nbCenter { GetTileCenter(nb) };
            const int nbIdx { m_Graph->FindNode(nbCenter) };

            if (nbIdx == -1)
                continue; // neighbour has no node, nothing to connect to or disconnect from

            // Remove any existing connection first, then re-add only if valid
            m_Graph->RemoveConnection(nodeIdx, nbIdx);

            if (AreTilesConnected(coords, nb, dir))
                m_Graph->AddConnection(nodeIdx, nbIdx);
        }
    }

    void TunnelComponent::RebuildGraph()
    {
        if (m_Grid == nullptr || m_Graph == nullptr)
            return;

        const int rows { static_cast<int>(m_Grid->GetTiles().Rows()) };
        const int cols { static_cast<int>(m_Grid->GetTiles().Cols()) };

        // First pass: add all passable tile centers as nodes
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const TileCoords coords { r, c };

                if (IsTilePassable(coords))
                    m_Graph->AddNode(GetTileCenter(coords));
            }
        }

        // Second pass: connect adjacent passable tiles that share an open face
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const TileCoords coords { r, c };

                if (!IsTilePassable(coords))
                    continue;

                const int nodeIdx { m_Graph->FindNode(GetTileCenter(coords)) };

                if (nodeIdx == -1)
                    continue;

                const std::array<std::pair<TileCoords, int>, 2> neighbours { {
                    { { r + 1, c     }, DIR_D },
                    { { r,     c + 1 }, DIR_R }
                } };

                for (const auto& [nb, dir] : neighbours)
                {
                    if (!IsTilePassable(nb))
                        continue;

                    if (!AreTilesConnected(coords, nb, dir))
                        continue;

                    const int nbIdx { m_Graph->FindNode(GetTileCenter(nb)) };

                    if (nbIdx == -1)
                        continue;

                    m_Graph->AddConnection(nodeIdx, nbIdx);
                }
            }
        }
    }
}