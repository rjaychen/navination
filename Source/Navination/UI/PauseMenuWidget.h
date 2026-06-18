#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(meta=(BindWidget))
    UButton* ResumeButton = nullptr;

    UPROPERTY(meta=(BindWidget))
    UButton* RestartButton = nullptr;

    UPROPERTY(meta=(BindWidget))
    UButton* QuitButton = nullptr;

    UFUNCTION()
    void HandleResumeClicked();

    UFUNCTION()
    void HandleRestartClicked();

    UFUNCTION()
    void HandleQuitClicked();
};
