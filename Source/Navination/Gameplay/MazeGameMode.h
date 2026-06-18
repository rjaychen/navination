#pragma once

#include "CoreMinimal.h"
#include "NavinationGameMode.h"
#include "MazeGameMode.generated.h"

class AMazeActor;
class AMazeGameState;
class UMazeHUD;
class UMainMenuWidget;
class UPauseMenuWidget;
class UGameOverWidget;
class ULevelTransitionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelComplete);

UCLASS()
class AMazeGameMode : public ANavinationGameMode
{
    GENERATED_BODY()
    public:
        AMazeGameMode();

        UPROPERTY(EditAnywhere)
        float TimePerLevel = 60.0f;

        UPROPERTY(EditAnywhere)
        int32 CurrentLevel = 1;

        UPROPERTY(VisibleAnywhere)
        float RemainingTime;

        UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
        FOnGameOver OnGameOver;

        UPROPERTY(BlueprintAssignable, Category="Maze|Delegates")
        FOnLevelComplete OnLevelComplete;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<AMazeActor> MazeActor;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        TObjectPtr<AMazeGameState> MazeGameState;

        virtual void BeginPlay() override;

        virtual void Tick(float DeltaTime) override;

        UFUNCTION(BlueprintCallable)
        void StartGame();

        UFUNCTION(BlueprintCallable)
        void ResumeGame();

        UFUNCTION(BlueprintCallable)
        void RestartLevel();

        UFUNCTION(BlueprintCallable)
        void ReturnToMainMenu();

        UFUNCTION(BlueprintCallable)
        void TogglePauseMenu();

        UFUNCTION(BlueprintCallable)
        void StartLevel();

        UFUNCTION(BlueprintCallable, Category="Maze")
        bool IsGameOver() const { return bGameOver; }

        UFUNCTION(BlueprintCallable, Category="Maze|Debug")
        void EvaluateGoalReached();

        private:
            FTimerHandle CountdownHandle;
            FTimerHandle TeleportHandle;
            FTimerHandle LevelTransitionHandle;
            FTimerHandle CorruptionHandle;

            bool bGameOver = false;
            bool bTransitioning = false;
            FVector PendingStartLocation = FVector::ZeroVector;
            FIntPoint LastPlayerTile = FIntPoint(-1, -1);
            bool bHasPlayerTile = false;
            bool bCorruptionActive = false;
            FIntPoint GoalTile = FIntPoint(-1, -1);

            UPROPERTY(EditAnywhere, Category="Extras")
            float CorruptionIntervalSeconds = 3.0f;

            UPROPERTY(EditAnywhere, Category="Extras")
            int32 CorruptionMinTiles = 1;

            UPROPERTY(EditAnywhere, Category="Extras")
            int32 CorruptionMaxTiles = 2;

            UPROPERTY(EditAnywhere, Category="UI")
            bool bStartInMainMenu = true;

            UPROPERTY(EditAnywhere, Category="UI")
            FName MainMenuLevelName = NAME_None;

            UPROPERTY(EditDefaultsOnly, Category="UI")
            TSubclassOf<UMazeHUD> MazeHUDClass;

            UPROPERTY(EditDefaultsOnly, Category="UI")
            TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

            UPROPERTY(EditDefaultsOnly, Category="UI")
            TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

            UPROPERTY(EditDefaultsOnly, Category="UI")
            TSubclassOf<UGameOverWidget> GameOverWidgetClass;

            UPROPERTY(EditDefaultsOnly, Category="UI")
            TSubclassOf<ULevelTransitionWidget> LevelTransitionWidgetClass;

            UPROPERTY(Transient)
            TObjectPtr<UMazeHUD> MazeHUDWidget;

            UPROPERTY(Transient)
            TObjectPtr<UMainMenuWidget> MainMenuWidget;

            UPROPERTY(Transient)
            TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

            UPROPERTY(Transient)
            TObjectPtr<UGameOverWidget> GameOverWidget;

            UPROPERTY(Transient)
            TObjectPtr<ULevelTransitionWidget> LevelTransitionWidget;

            UFUNCTION()
            void OnTimerTick();

            UFUNCTION()
            void CheckGoalReached();

            UFUNCTION()
            void HandleGameOver();

            UFUNCTION()
            void HandleLevelComplete();

            void TeleportPlayerToStart();
            void RefreshAbilityCharges();
            void UpdateExploration();
            void UpdateGoalTile();
            void UpdateHeartbeat();
            void UpdateCorruptionState();
            void HandleCorruptionTick();
            bool TryGetPlayerTile(FIntPoint& OutTile) const;
            void ShowMainMenu();
            void ShowPauseMenu();
            void HidePauseMenu();
            void ShowGameOver();
            void ShowLevelTransition(int32 CompletedLevel, int32 BonusSeconds);
};