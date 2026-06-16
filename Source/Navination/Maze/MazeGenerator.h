#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeTypes.h"
#include "MazeActor.h"
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

        UPROPERTY(EditAnywhere)
        TSubclassOf<AMazeActor> MazeActorClass;

        static FMazeGrid GenerateMaze(int32 Width, int32 Height, int32 Seed);

    protected:
        virtual void BeginPlay() override;

    private:
        static int32 FindRoot(TArray<int32>& Parent, int32 Node);

        static void Union(TArray<int32>& Parent, int32 A, int32 B);
        
        static TPair<int32, int32> BFSLongestPath(const FMazeGrid& Grid, int32 StartX, int32 StartY);
};