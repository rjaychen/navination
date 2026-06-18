#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MazeHUD.generated.h"

class AMazeActor;
class AMazeCharacter;
class AMazeGameState;
class UImage;
class UProgressBar;
class UTextBlock;
class UWidgetAnimation;
class UMazeMapWidget;

UCLASS()
class UMazeHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(meta=(BindWidget))
    UTextBlock* TimerText = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* LevelText = nullptr;

    UPROPERTY(meta=(BindWidget))
    UProgressBar* AbilityBar1 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UProgressBar* AbilityBar2 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UProgressBar* AbilityBar3 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UProgressBar* AbilityBar4 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* AbilityName1 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* AbilityName2 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* AbilityName3 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* AbilityName4 = nullptr;

    UPROPERTY(meta=(BindWidget))
    UImage* CompassArrow = nullptr;

    UPROPERTY(meta=(BindWidget))
    UMazeMapWidget* MiniMap = nullptr;

    UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
    UWidgetAnimation* LowTimePulse = nullptr;

    TWeakObjectPtr<AMazeGameState> MazeGameState;
    TWeakObjectPtr<AMazeActor> MazeActor;
    TWeakObjectPtr<AMazeCharacter> MazeCharacter;
    FIntPoint GoalTile = FIntPoint(-1, -1);

    UFUNCTION()
    void HandleTimeChanged(float NewTime);

    UFUNCTION()
    void HandleLevelChanged(int32 NewLevel);

    UFUNCTION()
    void HandleChargesChanged(const TArray<int32>& NewCharges);

    void UpdateTimerText(float NewTime);
    void UpdateLevelText(int32 NewLevel);
    void UpdateAbilityBars(const TArray<int32>& Charges);
    void UpdateAbilityNames();
    void UpdateAbilityLockState(int32 CurrentLevel);
    void UpdateGoalTile();
    void UpdateCompass();
};
