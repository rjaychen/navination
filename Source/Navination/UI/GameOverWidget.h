#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetResults(int32 CurrentLevel, int32 BestLevel);

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(meta=(BindWidget))
    UTextBlock* CurrentLevelText = nullptr;

    UPROPERTY(meta=(BindWidget))
    UTextBlock* BestLevelText = nullptr;

    UPROPERTY(meta=(BindWidget))
    UButton* RetryButton = nullptr;

    UPROPERTY(meta=(BindWidget))
    UButton* MainMenuButton = nullptr;

    UFUNCTION()
    void HandleRetryClicked();

    UFUNCTION()
    void HandleMainMenuClicked();
};
