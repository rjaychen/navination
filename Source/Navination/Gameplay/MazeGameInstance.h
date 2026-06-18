#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MazeGameInstance.generated.h"

UCLASS()
class UMazeGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Maze")
    int32 BestLevel = 1;

    UFUNCTION(BlueprintCallable, Category="Maze")
    void UpdateBestLevel(int32 Level);
};
