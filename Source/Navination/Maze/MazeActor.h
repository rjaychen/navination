#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Maze/MazeTypes.h"
#include "MazeActor.generated.h"

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

        UPROPERTY(EditAnywhere)
        TObjectPtr<UStaticMesh> FloorMesh;

        UPROPERTY(EditAnywhere)
        TObjectPtr<UStaticMesh> WallMesh;

        void BuildFromGrid(const FMazeGrid& Grid);
};