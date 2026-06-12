#include "MazeTypes.h"

EMazeTile FMazeGrid::GetTile(int32 X, int32 Y) const
{
    if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        return EMazeTile::Wall;
    return Tiles[Y * Width + X];
}

void FMazeGrid::SetTile(int32 X, int32 Y, EMazeTile Tile)
{
    if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        return;
    Tiles[Y * Width + X] = Tile;
}