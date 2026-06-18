#include "PauseMenuWidget.h"
#include "MazeGameMode.h"
#include "Components/Button.h"

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleResumeClicked);
    }

    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleRestartClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleQuitClicked);
    }
}

void UPauseMenuWidget::HandleResumeClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->ResumeGame();
    }
}

void UPauseMenuWidget::HandleRestartClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->RestartLevel();
    }
}

void UPauseMenuWidget::HandleQuitClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->ReturnToMainMenu();
    }
}
