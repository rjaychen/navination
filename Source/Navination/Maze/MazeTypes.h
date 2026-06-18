#pragma once
#include "CoreMinimal.h"
#include "MazeTypes.generated.h"

UENUM(BlueprintType)
enum class EMazeTile : uint8 { Floor, Wall, Start, Goal };

UENUM(BlueprintType)
enum class EMazeWallShape : uint8 { Straight, Corner, Junction, DeadEnd, Pillar };

USTRUCT()
struct FMazeGrid
{
    GENERATED_BODY()

    FMazeGrid()
        : Width(0)
        , Height(0)
    {
    }

    UPROPERTY(EditAnywhere)
    int32 Width = 0;

    UPROPERTY(EditAnywhere)
    int32 Height = 0;

    TArray<EMazeTile> Tiles;

    EMazeTile GetTile(int32 X, int32 Y) const;
    void SetTile(int32 X, int32 Y, EMazeTile Tile);
};