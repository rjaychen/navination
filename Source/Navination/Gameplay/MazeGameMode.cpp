#include "Navination.h"
#include "MazeGameMode.h"
#include "MazeActor.h"
#include "MazeGenerator.h"
#include "Kismet/GameplayStatics.h"

AMazeGameMode::AMazeGameMode()
    : Super()
{
    PrimaryActorTick.bCanEverTick = true;
    RemainingTime = TimePerLevel;
}

void AMazeGameMode::BeginPlay()
{
    Super::BeginPlay();

    MazeActor = Cast<AMazeActor>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMazeActor::StaticClass())
    );

    if (!IsValid(MazeActor))
    {
        UE_LOG(LogNavination, Warning, TEXT("AMazeGameMode: No AMazeActor found in level — place one in the scene."));
        return;
    }

    StartLevel();
}

void AMazeGameMode::StartLevel()
{
    bGameOver = false;
    RemainingTime = TimePerLevel;
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

    // Teleport player to (1,1) Start tile — skip on level 1, pawn not spawned yet
    if (CurrentLevel > 1)
    {
        const FVector StartWorldPos = MazeActor->GetActorLocation()
            + FVector(MazeActor->TileSize, MazeActor->TileSize, 100.0f);
        APawn* Pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (IsValid(Pawn))
            Pawn->TeleportTo(StartWorldPos, FRotator::ZeroRotator);
    }
}

void AMazeGameMode::Tick(float DeltaTime)
{
    if (!bGameOver && RemainingTime <= 0)
    {
        bGameOver = true;
        UE_LOG(LogNavination, Log, TEXT("GAME OVER!"));
    } else {
        CheckGoalReached();
    }
}

void AMazeGameMode::OnTimerTick()
{
    RemainingTime = FMath::Max(RemainingTime - 1.f, 0.f);
    UE_LOG(LogNavination, Log, TEXT("Time remaining: %.0f"), RemainingTime);
}

void AMazeGameMode::CheckGoalReached()
{
    if (!IsValid(MazeActor)) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;
    
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
            ++CurrentLevel;
            StartLevel();
        }
    }      
}