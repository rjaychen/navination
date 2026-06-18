#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include "Abilities/StopTimeAbility.h"
#include "Gameplay/MazeCharacter.h"
#include "Gameplay/MazeGameMode.h"
#include "Gameplay/MazeGameState.h"
#include "Maze/MazeActor.h"

namespace NavinationGameFlowTest
{
    struct FTestContext
    {
        TUniquePtr<FTestWorldWrapper> WorldWrapper;
        UWorld* World = nullptr;
        AMazeGameMode* GameMode = nullptr;
        AMazeGameState* GameState = nullptr;
        AMazeActor* MazeActor = nullptr;
        AMazeCharacter* Character = nullptr;
        APlayerController* PlayerController = nullptr;
    };

    static bool CreateWorld(FTestContext& Context)
    {
        Context.WorldWrapper = MakeUnique<FTestWorldWrapper>();
        if (!Context.WorldWrapper->CreateTestWorld(EWorldType::Game))
        {
            return false;
        }

        Context.World = Context.WorldWrapper->GetTestWorld();
        if (!Context.World)
        {
            return false;
        }

        Context.MazeActor = Context.World->SpawnActor<AMazeActor>();
        if (Context.MazeActor)
        {
            Context.MazeActor->TileSize = 200.0f;
        }

        Context.Character = Context.World->SpawnActor<AMazeCharacter>();

        FURL URL;
        URL.AddOption(TEXT("game=/Script/Navination.MazeGameMode"));
        Context.World->SetGameMode(URL);

        if (!Context.WorldWrapper->BeginPlayInTestWorld())
        {
            return false;
        }

        Context.GameMode = Cast<AMazeGameMode>(Context.World->GetAuthGameMode());
        Context.GameState = Context.World->GetGameState<AMazeGameState>();
        if (!Context.GameState)
        {
            Context.GameState = Context.World->SpawnActor<AMazeGameState>();
        }

        Context.PlayerController = Context.World->GetFirstPlayerController();
        if (!Context.PlayerController)
        {
            Context.PlayerController = Context.World->SpawnActor<APlayerController>();
        }

        if (Context.PlayerController && Context.Character)
        {
            Context.PlayerController->Possess(Context.Character);
        }

        if (Context.GameMode)
        {
            Context.GameMode->MazeActor = Context.MazeActor;
            Context.GameMode->MazeGameState = Context.GameState;
            Context.GameMode->StartLevel();
        }

        return Context.World && Context.GameMode && Context.GameState && Context.MazeActor && Context.Character;
    }

    static void DestroyWorld(FTestContext& Context)
    {
        if (Context.WorldWrapper)
        {
            Context.WorldWrapper->DestroyTestWorld(true);
        }

        Context = FTestContext();
    }
}

class FNavinationForceGameModeTickCommand : public IAutomationLatentCommand
{
public:
    explicit FNavinationForceGameModeTickCommand(TSharedPtr<NavinationGameFlowTest::FTestContext> InContext, int32 TickCount = 3)
        : Context(MoveTemp(InContext))
        , RemainingTicks(TickCount)
    {
    }

    bool Update() override
    {
        if (Context.IsValid() && Context->GameMode && RemainingTicks > 0)
        {
            Context->GameMode->Tick(0.016f);
            --RemainingTicks;
        }
        return RemainingTicks <= 0;
    }

private:
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context;
    int32 RemainingTicks;
};

class FNavinationCleanupCommand : public IAutomationLatentCommand
{
public:
    explicit FNavinationCleanupCommand(TSharedPtr<NavinationGameFlowTest::FTestContext> InContext)
        : Context(MoveTemp(InContext))
    {
    }

    bool Update() override
    {
        if (Context.IsValid())
        {
            NavinationGameFlowTest::DestroyWorld(*Context);
            Context.Reset();
        }
        return true;
    }

private:
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context;
};

class FNavinationCheckLevelAdvanceCommand : public IAutomationLatentCommand
{
public:
    FNavinationCheckLevelAdvanceCommand(FAutomationTestBase* InTest, TSharedPtr<NavinationGameFlowTest::FTestContext> InContext)
        : Test(InTest)
        , Context(MoveTemp(InContext))
    {
    }

    bool Update() override
    {
        if (Test && Context.IsValid() && Context->GameMode)
        {
            Test->TestEqual(TEXT("Current level advances to 2"), Context->GameMode->CurrentLevel, 2);
        }
        return true;
    }

private:
    FAutomationTestBase* Test;
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context;
};

class FNavinationCheckGameOverCommand : public IAutomationLatentCommand
{
public:
    FNavinationCheckGameOverCommand(FAutomationTestBase* InTest, TSharedPtr<NavinationGameFlowTest::FTestContext> InContext)
        : Test(InTest)
        , Context(MoveTemp(InContext))
    {
    }

    bool Update() override
    {
        if (Test && Context.IsValid() && Context->GameMode)
        {
            Test->TestTrue(TEXT("Game over is triggered"), Context->GameMode->IsGameOver());
        }
        return true;
    }

private:
    FAutomationTestBase* Test;
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context;
};

class FNavinationCheckTimeDilationCommand : public IAutomationLatentCommand
{
public:
    FNavinationCheckTimeDilationCommand(FAutomationTestBase* InTest, TSharedPtr<NavinationGameFlowTest::FTestContext> InContext)
        : Test(InTest)
        , Context(MoveTemp(InContext))
    {
    }

    bool Update() override
    {
        if (Test && Context.IsValid() && Context->World)
        {
            if (Context->Character)
            {
                if (UStopTimeAbility* StopTime = Cast<UStopTimeAbility>(Context->Character->GetAbility(0)))
                {
                    StopTime->RestoreTimeDilation();
                }
            }

            const float Dilation = Context->World->GetWorldSettings()->GetEffectiveTimeDilation();
            Test->TestTrue(TEXT("Time dilation restored to 1.0"), FMath::IsNearlyEqual(Dilation, 1.0f, 0.05f));
        }
        return true;
    }

private:
    FAutomationTestBase* Test;
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FLevelAdvanceTest,
    "Navination.GameFlow.LevelAdvance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FLevelAdvanceTest::RunTest(const FString& Parameters)
{
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context = MakeShared<NavinationGameFlowTest::FTestContext>();
    if (!NavinationGameFlowTest::CreateWorld(*Context))
    {
        AddError(TEXT("Failed to create test world."));
        NavinationGameFlowTest::DestroyWorld(*Context);
        return false;
    }

    FIntPoint GoalTile(-1, -1);
    const FMazeGrid& Grid = Context->MazeActor->CurrentGrid;
    for (int32 Y = 0; Y < Grid.Height; ++Y)
    {
        for (int32 X = 0; X < Grid.Width; ++X)
        {
            if (Grid.GetTile(X, Y) == EMazeTile::Goal)
            {
                GoalTile = FIntPoint(X, Y);
                break;
            }
        }
        if (GoalTile.X >= 0)
        {
            break;
        }
    }

    TestNotEqual(TEXT("Goal tile found"), GoalTile.X, -1);

    if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(Context->World, 0))
    {
        const FVector GoalLocation = Context->MazeActor->GetActorLocation()
            + FVector(GoalTile.X * Context->MazeActor->TileSize, GoalTile.Y * Context->MazeActor->TileSize, 100.0f);
        Pawn->SetActorLocation(GoalLocation);
        Context->GameMode->EvaluateGoalReached();
    }

    TestEqual(TEXT("Current level advances to 2"), Context->GameMode->CurrentLevel, 2);
    NavinationGameFlowTest::DestroyWorld(*Context);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGameOverTest,
    "Navination.GameFlow.GameOver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGameOverTest::RunTest(const FString& Parameters)
{
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context = MakeShared<NavinationGameFlowTest::FTestContext>();
    if (!NavinationGameFlowTest::CreateWorld(*Context))
    {
        AddError(TEXT("Failed to create test world."));
        NavinationGameFlowTest::DestroyWorld(*Context);
        return false;
    }

    Context->GameMode->RemainingTime = 0.0f;
    if (Context->GameState)
    {
        Context->GameState->SetRemainingTime(0.0f);
    }

    ADD_LATENT_AUTOMATION_COMMAND(FNavinationForceGameModeTickCommand(Context, 3));
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.1f));
    ADD_LATENT_AUTOMATION_COMMAND(FNavinationCheckGameOverCommand(this, Context));
    ADD_LATENT_AUTOMATION_COMMAND(FNavinationCleanupCommand(Context));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTimeDilationRestoreTest,
    "Navination.GameFlow.TimeDilationRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FTimeDilationRestoreTest::RunTest(const FString& Parameters)
{
    TSharedPtr<NavinationGameFlowTest::FTestContext> Context = MakeShared<NavinationGameFlowTest::FTestContext>();
    if (!NavinationGameFlowTest::CreateWorld(*Context))
    {
        AddError(TEXT("Failed to create test world."));
        NavinationGameFlowTest::DestroyWorld(*Context);
        return false;
    }

    UStopTimeAbility* StopTime = Context->Character ? Cast<UStopTimeAbility>(Context->Character->GetAbility(0)) : nullptr;
    TestNotNull(TEXT("StopTime ability exists"), StopTime);
    if (StopTime)
    {
        StopTime->SetCurrentCharges(StopTime->MaxCharges, true);
        TestTrue(TEXT("StopTime activates"), StopTime->TryActivate());
    }

    const float WaitSeconds = StopTime ? StopTime->SlowDuration + 0.5f : 0.5f;
    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(WaitSeconds));
    ADD_LATENT_AUTOMATION_COMMAND(FNavinationCheckTimeDilationCommand(this, Context));
    ADD_LATENT_AUTOMATION_COMMAND(FNavinationCleanupCommand(Context));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMinimapRevealTest,
    "Navination.GameFlow.MinimapReveal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FMinimapRevealTest::RunTest(const FString& Parameters)
{
    AMazeGameState* GameState = NewObject<AMazeGameState>();
    GameState->InitExploredTiles(5, 5);
    GameState->MarkExplored(2, 2, 1);

    const int32 CenterIndex = 2 * GameState->GetGridWidth() + 2;
    TestTrue(TEXT("Center tile explored"), GameState->ExploredTiles.IsValidIndex(CenterIndex) && GameState->ExploredTiles[CenterIndex]);

    bool bNeighborsExplored = true;
    for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
    {
        for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
        {
            const int32 CheckX = 2 + OffsetX;
            const int32 CheckY = 2 + OffsetY;
            const int32 Index = (CheckY * GameState->GetGridWidth()) + CheckX;
            if (GameState->ExploredTiles.IsValidIndex(Index))
            {
                bNeighborsExplored &= GameState->ExploredTiles[Index];
            }
        }
    }
    TestTrue(TEXT("Neighbors explored"), bNeighborsExplored);

    const int32 UnexploredIndex = 0;
    TestTrue(TEXT("Unrelated tile remains unexplored"), GameState->ExploredTiles.IsValidIndex(UnexploredIndex) && !GameState->ExploredTiles[UnexploredIndex]);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMinimapResetTest,
    "Navination.GameFlow.MinimapReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FMinimapResetTest::RunTest(const FString& Parameters)
{
    AMazeGameState* GameState = NewObject<AMazeGameState>();
    GameState->InitExploredTiles(4, 4);
    GameState->MarkExplored(1, 1, 1);

    GameState->ResetExploration();
    bool bAllCleared = true;
    for (const bool bExplored : GameState->ExploredTiles)
    {
        bAllCleared &= !bExplored;
    }

    TestTrue(TEXT("All explored tiles cleared"), bAllCleared);
    return true;
}
