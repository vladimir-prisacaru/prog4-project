#include "GridRenderer.h"
#include "Renderer.h"
#include "ResourceManager.h"



void dae::GridRenderer::Register()
{
    RegisterComponent<GridRenderer>("grid_renderer");

    RegisterParameter("spritesheet_path", &GridRenderer::m_SpritesheetPath);
    RegisterParameter("point_filter", &GridRenderer::m_PointFilter);
    RegisterParameter("tile_src_size", &GridRenderer::m_TileSrcSize);
    RegisterParameter("draw_order", &GridRenderer::m_DrawOrder);
}

void dae::GridRenderer::OnInit(EngineCtx& ctx)
{
    ctx.renderer->AddRenderable(this);

    m_Grid = GetComponent<GridComponent>();

    m_Spritesheet = ctx.resourceManager->LoadTexture(m_SpritesheetPath, m_PointFilter);
}

void dae::GridRenderer::OnDestroy(EngineCtx& ctx)
{
    ctx.renderer->RemoveRenderable(this);
}

int dae::GridRenderer::GetDrawOrder() const
{
    return m_DrawOrder;
}

void dae::GridRenderer::Render(const Renderer* renderer) const
{
    if (m_Grid == nullptr || m_Spritesheet == nullptr)
        return;

    const auto& tiles { m_Grid->GetTiles() };
    glm::vec2 gridPos { m_Grid->GetTransform().GetWorldPos() };
    float tileSize { m_Grid->GetTileSize() };

    // How many tiles fit across the spritesheet width
    const int sheetCols { static_cast<int>(m_Spritesheet->GetSize().x) / m_TileSrcSize };

    const size_t rows { tiles.Rows() };
    const size_t cols { tiles.Cols() };

    for (size_t row { 0 }; row < rows; row++)
    {
        for (size_t col { 0 }; col < cols; col++)
        {
            const int tileId { tiles(row, col) };

            if (tileId == INVALID_TILE_ID)
                continue;

            // Source rect: sample the correct tile from the spritesheet
            const int tileCol { tileId % sheetCols };
            const int tileRow { tileId / sheetCols };

            const SDL_FRect srcRect
            {
                static_cast<float>(tileCol * m_TileSrcSize),
                static_cast<float>(tileRow * m_TileSrcSize),
                static_cast<float>(m_TileSrcSize),
                static_cast<float>(m_TileSrcSize)
            };

            // Destination rect: world position and size
            const float worldX { gridPos.x + static_cast<float>(col) * tileSize };
            const float worldY { gridPos.y + static_cast<float>(row) * tileSize };

            const SDL_FRect dstRect
            {
                worldX,
                worldY,
                tileSize,
                tileSize
            };

            // Draw tile
            renderer->RenderTexture(*m_Spritesheet, srcRect, dstRect);
        }
    }
}