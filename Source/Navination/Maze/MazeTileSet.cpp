#include "MazeTileSet.h"

const FMazeTileVisual* UMazeTileSet::GetTileVisual(EMazeTile Tile) const
{
    switch (Tile)
    {
        case EMazeTile::Floor: return &Floor;
        case EMazeTile::Start: return &Start;
        case EMazeTile::Goal:  return &Goal;
        default:               return nullptr;
    }
}

const FMazeTileVisual* UMazeTileSet::GetWallVisual(EMazeWallShape Shape) const
{
    for (const FMazeWallVisual& Variant : WallVariants)
    {
        if (Variant.Shape == Shape)
        {
            return &Variant.Visual;
        }
    }

    return &DefaultWall;
}
