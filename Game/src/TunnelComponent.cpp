#include "TunnelComponent.h"
#include "GridComponent.h"
#include "SceneManager.h"
#include "Player.h"


namespace dae
{
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

        for (auto& player : m_Players)
        {
            m_LastPlayerTile[player] = INVALID_TILE_COORD;
            m_PlayerDigState[player] = false;
        }
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

    void TunnelComponent::OnDestroy(EngineCtx&)
    {
        // nothing to destroy yet
    }

    bool TunnelComponent::IsPlayerDigging(Player* player)
    {
        auto it { m_PlayerDigState.find(player) };

        if (it != m_PlayerDigState.end())
            return m_PlayerDigState[player];

        return false;
    }

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
                }
                else if (outDir == OppositeDir(dir2))
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir1));
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

                return;
            }

            // If going sideways
            if (outDir != OppositeDir(dir))
            {
                m_Grid->SetTileId(coords, GetCornerTileId(outDir, OppositeDir(dir)));
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
            // TODO: CHANGE FROM m_DugoutLevels to 0, when digging is implemented
            m_Grid->SetTileId(coords, GetDugoutTileId(inDir, m_DugoutLevels-1));
        }

        // Going into tunnel
        if (int dir { }; IsTileTunnel(newTileId, dir))
        {
            // If not just passing through
            if (dir != inDir && dir != OppositeDir(inDir))
            {
                m_Grid->SetTileId(coords, GetEdgeTileId(OppositeDir(inDir)));
            }
        }

        // Going into edge
        if (int dir { }; IsTileEdge(newTileId, dir))
        {
            if (inDir == dir)
            {
                m_Grid->SetTileId(coords, GetEmptyTileId());
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
                }
                else if (inDir == dir2)
                {
                    m_Grid->SetTileId(coords, GetEdgeTileId(dir1));
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

                return;
            }

            // If going sideways
            if (inDir != dir)
            {
                m_Grid->SetTileId(coords, GetCornerTileId(OppositeDir(inDir), OppositeDir(dir)));
            }

            return;
        }
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

    bool TunnelComponent::IsTileFull(int id)
    {
        return id == m_FullTileId;
    }

    bool TunnelComponent::IsTileEmpty(int id)
    {
        return id == m_EmptyTileId;
    }

    bool TunnelComponent::IsTileDugout(int id, int& dir, int& dugoutLevel)
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

    bool TunnelComponent::IsTileTunnel(int id, int& dir)
    {
        auto it { std::find(m_TunnelTileIds.begin(), m_TunnelTileIds.end(), id) };

        // Skip if not found
        if (it == m_TunnelTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_TunnelTileIds.begin()) };
        dir = idInVector;

        return true;
    }

    bool TunnelComponent::IsTileEdge(int id, int& dir)
    {
        auto it { std::find(m_EdgeTileIds.begin(), m_EdgeTileIds.end(), id) };

        // Skip if not found
        if (it == m_EdgeTileIds.end())
            return false;

        const int idInVector { static_cast<int>(it - m_EdgeTileIds.begin()) };
        dir = idInVector;

        return true;
    }

    bool TunnelComponent::IsTileCorner(int id, int& dir1, int& dir2)
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


}