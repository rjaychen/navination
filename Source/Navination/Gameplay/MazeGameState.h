#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MazeGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExplorationUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityChargesChanged, const TArray<int32>&, NewCharges);

UCLASS()
class AMazeGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    AMazeGameState();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Maze")
    int32 CurrentLevel = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Maze")
    float RemainingTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Maze")
    TArray<int32> AbilityCharges;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Maze")
    TArray<bool> ExploredTiles;

    UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
    FOnTimeChanged OnTimeChanged;

    UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
    FOnLevelChanged OnLevelChanged;

    UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
    FOnExplorationUpdated OnExplorationUpdated;

    UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
    FOnAbilityChargesChanged OnAbilityChargesChanged;

    UFUNCTION(BlueprintCallable, Category="Maze")
    void SetCurrentLevel(int32 NewLevel);

    UFUNCTION(BlueprintCallable, Category="Maze")
    void SetRemainingTime(float NewTime);

    UFUNCTION(BlueprintCallable, Category="Maze")
    void SetAbilityCharges(const TArray<int32>& NewCharges);

    UFUNCTION(BlueprintCallable, Category="Maze")
    void InitExploredTiles(int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable, Category="Maze")
    void ResetExploration();

    UFUNCTION(BlueprintCallable, Category="Maze")
    void MarkExplored(int32 TileX, int32 TileY, int32 RevealRadius);

    UFUNCTION(BlueprintCallable, Category="Maze")
    int32 GetGridWidth() const;

    UFUNCTION(BlueprintCallable, Category="Maze")
    int32 GetGridHeight() const;

private:
    int32 GridWidth = 0;
    int32 GridHeight = 0;

    int32 GetIndex(int32 TileX, int32 TileY) const;
    bool IsInBounds(int32 TileX, int32 TileY) const;
};
