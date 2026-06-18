#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelTransitionWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class ULevelTransitionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void PlayTransition(int32 CompletedLevel, int32 BonusSeconds);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Transition")
    float TransitionDuration = 2.0f;

protected:
    UPROPERTY(meta=(BindWidget))
    UTextBlock* LevelCompleteText = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* BonusTimeText = nullptr;

    UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
    UWidgetAnimation* PopAnimation = nullptr;
};
