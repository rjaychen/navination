#include "MainMenuWidget.h"
#include "MazeGameMode.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PlayButton)
    {
        PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlayClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
    }
}

void UMainMenuWidget::HandlePlayClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->StartGame();
    }
}

void UMainMenuWidget::HandleQuitClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}
