#pragma once

#include "CoreMinimal.h"
#include "NavinationGameMode.h"
#include "MazeGameMode.generated.h"

class AMazeActor;

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

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<AMazeActor> MazeActor;

        virtual void BeginPlay() override;

        virtual void Tick(float DeltaTime) override;

        UFUNCTION(BlueprintCallable)
        void StartLevel();

        private:
            FTimerHandle CountdownHandle;

            bool bGameOver = false;

            UFUNCTION()
            void OnTimerTick();

            UFUNCTION()
            void CheckGoalReached();
};