#include "Navination.h"
#include "MazeTypes.h"
#include "MazeGenerator.h"

AMazeGenerator::AMazeGenerator()
    : Super()
{
    MazeActorClass = AMazeActor::StaticClass();
}

void AMazeGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    const FMazeGrid Grid = GenerateMaze(MazeWidth, MazeHeight, Seed);
    
    UE_LOG(LogNavination, Log, TEXT("Maze has width: %d, height: %d"), Grid.Width, Grid.Height);
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    const FVector SpawnLocation = GetActorLocation();
    const FRotator SpawnRotation = GetActorRotation();

    if (AMazeActor* MazeActor = GetWorld()->SpawnActor<AMazeActor>(
            MazeActorClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams))
    {
        MazeActor->BuildFromGrid(Grid);
    }
}

int32 AMazeGenerator::FindRoot(TArray<int32>& Parent, int32 Node)
{
    if (Parent[Node] != Node)
        Parent[Node] = FindRoot(Parent, Parent[Node]);
    return Parent[Node];
}

void AMazeGenerator::Union(TArray<int32>& Parent, int32 A, int32 B)
{
    int32 RootA = FindRoot(Parent, A);
    int32 RootB = FindRoot(Parent, B);
    if (RootA != RootB)
        Parent[RootA] = RootB;
}

FMazeGrid AMazeGenerator::GenerateMaze(int32 InWidth, int32 InHeight, int32 InSeed)
{
    struct FEdge { int32 CellA, CellB, WallX, WallY; };

    FMazeGrid Grid;
    Grid.Width = 2 * InWidth + 1;
    Grid.Height = 2 * InHeight + 1;

    Grid.Tiles.SetNum(Grid.Width * Grid.Height);
    
    for (EMazeTile& T : Grid.Tiles)
        T = EMazeTile::Wall;

    TArray<int32> Parent; // Union-Find
    Parent.SetNum(InWidth * InHeight);
    for (int32 i = 0; i < InWidth * InHeight; i++)
        Parent[i] = i;

    TArray<FEdge> Edges; // Edge List
    for (int32 cy = 0; cy < InHeight; cy++)
    {
        for (int32 cx = 0; cx < InWidth; cx++)
        {
            if (cy+1 < InHeight)
                Edges.Add(FEdge{ cy * InWidth + cx, (cy + 1) * InWidth + cx, 2 * cx + 1, 2 * cy + 2 }); // South Neighbor
            if (cx+1 < InWidth)
                Edges.Add(FEdge{ cy * InWidth + cx, cy * InWidth + cx + 1, 2 * cx + 2, 2 * cy + 1}); // East Neighbor
        }
    }

    FRandomStream Stream(InSeed); // Fisher-Yates shuffle
    for (int32 i = Edges.Num() - 1; i > 0; i--)
    {
        int32 j = Stream.RandHelper(i + 1);
        Edges.Swap(i, j);
    }

    for (FEdge& Edge : Edges) // Kruskal's algorithm
    {
        int32 RootA = FindRoot(Parent, Edge.CellA);
        int32 RootB = FindRoot(Parent, Edge.CellB);
        if (RootA != RootB)
        {
            Union(Parent, Edge.CellA, Edge.CellB);
            Grid.SetTile(Edge.WallX, Edge.WallY, EMazeTile::Floor);
            Grid.SetTile(2 * (Edge.CellA % InWidth) + 1, 2 * (Edge.CellA / InWidth) + 1, EMazeTile::Floor);
            Grid.SetTile(2 * (Edge.CellB % InWidth) + 1, 2 * (Edge.CellB / InWidth) + 1, EMazeTile::Floor);
        }
    }

    Grid.SetTile(1, 1, EMazeTile::Start); // 1-indexed -- odd coords are cell centers, even coords are walls
    TPair<int32, int32> Goal = BFSLongestPath(Grid, 1, 1);
    Grid.SetTile(Goal.Key, Goal.Value, EMazeTile::Goal);
    return Grid;
}

TPair<int32, int32> AMazeGenerator::BFSLongestPath(const FMazeGrid& Grid, int32 StartX, int32 StartY)
{
    TArray<bool> Visited;
    Visited.Init(false, Grid.Width * Grid.Height);

    TQueue<TTuple<int32, int32, int32>> Queue;
    Queue.Enqueue(TTuple<int32, int32, int32>(StartX, StartY, 0));
    Visited[StartY * Grid.Width + StartX] = true;

    int32 FarthestX = StartX, FarthestY = StartY;
    int32 MaxDistance = 0;

    static const int32 DX[] = {0, 1, 0, -1};
    static const int32 DY[] = {1, 0, -1, 0};

    while(!Queue.IsEmpty())
    {
        TTuple<int32, int32, int32> Current;
        Queue.Dequeue(Current);
        int32 CurrentDistance = Current.Get<2>();
        if (CurrentDistance > MaxDistance)
        {
            MaxDistance = CurrentDistance;
            FarthestX = Current.Get<0>();
            FarthestY = Current.Get<1>();
        }

        for (int32 i = 0; i < 4; i++)
        {
            int32 NewX = Current.Get<0>() + DX[i];
            int32 NewY = Current.Get<1>() + DY[i];
            if (NewX >= 0 && NewX < Grid.Width &&
                NewY >= 0 && NewY < Grid.Height &&
                !Visited[NewY * Grid.Width + NewX] &&
                Grid.GetTile(NewX, NewY) != EMazeTile::Wall)
            {
                Queue.Enqueue(TTuple<int32, int32, int32>(NewX, NewY, CurrentDistance + 1));
                Visited[NewY * Grid.Width + NewX] = true;
            }
        }
        
    }

    return TPair<int32, int32>(FarthestX, FarthestY);
}