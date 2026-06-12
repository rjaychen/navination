#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Maze/MazeTypes.h"
#include "MazeGenerator.generated.h"

UCLASS()
class AMazeGenerator : public AActor
{
    GENERATED_BODY()
    public:
        AMazeGenerator();

        UPROPERTY(EditAnywhere)
        int32 MazeWidth = 3;

        UPROPERTY(EditAnywhere)
        int32 MazeHeight = 3;

        UPROPERTY(EditAnywhere)
        int32 Seed = 42;

        FMazeGrid GenerateMaze(int32 Width, int32 Height, int32 Seed);

    protected:
        virtual void BeginPlay() override;

    private:
        int32 FindRoot(TArray<int32>& Parent, int32 Node);

        void Union(TArray<int32>& Parent, int32 A, int32 B);
        
        TPair<int32, int32> BFSLongestPath(const FMazeGrid& Grid, int32 StartX, int32 StartY);
};