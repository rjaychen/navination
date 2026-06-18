#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MazeTypes.h"
#include "MazeTileSet.generated.h"

USTRUCT(BlueprintType)
struct FMazeTileVisual
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<UStaticMesh> Mesh = nullptr;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UMaterialInterface> Material = nullptr;
};

USTRUCT(BlueprintType)
struct FMazeWallVisual
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EMazeWallShape Shape = EMazeWallShape::Straight;

    UPROPERTY(EditAnywhere)
    FMazeTileVisual Visual;
};

UCLASS(BlueprintType)
class UMazeTileSet : public UDataAsset
{
    GENERATED_BODY()
    public:
        UPROPERTY(EditAnywhere, Category="Tiles")
        FMazeTileVisual Floor;

        UPROPERTY(EditAnywhere, Category="Tiles")
        FMazeTileVisual Start;

        UPROPERTY(EditAnywhere, Category="Tiles")
        FMazeTileVisual Goal;

        UPROPERTY(EditAnywhere, Category="Walls")
        FMazeTileVisual DefaultWall;

        UPROPERTY(EditAnywhere, Category="Walls")
        TArray<FMazeWallVisual> WallVariants;

        UPROPERTY(EditAnywhere, Category="Goal Tint")
        FLinearColor GoalBaseColor = FLinearColor::White;

        UPROPERTY(EditAnywhere, Category="Goal Tint")
        FLinearColor GoalGoldColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

        UPROPERTY(EditAnywhere, Category="Goal Tint")
        FName GoalTintParameter = TEXT("TintColor");

        const FMazeTileVisual* GetTileVisual(EMazeTile Tile) const;
        const FMazeTileVisual* GetWallVisual(EMazeWallShape Shape) const;
};
