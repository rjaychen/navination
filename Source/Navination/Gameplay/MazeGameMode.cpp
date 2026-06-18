#include "MazeGameMode.h"
#include "Navination.h"
#include "MazeActor.h"
#include "MazeCharacter.h"
#include "MazeGameInstance.h"
#include "MazeGameState.h"
#include "MazeGenerator.h"
#include "UI/GameOverWidget.h"
#include "UI/LevelTransitionWidget.h"
#include "UI/MainMenuWidget.h"
#include "UI/MazeHUD.h"
#include "UI/PauseMenuWidget.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AMazeGameMode::AMazeGameMode()
    : Super()
{
    PrimaryActorTick.bCanEverTick = true;
    RemainingTime = TimePerLevel;
    GameStateClass = AMazeGameState::StaticClass();
    DefaultPawnClass = AMazeCharacter::StaticClass();
}

void AMazeGameMode::BeginPlay()
{
    Super::BeginPlay();

    MazeActor = Cast<AMazeActor>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMazeActor::StaticClass())
    );
    MazeGameState = GetGameState<AMazeGameState>();

    OnGameOver.AddDynamic(this, &AMazeGameMode::HandleGameOver);
    OnLevelComplete.AddDynamic(this, &AMazeGameMode::HandleLevelComplete);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (MazeHUDClass)
        {
            MazeHUDWidget = CreateWidget<UMazeHUD>(PC, MazeHUDClass);
            if (MazeHUDWidget)
            {
                MazeHUDWidget->AddToViewport();
            }
        }

        if (MainMenuWidgetClass)
        {
            MainMenuWidget = CreateWidget<UMainMenuWidget>(PC, MainMenuWidgetClass);
            if (MainMenuWidget)
            {
                MainMenuWidget->AddToViewport();
                MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        if (PauseMenuWidgetClass)
        {
            PauseMenuWidget = CreateWidget<UPauseMenuWidget>(PC, PauseMenuWidgetClass);
            if (PauseMenuWidget)
            {
                PauseMenuWidget->AddToViewport();
                PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        if (GameOverWidgetClass)
        {
            GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
            if (GameOverWidget)
            {
                GameOverWidget->AddToViewport();
                GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        if (LevelTransitionWidgetClass)
        {
            LevelTransitionWidget = CreateWidget<ULevelTransitionWidget>(PC, LevelTransitionWidgetClass);
            if (LevelTransitionWidget)
            {
                LevelTransitionWidget->AddToViewport();
                LevelTransitionWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    if (!IsValid(MazeActor))
    {
        UE_LOG(LogNavination, Warning, TEXT("AMazeGameMode: No AMazeActor found in level — place one in the scene."));
        return;
    }

    if (bStartInMainMenu && MainMenuWidget)
    {
        ShowMainMenu();
    }
    else
    {
        StartLevel();
    }
}

void AMazeGameMode::StartLevel()
{
    bGameOver = false;
    bTransitioning = false;
    RemainingTime = TimePerLevel;
    bHasPlayerTile = false;
    LastPlayerTile = FIntPoint(-1, -1);
    bCorruptionActive = false;
    GetWorldTimerManager().ClearTimer(CorruptionHandle);
    if (IsValid(MazeGameState))
    {
        MazeGameState->SetRemainingTime(RemainingTime);
        MazeGameState->SetCurrentLevel(CurrentLevel);
    }

    GetWorldTimerManager().ClearTimer(CountdownHandle);
    GetWorldTimerManager().SetTimer(
        CountdownHandle,
        this,
        &AMazeGameMode::OnTimerTick,
        1.0f,
        true
    );

    if (!IsValid(MazeActor)) return;

    const int32 Size = 3 + CurrentLevel;
    const FMazeGrid Grid = AMazeGenerator::GenerateMaze(Size, Size, CurrentLevel);
    MazeActor->BuildFromGrid(Grid);
    MazeActor->SetGoalTintForLevel(CurrentLevel);
    UpdateGoalTile();

    if (IsValid(MazeGameState))
    {
        MazeGameState->ResetExploration();
        MazeGameState->InitExploredTiles(Grid.Width, Grid.Height);
    }

    PendingStartLocation = MazeActor->GetActorLocation()
        + FVector(MazeActor->TileSize, MazeActor->TileSize, 100.0f);
    TeleportPlayerToStart();

    RefreshAbilityCharges();

    if (MazeHUDWidget)
    {
        MazeHUDWidget->SetVisibility(ESlateVisibility::Visible);
    }

    if (LevelTransitionWidget)
    {
        LevelTransitionWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (GameOverWidget)
    {
        GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AMazeGameMode::Tick(float DeltaTime)
{
    if (UGameplayStatics::IsGamePaused(this))
    {
        return;
    }

    if (bTransitioning)
    {
        return;
    }

    UpdateExploration();
    UpdateHeartbeat();
    UpdateCorruptionState();

    if (!bGameOver && RemainingTime <= 0)
    {
        bGameOver = true;
        if (UWorld* World = GetWorld())
        {
            World->GetWorldSettings()->SetTimeDilation(1.0f);
        }
        OnGameOver.Broadcast();
        UE_LOG(LogNavination, Log, TEXT("GAME OVER!"));
    }
    else
    {
        CheckGoalReached();
    }
}

void AMazeGameMode::OnTimerTick()
{
    RemainingTime = FMath::Max(RemainingTime - 1.f, 0.f);
    if (IsValid(MazeGameState))
    {
        MazeGameState->SetRemainingTime(RemainingTime);
    }
    UE_LOG(LogNavination, Log, TEXT("Time remaining: %.0f"), RemainingTime);
}

void AMazeGameMode::CheckGoalReached()
{
    if (!IsValid(MazeActor)) return;
    if (MazeActor->CurrentGrid.Width <= 0 || MazeActor->CurrentGrid.Height <= 0)
    {
        return;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;
    if (!IsValid(Pawn))
    {
        Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    }
    
    if (!IsValid(Pawn)) return;

    FVector MazeOrigin = MazeActor->GetActorLocation();
    FVector LocalPos   = Pawn->GetActorLocation() - MazeOrigin;
    int32 TileX        = FMath::FloorToInt(LocalPos.X / MazeActor->TileSize);
    int32 TileY        = FMath::FloorToInt(LocalPos.Y / MazeActor->TileSize);
    
    if (TileX > -1 && TileX < MazeActor->CurrentGrid.Width && TileY > -1 && TileY < MazeActor->CurrentGrid.Height)
    {
        if (MazeActor->CurrentGrid.GetTile(TileX, TileY) == EMazeTile::Goal)
        {
            UE_LOG(LogNavination, Log, TEXT("Reached GOAL! Completed level %d."), CurrentLevel);
            bTransitioning = true;
            GetWorldTimerManager().ClearTimer(CountdownHandle);
            OnLevelComplete.Broadcast();
            ++CurrentLevel;
            if (IsValid(MazeGameState))
            {
                MazeGameState->SetCurrentLevel(CurrentLevel);
            }
        }
    }      
}

void AMazeGameMode::EvaluateGoalReached()
{
    CheckGoalReached();
}

void AMazeGameMode::TeleportPlayerToStart()
{
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (IsValid(Pawn))
    {
        Pawn->TeleportTo(PendingStartLocation, FRotator::ZeroRotator);
        return;
    }

    GetWorldTimerManager().SetTimerForNextTick(this, &AMazeGameMode::TeleportPlayerToStart);
}

void AMazeGameMode::RefreshAbilityCharges()
{
    AMazeCharacter* MazeCharacter = Cast<AMazeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!IsValid(MazeCharacter))
    {
        return;
    }

    MazeCharacter->ApplyAbilityUnlocks(CurrentLevel);
    if (IsValid(MazeGameState))
    {
        TArray<int32> Charges;
        MazeCharacter->GetAbilityCharges(Charges);
        MazeGameState->SetAbilityCharges(Charges);
    }
}

void AMazeGameMode::StartGame()
{
    CurrentLevel = 1;
    bGameOver = false;
    bTransitioning = false;

    if (MainMenuWidget)
    {
        MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    UGameplayStatics::SetGamePaused(this, false);
    StartLevel();
}

void AMazeGameMode::ResumeGame()
{
    UGameplayStatics::SetGamePaused(this, false);
    HidePauseMenu();
}

void AMazeGameMode::RestartLevel()
{
    bGameOver = false;
    bTransitioning = false;
    GetWorldTimerManager().ClearTimer(LevelTransitionHandle);
    UGameplayStatics::SetGamePaused(this, false);
    HidePauseMenu();
    StartLevel();
}

void AMazeGameMode::ReturnToMainMenu()
{
    bGameOver = false;
    bTransitioning = false;
    GetWorldTimerManager().ClearTimer(LevelTransitionHandle);
    GetWorldTimerManager().ClearTimer(CountdownHandle);
    CurrentLevel = 1;

    UGameplayStatics::SetGamePaused(this, false);

    if (MainMenuLevelName != NAME_None)
    {
        UGameplayStatics::OpenLevel(this, MainMenuLevelName);
        return;
    }

    ShowMainMenu();
}

void AMazeGameMode::TogglePauseMenu()
{
    if (UGameplayStatics::IsGamePaused(this))
    {
        ResumeGame();
    }
    else
    {
        ShowPauseMenu();
    }
}

void AMazeGameMode::HandleGameOver()
{
    bGameOver = true;
    GetWorldTimerManager().ClearTimer(CountdownHandle);
    ShowGameOver();
}

void AMazeGameMode::HandleLevelComplete()
{
    const int32 CompletedLevel = CurrentLevel;
    const int32 BonusSeconds = FMath::Max(0, FMath::RoundToInt(RemainingTime * 0.25f));
    ShowLevelTransition(CompletedLevel, BonusSeconds);

    const float Delay = LevelTransitionWidget ? LevelTransitionWidget->TransitionDuration : 2.0f;
    GetWorldTimerManager().SetTimer(LevelTransitionHandle, this, &AMazeGameMode::StartLevel, Delay, false);
}

void AMazeGameMode::UpdateExploration()
{
    if (!IsValid(MazeActor) || !IsValid(MazeGameState))
    {
        return;
    }
    if (MazeActor->CurrentGrid.Width <= 0 || MazeActor->CurrentGrid.Height <= 0)
    {
        return;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!IsValid(Pawn))
    {
        return;
    }

    const FVector MazeOrigin = MazeActor->GetActorLocation();
    const FVector LocalPos = Pawn->GetActorLocation() - MazeOrigin;
    const int32 TileX = FMath::FloorToInt(LocalPos.X / MazeActor->TileSize);
    const int32 TileY = FMath::FloorToInt(LocalPos.Y / MazeActor->TileSize);

    if (TileX < 0 || TileX >= MazeActor->CurrentGrid.Width || TileY < 0 || TileY >= MazeActor->CurrentGrid.Height)
    {
        return;
    }

    if (!bHasPlayerTile || TileX != LastPlayerTile.X || TileY != LastPlayerTile.Y)
    {
        LastPlayerTile = FIntPoint(TileX, TileY);
        bHasPlayerTile = true;
        MazeGameState->MarkExplored(TileX, TileY, 2);
    }
}

void AMazeGameMode::UpdateGoalTile()
{
    GoalTile = FIntPoint(-1, -1);
    if (!IsValid(MazeActor))
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

void AMazeGameMode::UpdateHeartbeat()
{
    if (!IsValid(MazeActor))
    {
        return;
    }

    AMazeCharacter* MazeCharacter = Cast<AMazeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!IsValid(MazeCharacter))
    {
        return;
    }

    UAudioComponent* HeartbeatAudio = MazeCharacter->GetHeartbeatAudio();
    if (!HeartbeatAudio)
    {
        return;
    }

    if (GoalTile.X < 0 || GoalTile.Y < 0)
    {
        return;
    }

    FIntPoint PlayerTile;
    if (!TryGetPlayerTile(PlayerTile))
    {
        return;
    }

    const float Distance = FVector2D::Distance(
        FVector2D(PlayerTile.X, PlayerTile.Y),
        FVector2D(GoalTile.X, GoalTile.Y)
    );
    const float MaxDistance = FVector2D::Distance(
        FVector2D(0.0f, 0.0f),
        FVector2D(MazeActor->CurrentGrid.Width - 1, MazeActor->CurrentGrid.Height - 1)
    );

    const float Normalized = MaxDistance > 0.0f
        ? FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f)
        : 1.0f;

    HeartbeatAudio->SetFloatParameter(MazeCharacter->GetHeartbeatParameterName(), Normalized);
}

void AMazeGameMode::UpdateCorruptionState()
{
    const bool bShouldCorrupt = RemainingTime > 0.0f && RemainingTime < 15.0f;
    if (bShouldCorrupt && !bCorruptionActive)
    {
        bCorruptionActive = true;
        GetWorldTimerManager().ClearTimer(CorruptionHandle);
        GetWorldTimerManager().SetTimer(
            CorruptionHandle,
            this,
            &AMazeGameMode::HandleCorruptionTick,
            CorruptionIntervalSeconds,
            true
        );
    }
    else if (!bShouldCorrupt && bCorruptionActive)
    {
        bCorruptionActive = false;
        GetWorldTimerManager().ClearTimer(CorruptionHandle);
    }
}

void AMazeGameMode::HandleCorruptionTick()
{
    if (!IsValid(MazeActor))
    {
        return;
    }

    FIntPoint PlayerTile;
    const bool bHasPlayer = TryGetPlayerTile(PlayerTile);

    const int32 TilesToCorrupt = FMath::Clamp(
        FMath::RandRange(CorruptionMinTiles, CorruptionMaxTiles),
        1,
        4
    );

    int32 Attempts = 0;
    int32 Corrupted = 0;
    const int32 MaxAttempts = TilesToCorrupt * 8;

    while (Corrupted < TilesToCorrupt && Attempts < MaxAttempts)
    {
        ++Attempts;
        const int32 TileX = FMath::RandRange(0, MazeActor->CurrentGrid.Width - 1);
        const int32 TileY = FMath::RandRange(0, MazeActor->CurrentGrid.Height - 1);
        if (bHasPlayer && PlayerTile.X == TileX && PlayerTile.Y == TileY)
        {
            continue;
        }
        if (MazeActor->CurrentGrid.GetTile(TileX, TileY) != EMazeTile::Floor)
        {
            continue;
        }

        if (MazeActor->RegrowWallAt(TileX, TileY))
        {
            ++Corrupted;
        }
    }
}

bool AMazeGameMode::TryGetPlayerTile(FIntPoint& OutTile) const
{
    if (!IsValid(MazeActor))
    {
        return false;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!IsValid(Pawn))
    {
        return false;
    }

    const FVector MazeOrigin = MazeActor->GetActorLocation();
    const FVector LocalPos = Pawn->GetActorLocation() - MazeOrigin;
    const int32 TileX = FMath::FloorToInt(LocalPos.X / MazeActor->TileSize);
    const int32 TileY = FMath::FloorToInt(LocalPos.Y / MazeActor->TileSize);

    if (TileX < 0 || TileX >= MazeActor->CurrentGrid.Width || TileY < 0 || TileY >= MazeActor->CurrentGrid.Height)
    {
        return false;
    }

    OutTile = FIntPoint(TileX, TileY);
    return true;
}

void AMazeGameMode::ShowMainMenu()
{
    if (MainMenuWidget)
    {
        MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
    }

    if (MazeHUDWidget)
    {
        MazeHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (GameOverWidget)
    {
        GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (LevelTransitionWidget)
    {
        LevelTransitionWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    UGameplayStatics::SetGamePaused(this, true);
}

void AMazeGameMode::ShowPauseMenu()
{
    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameAndUI InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    UGameplayStatics::SetGamePaused(this, true);
}

void AMazeGameMode::HidePauseMenu()
{
    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void AMazeGameMode::ShowGameOver()
{
    if (MazeHUDWidget)
    {
        MazeHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (GameOverWidget)
    {
        int32 BestLevel = CurrentLevel;
        if (UMazeGameInstance* GameInstance = GetGameInstance<UMazeGameInstance>())
        {
            GameInstance->UpdateBestLevel(CurrentLevel);
            BestLevel = GameInstance->BestLevel;
        }

        GameOverWidget->SetResults(CurrentLevel, BestLevel);
        GameOverWidget->SetVisibility(ESlateVisibility::Visible);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    UGameplayStatics::SetGamePaused(this, true);
}

void AMazeGameMode::ShowLevelTransition(int32 CompletedLevel, int32 BonusSeconds)
{
    if (!LevelTransitionWidget)
    {
        return;
    }

    LevelTransitionWidget->SetVisibility(ESlateVisibility::Visible);
    LevelTransitionWidget->PlayTransition(CompletedLevel, BonusSeconds);
}