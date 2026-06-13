#include "Navination.h"
#include "MazeActor.h"

AMazeActor::AMazeActor()
    : Super()
{
    FloorISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISMC"));
    RootComponent = FloorISMC;

    WallISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISMC"));
    WallISMC->SetupAttachment(RootComponent);
}

void AMazeActor::BuildFromGrid(const FMazeGrid& Grid)
{
    FloorISMC->ClearInstances();
    WallISMC->ClearInstances();

    if (FloorMesh)
        FloorISMC->SetStaticMesh(FloorMesh);
    if (WallMesh)
        WallISMC->SetStaticMesh(WallMesh);

    const float ScaleFactor = TileSize / 100.0f;
    const FVector WallScale{ ScaleFactor };

    for (int32 ty = 0; ty < Grid.Height; ty++)
    {
        for (int32 tx = 0; tx < Grid.Width; tx++)
        {
            const EMazeTile Tile = Grid.GetTile(tx, ty);
            
            const float Z = TileSize * 0.5f;
            const FVector Location{ static_cast<float>(tx) * TileSize, static_cast<float>(ty) * TileSize, Z };
            
            if (Tile == EMazeTile::Wall)
                WallISMC->AddInstance(FTransform{ FRotator::ZeroRotator, Location, WallScale });
        }
    }
}