#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MazeTypes.h"
#include "MazeActor.generated.h"

class UMazeTileSet;
struct FMazeTileVisual;
class UMaterialInstanceDynamic;

UCLASS()
class AMazeActor : public AActor
{
    GENERATED_BODY()
    public:
        AMazeActor();

        UPROPERTY(EditAnywhere)
        float TileSize = 200.f;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> FloorISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallStraightISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallCornerISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallJunctionISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallDeadEndISMC;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<UInstancedStaticMeshComponent> WallPillarISMC;

        UPROPERTY(EditAnywhere)
        TObjectPtr<UStaticMesh> FloorMesh;

        UPROPERTY(EditAnywhere)
        TObjectPtr<UStaticMesh> WallMesh;

        UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> StartISMC;
        UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> GoalISMC;
        UPROPERTY(EditAnywhere)    TObjectPtr<UStaticMesh> StartMesh;
        UPROPERTY(EditAnywhere)    TObjectPtr<UStaticMesh> GoalMesh;

        UPROPERTY(EditAnywhere)
        TObjectPtr<UMazeTileSet> TileSet;

        UPROPERTY(EditAnywhere)
        int32 GoalTintMaxLevel = 10;

        FMazeGrid CurrentGrid;

        void BuildFromGrid(const FMazeGrid& Grid);

        UFUNCTION(BlueprintCallable, Category="Maze")
        bool DestroyWallAt(int32 TileX, int32 TileY);

        UFUNCTION(BlueprintCallable, Category="Maze")
        bool RegrowWallAt(int32 TileX, int32 TileY);

        void SetGoalTintForLevel(int32 Level);

    private:
        TArray<uint8> WallNeighborMask;
        float GoalTintLevelProgress = 0.0f;
        UPROPERTY(Transient)
        TObjectPtr<UMaterialInstanceDynamic> GoalTintMID;

        void ClearInstances();
        void BuildWallInstances();
        uint8 BuildNeighborMask(int32 TileX, int32 TileY) const;
        EMazeWallShape ClassifyWallShape(uint8 NeighborMask) const;
        FRotator GetWallRotation(uint8 NeighborMask, EMazeWallShape Shape) const;
        UInstancedStaticMeshComponent* GetWallISMC(EMazeWallShape Shape) const;
        void ConfigureTileISMC(UInstancedStaticMeshComponent* ISMC, const FMazeTileVisual* Visual, UStaticMesh* FallbackMesh) const;
        void ConfigureWallISMC(UInstancedStaticMeshComponent* ISMC, EMazeWallShape Shape) const;
        void ApplyGoalTint();
        void SpawnRubbleAt(const FVector& WorldLocation);
};