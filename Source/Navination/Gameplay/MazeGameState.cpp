#include "MazeGameState.h"

AMazeGameState::AMazeGameState()
{
    AbilityCharges.Init(0, 4);
}

void AMazeGameState::SetCurrentLevel(int32 NewLevel)
{
    if (CurrentLevel == NewLevel)
    {
        return;
    }

    CurrentLevel = NewLevel;
    OnLevelChanged.Broadcast(CurrentLevel);
}

void AMazeGameState::SetRemainingTime(float NewTime)
{
    if (FMath::IsNearlyEqual(RemainingTime, NewTime))
    {
        return;
    }

    RemainingTime = NewTime;
    OnTimeChanged.Broadcast(RemainingTime);
}

void AMazeGameState::SetAbilityCharges(const TArray<int32>& NewCharges)
{
    AbilityCharges = NewCharges;
    if (AbilityCharges.Num() < 4)
    {
        AbilityCharges.AddZeroed(4 - AbilityCharges.Num());
    }
    OnAbilityChargesChanged.Broadcast(AbilityCharges);
}

void AMazeGameState::InitExploredTiles(int32 Width, int32 Height)
{
    GridWidth = Width;
    GridHeight = Height;
    ExploredTiles.Init(false, GridWidth * GridHeight);
    OnExplorationUpdated.Broadcast();
}

void AMazeGameState::ResetExploration()
{
    for (bool& Tile : ExploredTiles)
    {
        Tile = false;
    }
    OnExplorationUpdated.Broadcast();
}

void AMazeGameState::MarkExplored(int32 TileX, int32 TileY, int32 RevealRadius)
{
    if (GridWidth <= 0 || GridHeight <= 0)
    {
        return;
    }

    bool bAnyChanged = false;
    for (int32 OffsetY = -RevealRadius; OffsetY <= RevealRadius; ++OffsetY)
    {
        for (int32 OffsetX = -RevealRadius; OffsetX <= RevealRadius; ++OffsetX)
        {
            const int32 CheckX = TileX + OffsetX;
            const int32 CheckY = TileY + OffsetY;
            if (!IsInBounds(CheckX, CheckY))
            {
                continue;
            }

            const int32 Index = GetIndex(CheckX, CheckY);
            if (!ExploredTiles.IsValidIndex(Index))
            {
                continue;
            }

            if (!ExploredTiles[Index])
            {
                ExploredTiles[Index] = true;
                bAnyChanged = true;
            }
        }
    }

    if (bAnyChanged)
    {
        OnExplorationUpdated.Broadcast();
    }
}

int32 AMazeGameState::GetGridWidth() const
{
    return GridWidth;
}

int32 AMazeGameState::GetGridHeight() const
{
    return GridHeight;
}

int32 AMazeGameState::GetIndex(int32 TileX, int32 TileY) const
{
    return (TileY * GridWidth) + TileX;
}

bool AMazeGameState::IsInBounds(int32 TileX, int32 TileY) const
{
    return TileX >= 0 && TileX < GridWidth && TileY >= 0 && TileY < GridHeight;
}
