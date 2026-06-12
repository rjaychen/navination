#pragma once
#include "CoreMinimal.h"
#include "MazeTypes.generated.h"

UENUM(BlueprintType)
enum class EMazeTile : uint8 { Floor, Wall, Start, Goal };

USTRUCT()
struct FMazeGrid
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere)
    int32 Width;

    UPROPERTY(EditAnywhere)
    int32 Height;

    TArray<EMazeTile> Tiles;

    EMazeTile GetTile(int32 X, int32 Y) const;
    void SetTile(int32 X, int32 Y, EMazeTile Tile);
};