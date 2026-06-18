#include "MazeHUD.h"
#include "MazeActor.h"
#include "MazeCharacter.h"
#include "MazeGameState.h"
#include "MazeMapWidget.h"
#include "Abilities/AbilityComponent.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UMazeHUD::NativeConstruct()
{
    Super::NativeConstruct();

    MazeGameState = GetWorld() ? GetWorld()->GetGameState<AMazeGameState>() : nullptr;
    MazeActor = Cast<AMazeActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMazeActor::StaticClass()));
    MazeCharacter = Cast<AMazeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

    if (MazeGameState.IsValid())
    {
        MazeGameState->OnTimeChanged.AddDynamic(this, &UMazeHUD::HandleTimeChanged);
        MazeGameState->OnLevelChanged.AddDynamic(this, &UMazeHUD::HandleLevelChanged);
        MazeGameState->OnAbilityChargesChanged.AddDynamic(this, &UMazeHUD::HandleChargesChanged);

        HandleTimeChanged(MazeGameState->RemainingTime);
        HandleLevelChanged(MazeGameState->CurrentLevel);
        HandleChargesChanged(MazeGameState->AbilityCharges);
    }

    if (MiniMap)
    {
        MiniMap->SetMazeReferences(MazeGameState.Get(), MazeActor.Get(), MazeCharacter.Get());
    }

    UpdateAbilityNames();
    if (MazeGameState.IsValid())
    {
        UpdateAbilityLockState(MazeGameState->CurrentLevel);
    }
    UpdateGoalTile();
}

void UMazeHUD::NativeDestruct()
{
    if (MazeGameState.IsValid())
    {
        MazeGameState->OnTimeChanged.RemoveDynamic(this, &UMazeHUD::HandleTimeChanged);
        MazeGameState->OnLevelChanged.RemoveDynamic(this, &UMazeHUD::HandleLevelChanged);
        MazeGameState->OnAbilityChargesChanged.RemoveDynamic(this, &UMazeHUD::HandleChargesChanged);
    }

    Super::NativeDestruct();
}

void UMazeHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateCompass();
}

void UMazeHUD::HandleTimeChanged(float NewTime)
{
    UpdateTimerText(NewTime);
    if (LowTimePulse)
    {
        if (NewTime < 15.0f)
        {
            if (!IsAnimationPlaying(LowTimePulse))
            {
                PlayAnimation(LowTimePulse, 0.0f, 0);
            }
        }
        else
        {
            StopAnimation(LowTimePulse);
        }
    }
}

void UMazeHUD::HandleLevelChanged(int32 NewLevel)
{
    UpdateLevelText(NewLevel);
    UpdateGoalTile();
    UpdateAbilityLockState(NewLevel);
}

void UMazeHUD::HandleChargesChanged(const TArray<int32>& NewCharges)
{
    UpdateAbilityBars(NewCharges);
}

void UMazeHUD::UpdateTimerText(float NewTime)
{
    if (!TimerText)
    {
        return;
    }

    const int32 RoundedTime = FMath::Max(0, FMath::RoundToInt(NewTime));
    const int32 Minutes = RoundedTime / 60;
    const int32 Seconds = RoundedTime % 60;
    TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
}

void UMazeHUD::UpdateLevelText(int32 NewLevel)
{
    if (!LevelText)
    {
        return;
    }

    LevelText->SetText(FText::FromString(FString::Printf(TEXT("LEVEL %d"), NewLevel)));
}

void UMazeHUD::UpdateAbilityBars(const TArray<int32>& Charges)
{
    if (!MazeCharacter.IsValid())
    {
        MazeCharacter = Cast<AMazeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    }

    UProgressBar* Bars[4] = { AbilityBar1, AbilityBar2, AbilityBar3, AbilityBar4 };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        if (!Bars[Index])
        {
            continue;
        }

        const int32 CurrentLevel = MazeGameState.IsValid() ? MazeGameState->CurrentLevel : 1;
        const bool bUnlocked = MazeCharacter.IsValid()
            ? MazeCharacter->IsAbilityUnlocked(Index, CurrentLevel)
            : true;
        const int32 CurrentCharges = Charges.IsValidIndex(Index) ? Charges[Index] : 0;
        const UAbilityComponent* Ability = MazeCharacter.IsValid() ? MazeCharacter->GetAbility(Index) : nullptr;
        const int32 MaxCharges = Ability ? FMath::Max(Ability->MaxCharges, 1) : 1;
        const float Percent = bUnlocked
            ? static_cast<float>(CurrentCharges) / static_cast<float>(MaxCharges)
            : 0.0f;
        Bars[Index]->SetPercent(Percent);
    }
}

void UMazeHUD::UpdateAbilityNames()
{
    const FText Names[4] = {
        FText::FromString(TEXT("STOP TIME")),
        FText::FromString(TEXT("DASH")),
        FText::FromString(TEXT("BREAK WALL")),
        FText::FromString(TEXT("TELEPORT"))
    };

    UTextBlock* NameBlocks[4] = { AbilityName1, AbilityName2, AbilityName3, AbilityName4 };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        if (NameBlocks[Index])
        {
            NameBlocks[Index]->SetText(Names[Index]);
        }
    }
}

void UMazeHUD::UpdateAbilityLockState(int32 CurrentLevel)
{
    if (!MazeCharacter.IsValid())
    {
        MazeCharacter = Cast<AMazeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    }

    UProgressBar* Bars[4] = { AbilityBar1, AbilityBar2, AbilityBar3, AbilityBar4 };
    UTextBlock* NameBlocks[4] = { AbilityName1, AbilityName2, AbilityName3, AbilityName4 };

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const bool bUnlocked = MazeCharacter.IsValid()
            ? MazeCharacter->IsAbilityUnlocked(Index, CurrentLevel)
            : true;
        const float Opacity = bUnlocked ? 1.0f : 0.35f;
        if (Bars[Index])
        {
            Bars[Index]->SetRenderOpacity(Opacity);
        }
        if (NameBlocks[Index])
        {
            NameBlocks[Index]->SetRenderOpacity(Opacity);
        }
    }
}

void UMazeHUD::UpdateGoalTile()
{
    GoalTile = FIntPoint(-1, -1);
    if (!MazeActor.IsValid())
    {
        return;
    }

    const FMazeGrid& Grid = MazeActor->CurrentGrid;
    for (int32 Y = 0; Y < Grid.Height; ++Y)
    {
        for (int32 X = 0; X < Grid.Width; ++X)
        {
            if (Grid.GetTile(X, Y) == EMazeTile::Goal)
            {
                GoalTile = FIntPoint(X, Y);
                return;
            }
        }
    }
}

void UMazeHUD::UpdateCompass()
{
    if (!CompassArrow || !MazeActor.IsValid())
    {
        return;
    }

    if (GoalTile.X < 0 || GoalTile.Y < 0)
    {
        return;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn)
    {
        return;
    }

    const FVector MazeOrigin = MazeActor->GetActorLocation();
    const float TileSize = MazeActor->TileSize;
    const FVector GoalWorld = MazeOrigin + FVector((GoalTile.X + 0.5f) * TileSize, (GoalTile.Y + 0.5f) * TileSize, 0.0f);
    const FVector ToGoal = (GoalWorld - Pawn->GetActorLocation());
    const FVector2D ToGoal2D(ToGoal.X, ToGoal.Y);
    if (ToGoal2D.IsNearlyZero())
    {
        return;
    }

    const FVector Forward = Pawn->GetActorForwardVector();
    const FVector2D Forward2D(Forward.X, Forward.Y);
    const float Dot = FVector2D::DotProduct(Forward2D.GetSafeNormal(), ToGoal2D.GetSafeNormal());
    const float Cross = Forward2D.X * ToGoal2D.Y - Forward2D.Y * ToGoal2D.X;
    const float Angle = FMath::RadiansToDegrees(FMath::Atan2(Cross, Dot));

    CompassArrow->SetRenderTransformAngle(Angle);
}
