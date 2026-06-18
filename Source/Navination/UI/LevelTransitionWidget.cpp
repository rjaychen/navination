#include "LevelTransitionWidget.h"
#include "Components/TextBlock.h"

void ULevelTransitionWidget::PlayTransition(int32 CompletedLevel, int32 BonusSeconds)
{
    if (LevelCompleteText)
    {
        LevelCompleteText->SetText(FText::FromString(FString::Printf(TEXT("LEVEL %d COMPLETE"), CompletedLevel)));
    }

    if (BonusTimeText)
    {
        if (BonusSeconds > 0)
        {
            BonusTimeText->SetText(FText::FromString(FString::Printf(TEXT("+%ds bonus"), BonusSeconds)));
            BonusTimeText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            BonusTimeText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (PopAnimation)
    {
        PlayAnimation(PopAnimation);
    }
}
