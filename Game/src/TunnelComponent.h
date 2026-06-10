#pragma once

#include "GameObject.h"
#include "GridComponent.h"
#include "Graph.h"



namespace dae
{
    class Player;

    class TunnelComponent : public Component, public Registrar<TunnelComponent>
    {
        public:

        static void Register();

        explicit TunnelComponent(GameObject* owner) : Component(owner) { };
        virtual ~TunnelComponent();

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        bool IsPlayerDigging(Player* player);
        Graph* GetNavigationGraph();



        private:

        // --- Tile resolution ---
        void ResolveOldTile(int outDir, TileCoords coords);
        void ResolveNewTile(int inDir, TileCoords coords);

        // --- Direction Helpers ---
        static constexpr int INVALID_DIR { -1 };
        static constexpr int DIR_U { 0 };
        static constexpr int DIR_R { 1 };
        static constexpr int DIR_D { 2 };
        static constexpr int DIR_L { 3 };

        static constexpr int GetDir(int dRow, int dCol);
        static constexpr int OppositeDir(int dir);

        // --- Tile Helpers ---
        bool IsTileFull(int id) const;
        bool IsTileEmpty(int id) const;
        bool IsTileDugout(int id, int& dir, int& dugoutLevel) const;
        bool IsTileTunnel(int id, int& dir) const;
        bool IsTileEdge(int id, int& dir) const;
        bool IsTileCorner(int id, int& dir1, int& dir2) const;

        int GetFullTileId();
        int GetEmptyTileId();
        int GetDugoutTileId(int dir, int dugoutLevel);
        int GetTunnelTileId(int dir);
        int GetEdgeTileId(int dir);
        int GetCornerTileId(int dir1, int dir2);

        // --- Graph Helpers ---

        // Returns the world-space center of a tile
        glm::vec2 GetTileCenter(TileCoords coords) const;
        // Returns true if the tile at coords is not solid (passable)
        bool IsTilePassable(TileCoords coords) const;
        // Ensures a node exists for this tile center and connects it to its
        // passable orthogonal neighbours. Should be called whenever a tile
        // transitions from solid to non-solid.
        void AddTileToGraph(TileCoords coords);
        // Rebuilds the entire graph from the current grid state.
        // Called once at the end of OnInit for pre-dug tiles in the scene.
        void RebuildGraph();

        // --- Params ---

        // Ids of dugout tiles (tiles at the end of a tunnel)
        std::vector<int> m_DugoutTileIds { };
        // Number of discreet levels of how dug out a tile can be
        // (e.g. if set to 3 then there is a 1/3 dug out tile, 2/3 and fully dug out)
        // Then, to get the correct tile: m_DugoutTiles[(direction * m_DugoutLevels) + dugoutLevel]
        int m_DugoutLevels { };
        // Ids of corner tiles
        // Ordered like: m_CornerTileIds[0]: up and right directions connect to tunnels,
        // 1: right-down, 2: down-left, 3: left-up;
        std::vector<int> m_CornerTileIds { };
        // Ids of tiles in the middle of the tunnel
        std::vector<int> m_TunnelTileIds { };
        // Ids of edge tiles
        std::vector<int> m_EdgeTileIds { };
        // Id of the completely untouched tile
        int m_FullTileId { -1 };
        // Id of the tile completely dug out from all sides
        int m_EmptyTileId { };


        // --- Other ---

        // Last tile visited by the player, keyed by player ptr
        std::unordered_map<Player*, std::pair<int, int>> m_LastPlayerTile { };
        // Stores if the player was digging or not
        std::unordered_map<Player*, bool> m_PlayerDigState { };
        // Component that owns the tile grid
        GridComponent* m_Grid { };
        // All active players
        std::vector<Player*> m_Players { };
        // Navigation graph of all dug out tile centers
        std::unique_ptr<Graph> m_Graph { };
    };
}