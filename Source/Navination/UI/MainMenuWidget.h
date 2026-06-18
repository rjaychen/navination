#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS()
class UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(meta=(BindWidget))
    UButton* PlayButton = nullptr;

    UPROPERTY(meta=(BindWidget))
    UButton* QuitButton = nullptr;

    UFUNCTION()
    void HandlePlayClicked();

    UFUNCTION()
    void HandleQuitClicked();
};
