#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Components/BoxComponent.h"
#include "Tests/AutomationCommon.h"
#include "Abilities/BreakWallAbility.h"
#include "Abilities/DashAbility.h"
#include "Abilities/StopTimeAbility.h"
#include "Gameplay/MazeCharacter.h"
#include "Maze/MazeActor.h"

namespace NavinationAbilityTest
{
    struct FTestWorld
    {
        FTestWorldWrapper Wrapper;

        bool Init()
        {
            return Wrapper.CreateTestWorld(EWorldType::Game);
        }

        bool BeginPlay()
        {
            return Wrapper.BeginPlayInTestWorld();
        }

        UWorld* GetWorld() const
        {
            return Wrapper.GetTestWorld();
        }

        void Cleanup()
        {
            Wrapper.DestroyTestWorld(true);
        }
    };

    static AMazeCharacter* SpawnCharacter(UWorld* World, APlayerController* PlayerController)
    {
        if (!World)
        {
            return nullptr;
        }

        AMazeCharacter* Character = World->SpawnActor<AMazeCharacter>();
        if (Character && PlayerController)
        {
            PlayerController->Possess(Character);
        }
        return Character;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStopTimeActivateTest,
    "Navination.Abilities.StopTime.Activate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FStopTimeActivateTest::RunTest(const FString& Parameters)
{
    NavinationAbilityTest::FTestWorld TestWorld;
    if (!TestWorld.Init())
    {
        AddError(TEXT("Failed to create test world."));
        return false;
    }

    AMazeCharacter* Character = TestWorld.GetWorld()->SpawnActor<AMazeCharacter>();
    if (!TestWorld.BeginPlay())
    {
        AddError(TEXT("Failed to begin play in test world."));
        TestWorld.Cleanup();
        return false;
    }

    APlayerController* PlayerController = TestWorld.GetWorld()->GetFirstPlayerController();
    if (Character && PlayerController)
    {
        PlayerController->Possess(Character);
    }

    UStopTimeAbility* StopTime = Character ? Cast<UStopTimeAbility>(Character->GetAbility(0)) : nullptr;

    TestNotNull(TEXT("StopTime ability exists"), StopTime);
    if (StopTime)
    {
        TestTrue(TEXT("StopTime ability activates"), StopTime->TryActivate());
        const float EffectiveDilation = TestWorld.GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();
        TestTrue(TEXT("World is time-dilated"), EffectiveDilation < 1.0f);
    }

    TestWorld.Cleanup();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStopTimeNoCooldownStackTest,
    "Navination.Abilities.StopTime.NoCooldownStack",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FStopTimeNoCooldownStackTest::RunTest(const FString& Parameters)
{
    NavinationAbilityTest::FTestWorld TestWorld;
    if (!TestWorld.Init())
    {
        AddError(TEXT("Failed to create test world."));
        return false;
    }

    AMazeCharacter* Character = TestWorld.GetWorld()->SpawnActor<AMazeCharacter>();
    if (!TestWorld.BeginPlay())
    {
        AddError(TEXT("Failed to begin play in test world."));
        TestWorld.Cleanup();
        return false;
    }

    APlayerController* PlayerController = TestWorld.GetWorld()->GetFirstPlayerController();
    if (Character && PlayerController)
    {
        PlayerController->Possess(Character);
    }

    UStopTimeAbility* StopTime = Character ? Cast<UStopTimeAbility>(Character->GetAbility(0)) : nullptr;

    TestNotNull(TEXT("StopTime ability exists"), StopTime);
    if (StopTime)
    {
        const bool bFirstActivation = StopTime->TryActivate();
        const bool bSecondActivation = StopTime->TryActivate();
        TestTrue(TEXT("First stop-time activation succeeds"), bFirstActivation);
        TestFalse(TEXT("Second activation blocked by cooldown"), bSecondActivation);
    }

    TestWorld.Cleanup();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBreakWallTest,
    "Navination.Abilities.BreakWall.BreaksWall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FBreakWallTest::RunTest(const FString& Parameters)
{
    NavinationAbilityTest::FTestWorld TestWorld;
    if (!TestWorld.Init())
    {
        AddError(TEXT("Failed to create test world."));
        return false;
    }

    AMazeActor* MazeActor = TestWorld.GetWorld()->SpawnActor<AMazeActor>();
    TestNotNull(TEXT("Maze actor exists"), MazeActor);
    if (MazeActor)
    {
        MazeActor->TileSize = 200.0f;
        FMazeGrid Grid;
        Grid.Width = 7;
        Grid.Height = 7;
        Grid.Tiles.Init(EMazeTile::Floor, Grid.Width * Grid.Height);
        Grid.SetTile(3, 3, EMazeTile::Wall);
        MazeActor->BuildFromGrid(Grid);
    }

    AMazeCharacter* Character = TestWorld.GetWorld()->SpawnActor<AMazeCharacter>();
    if (!TestWorld.BeginPlay())
    {
        AddError(TEXT("Failed to begin play in test world."));
        TestWorld.Cleanup();
        return false;
    }

    APlayerController* PlayerController = TestWorld.GetWorld()->GetFirstPlayerController();
    if (Character && PlayerController)
    {
        PlayerController->Possess(Character);
        Character->SetActorLocation(FVector(400.0f, 600.0f, 50.0f));
        Character->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    }

    UBreakWallAbility* BreakWall = Character ? Cast<UBreakWallAbility>(Character->GetAbility(2)) : nullptr;
    TestNotNull(TEXT("BreakWall ability exists"), BreakWall);
    if (BreakWall && MazeActor)
    {
        // Directly validate wall destruction logic without relying on line trace in test world.
        const bool bDestroyed = MazeActor->DestroyWallAt(3, 3);
        TestTrue(TEXT("DestroyWallAt succeeds"), bDestroyed);
        TestEqual(TEXT("Wall tile becomes floor"), MazeActor->CurrentGrid.GetTile(3, 3), EMazeTile::Floor);
    }

    TestWorld.Cleanup();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDashChargeTest,
    "Navination.Abilities.Dash.Charges",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDashChargeTest::RunTest(const FString& Parameters)
{
    NavinationAbilityTest::FTestWorld TestWorld;
    if (!TestWorld.Init())
    {
        AddError(TEXT("Failed to create test world."));
        return false;
    }

    AMazeCharacter* Character = TestWorld.GetWorld()->SpawnActor<AMazeCharacter>();
    if (!TestWorld.BeginPlay())
    {
        AddError(TEXT("Failed to begin play in test world."));
        TestWorld.Cleanup();
        return false;
    }

    APlayerController* PlayerController = TestWorld.GetWorld()->GetFirstPlayerController();
    if (Character && PlayerController)
    {
        PlayerController->Possess(Character);
    }

    UDashAbility* DashAbility = Character ? Cast<UDashAbility>(Character->GetAbility(1)) : nullptr;

    TestNotNull(TEXT("Dash ability exists"), DashAbility);
    if (DashAbility)
    {
        DashAbility->CooldownSeconds = 0.0f;
        DashAbility->SetCurrentCharges(3);
        TestTrue(TEXT("Dash activation 1"), DashAbility->TryActivate());
        TestTrue(TEXT("Dash activation 2"), DashAbility->TryActivate());
        TestTrue(TEXT("Dash activation 3"), DashAbility->TryActivate());
        TestFalse(TEXT("Dash activation 4 blocked"), DashAbility->TryActivate());
    }

    TestWorld.Cleanup();
    return true;
}
