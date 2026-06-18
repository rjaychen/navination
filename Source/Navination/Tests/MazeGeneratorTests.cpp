#include "Misc/AutomationTest.h"
#include "Maze/MazeGenerator.h"
#include "Maze/MazeTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMazePerfectTest,
    "Navination.Maze.PerfectMazeProperty",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FMazePerfectTest::RunTest(const FString& Parameters)
{
    const int32 CellWidth = 5;
    const int32 CellHeight = 5;
    const FMazeGrid Grid = AMazeGenerator::GenerateMaze(CellWidth, CellHeight, 42);

    int32 PassageCount = 0;
    for (int32 Y = 0; Y < Grid.Height; ++Y)
    {
        for (int32 X = 0; X < Grid.Width; ++X)
        {
            const bool bIsEdgePassage = (X % 2 == 0) ^ (Y % 2 == 0);
            if (bIsEdgePassage && Grid.GetTile(X, Y) == EMazeTile::Floor)
            {
                ++PassageCount;
            }
        }
    }

    const int32 ExpectedPassages = (CellWidth * CellHeight) - 1;
    TestEqual(TEXT("Carved passages count"), PassageCount, ExpectedPassages);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMazeBFSGoalTest,
    "Navination.Maze.GoalDistance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FMazeBFSGoalTest::RunTest(const FString& Parameters)
{
    const FMazeGrid Grid = AMazeGenerator::GenerateMaze(5, 5, 42);
    FIntPoint GoalTile(-1, -1);

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

    TestTrue(TEXT("Goal tile found"), GoalTile.X >= 0 && GoalTile.Y >= 0);
    const FVector2D StartTile(1.0f, 1.0f);
    const FVector2D Goal(GoalTile.X, GoalTile.Y);
    const FVector2D FarthestCorner(Grid.Width - 2, Grid.Height - 2);
    const float Distance = FVector2D::Distance(StartTile, Goal);
    const float MaxDistance = FVector2D::Distance(StartTile, FarthestCorner);

    TestTrue(TEXT("Goal tile is in the far half of the maze"), Distance >= MaxDistance * 0.5f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMazeBoundaryTest,
    "Navination.Maze.BoundaryGetSet",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FMazeBoundaryTest::RunTest(const FString& Parameters)
{
    FMazeGrid Grid = AMazeGenerator::GenerateMaze(3, 3, 7);

    Grid.SetTile(-1, -1, EMazeTile::Floor);
    Grid.SetTile(Grid.Width + 1, Grid.Height + 1, EMazeTile::Floor);

    TestEqual(TEXT("GetTile safely handles negative indices"), Grid.GetTile(-1, -1), EMazeTile::Wall);
    TestEqual(TEXT("GetTile safely handles positive indices"), Grid.GetTile(Grid.Width, Grid.Height), EMazeTile::Wall);
    return true;
}
