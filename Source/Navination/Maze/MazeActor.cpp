#include "MazeActor.h"
#include "Navination.h"

AMazeActor::AMazeActor()
    : Super()
{
    FloorISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISMC"));
    RootComponent = FloorISMC;

    WallISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISMC"));
    WallISMC->SetupAttachment(RootComponent);

    StartISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StartISMC"));
    StartISMC->SetupAttachment(RootComponent);

    GoalISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GoalISMC"));
    GoalISMC->SetupAttachment(RootComponent);
}

void AMazeActor::BuildFromGrid(const FMazeGrid& Grid)
{
    CurrentGrid = Grid;
    
    for (UInstancedStaticMeshComponent* ISMC : { FloorISMC, WallISMC, StartISMC, GoalISMC })
    {
        if (ISMC) ISMC->ClearInstances();
    }

    if (FloorMesh) FloorISMC->SetStaticMesh(FloorMesh);
    if (WallMesh)  WallISMC->SetStaticMesh(WallMesh);
    if (StartMesh) StartISMC->SetStaticMesh(StartMesh);
    if (GoalMesh)  GoalISMC->SetStaticMesh(GoalMesh);

    const float ScaleFactor = TileSize / 100.0f;
    
    // Scales
    const FVector WallScale{ ScaleFactor };             
    const FVector FloorScale{ Grid.Width * ScaleFactor * 0.125f,
                              Grid.Height * ScaleFactor * 0.125f,
                              1.0f };
    const FVector MarkerScale{ ScaleFactor * 0.25f };

    // Z-Heights
    const float WallZ   = TileSize * 0.5f;
    const float FloorZ  = 0.0f;
    const float MarkerZ = TileSize * 0.125f;

    const float CenterX = Grid.Width * TileSize * 0.5f;
    const float CenterY = Grid.Height * TileSize * 0.5f;

    FloorISMC->AddInstance(FTransform{ FRotator::ZeroRotator, 
                                        FVector(CenterX, CenterY, FloorZ),
                                        FloorScale });

    for (int32 ty = 0; ty < Grid.Height; ++ty)
    {
        for (int32 tx = 0; tx < Grid.Width; ++tx)
        {
            const EMazeTile Tile = Grid.GetTile(tx, ty);

            const float TileX = static_cast<float>(tx) * TileSize;
            const float TileY = static_cast<float>(ty) * TileSize;

            if (Tile == EMazeTile::Wall)
            {
                const FVector Location{ TileX, TileY, 0 };
                WallISMC->AddInstance(FTransform{ FRotator::ZeroRotator, Location, WallScale });
            }

            // 2. Setup Start and Goal Markers
            UInstancedStaticMeshComponent* MarkerISMC = nullptr;
            switch (Tile)
            {
                case EMazeTile::Start: MarkerISMC = StartISMC; break;
                case EMazeTile::Goal:  MarkerISMC = GoalISMC;  break;
                default:               break; 
            }

            if (MarkerISMC)
            {
                const FVector Location{ TileX, TileY, MarkerZ };
                MarkerISMC->AddInstance(FTransform{ FRotator::ZeroRotator, Location, MarkerScale });
            }

        }
    }
}