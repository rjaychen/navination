#include "GameOverWidget.h"
#include "MazeGameMode.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RetryButton)
    {
        RetryButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleRetryClicked);
    }

    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleMainMenuClicked);
    }
}

void UGameOverWidget::SetResults(int32 CurrentLevel, int32 BestLevel)
{
    if (CurrentLevelText)
    {
        CurrentLevelText->SetText(FText::FromString(FString::Printf(TEXT("LEVEL %d"), CurrentLevel)));
    }

    if (BestLevelText)
    {
        BestLevelText->SetText(FText::FromString(FString::Printf(TEXT("BEST %d"), BestLevel)));
    }
}

void UGameOverWidget::HandleRetryClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->RestartLevel();
    }
}

void UGameOverWidget::HandleMainMenuClicked()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->ReturnToMainMenu();
    }
}
