#include "MazeActor.h"
#include "Navination.h"
#include "MazeTileSet.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "TimerManager.h"

namespace
{
    constexpr uint8 NorthBit = 1 << 0;
    constexpr uint8 EastBit  = 1 << 1;
    constexpr uint8 SouthBit = 1 << 2;
    constexpr uint8 WestBit  = 1 << 3;
}

AMazeActor::AMazeActor()
    : Super()
{
    FloorISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISMC"));
    RootComponent = FloorISMC;

    WallISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISMC"));
    WallISMC->SetupAttachment(RootComponent);

    WallStraightISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallStraightISMC"));
    WallStraightISMC->SetupAttachment(RootComponent);

    WallCornerISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallCornerISMC"));
    WallCornerISMC->SetupAttachment(RootComponent);

    WallJunctionISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallJunctionISMC"));
    WallJunctionISMC->SetupAttachment(RootComponent);

    WallDeadEndISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallDeadEndISMC"));
    WallDeadEndISMC->SetupAttachment(RootComponent);

    WallPillarISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallPillarISMC"));
    WallPillarISMC->SetupAttachment(RootComponent);

    StartISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StartISMC"));
    StartISMC->SetupAttachment(RootComponent);

    GoalISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GoalISMC"));
    GoalISMC->SetupAttachment(RootComponent);
}

void AMazeActor::ClearInstances()
{
    for (UInstancedStaticMeshComponent* ISMC : { FloorISMC, WallISMC, WallStraightISMC, WallCornerISMC, WallJunctionISMC,
        WallDeadEndISMC, WallPillarISMC, StartISMC, GoalISMC })
    {
        if (ISMC)
        {
            ISMC->ClearInstances();
        }
    }
}

void AMazeActor::ConfigureTileISMC(UInstancedStaticMeshComponent* ISMC, const FMazeTileVisual* Visual, UStaticMesh* FallbackMesh) const
{
    if (!ISMC)
    {
        return;
    }

    if (Visual && Visual->Mesh)
    {
        ISMC->SetStaticMesh(Visual->Mesh);
    }
    else if (FallbackMesh)
    {
        ISMC->SetStaticMesh(FallbackMesh);
    }

    if (Visual && Visual->Material)
    {
        ISMC->SetMaterial(0, Visual->Material);
    }
}

void AMazeActor::ConfigureWallISMC(UInstancedStaticMeshComponent* ISMC, EMazeWallShape Shape) const
{
    const FMazeTileVisual* Visual = TileSet ? TileSet->GetWallVisual(Shape) : nullptr;
    ConfigureTileISMC(ISMC, Visual, WallMesh);
}

void AMazeActor::BuildFromGrid(const FMazeGrid& Grid)
{
    CurrentGrid = Grid;
    WallNeighborMask.Init(0, Grid.Width * Grid.Height);

    ClearInstances();

    ConfigureTileISMC(FloorISMC, TileSet ? TileSet->GetTileVisual(EMazeTile::Floor) : nullptr, FloorMesh);
    ConfigureTileISMC(StartISMC, TileSet ? TileSet->GetTileVisual(EMazeTile::Start) : nullptr, StartMesh);
    ConfigureTileISMC(GoalISMC, TileSet ? TileSet->GetTileVisual(EMazeTile::Goal) : nullptr, GoalMesh);

    ConfigureWallISMC(WallStraightISMC, EMazeWallShape::Straight);
    ConfigureWallISMC(WallCornerISMC, EMazeWallShape::Corner);
    ConfigureWallISMC(WallJunctionISMC, EMazeWallShape::Junction);
    ConfigureWallISMC(WallDeadEndISMC, EMazeWallShape::DeadEnd);
    ConfigureWallISMC(WallPillarISMC, EMazeWallShape::Pillar);

    const float ScaleFactor = TileSize / 100.0f;
    
    // Scales
    const FVector FloorScale{ Grid.Width * ScaleFactor * 0.125f,
                              Grid.Height * ScaleFactor * 0.125f,
                              1.0f };
    const FVector MarkerScale{ ScaleFactor * 0.25f };

    // Z-Heights
    const float FloorZ  = 0.0f;
    const float MarkerZ = TileSize * 0.125f;

    const float CenterX = Grid.Width * TileSize * 0.5f;
    const float CenterY = Grid.Height * TileSize * 0.5f;

    FloorISMC->AddInstance(FTransform{ FRotator::ZeroRotator, 
                                        FVector(CenterX, CenterY, FloorZ),
                                        FloorScale });

    BuildWallInstances();

    for (int32 ty = 0; ty < Grid.Height; ++ty)
    {
        for (int32 tx = 0; tx < Grid.Width; ++tx)
        {
            const EMazeTile Tile = Grid.GetTile(tx, ty);

            const float TileX = static_cast<float>(tx) * TileSize;
            const float TileY = static_cast<float>(ty) * TileSize;

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

    ApplyGoalTint();
}

bool AMazeActor::DestroyWallAt(int32 TileX, int32 TileY)
{
    if (CurrentGrid.GetTile(TileX, TileY) != EMazeTile::Wall)
    {
        return false;
    }

    CurrentGrid.SetTile(TileX, TileY, EMazeTile::Floor);
    BuildWallInstances();

    const FVector WorldLocation = GetActorLocation()
        + FVector(static_cast<float>(TileX) * TileSize, static_cast<float>(TileY) * TileSize, TileSize * 0.25f);
    SpawnRubbleAt(WorldLocation);
    return true;
}

bool AMazeActor::RegrowWallAt(int32 TileX, int32 TileY)
{
    const EMazeTile CurrentTile = CurrentGrid.GetTile(TileX, TileY);
    if (CurrentTile != EMazeTile::Floor)
    {
        return false;
    }

    CurrentGrid.SetTile(TileX, TileY, EMazeTile::Wall);
    BuildWallInstances();
    return true;
}

void AMazeActor::SetGoalTintForLevel(int32 Level)
{
    const int32 Denominator = FMath::Max(GoalTintMaxLevel, 1);
    GoalTintLevelProgress = FMath::Clamp(static_cast<float>(Level - 1) / static_cast<float>(Denominator), 0.0f, 1.0f);
    ApplyGoalTint();
}

void AMazeActor::BuildWallInstances()
{
    for (UInstancedStaticMeshComponent* ISMC : { WallISMC, WallStraightISMC, WallCornerISMC, WallJunctionISMC,
        WallDeadEndISMC, WallPillarISMC })
    {
        if (ISMC)
        {
            ISMC->ClearInstances();
        }
    }

    if (CurrentGrid.Width <= 0 || CurrentGrid.Height <= 0)
    {
        return;
    }

    const int32 ExpectedSize = CurrentGrid.Width * CurrentGrid.Height;
    if (WallNeighborMask.Num() != ExpectedSize)
    {
        WallNeighborMask.Init(0, ExpectedSize);
    }

    const float ScaleFactor = TileSize / 100.0f;
    const FVector WallScale{ ScaleFactor };

    for (int32 ty = 0; ty < CurrentGrid.Height; ++ty)
    {
        for (int32 tx = 0; tx < CurrentGrid.Width; ++tx)
        {
            if (CurrentGrid.GetTile(tx, ty) != EMazeTile::Wall)
            {
                continue;
            }

            const uint8 Mask = BuildNeighborMask(tx, ty);
            WallNeighborMask[ty * CurrentGrid.Width + tx] = Mask;

            const EMazeWallShape Shape = ClassifyWallShape(Mask);
            UInstancedStaticMeshComponent* TargetISMC = GetWallISMC(Shape);
            if (!TargetISMC)
            {
                continue;
            }

            const float TileX = static_cast<float>(tx) * TileSize;
            const float TileY = static_cast<float>(ty) * TileSize;
            const FVector Location{ TileX, TileY, 0.0f };
            const FRotator Rotation = GetWallRotation(Mask, Shape);

            TargetISMC->AddInstance(FTransform{ Rotation, Location, WallScale });
        }
    }
}

uint8 AMazeActor::BuildNeighborMask(int32 TileX, int32 TileY) const
{
    uint8 Mask = 0;
    if (CurrentGrid.GetTile(TileX, TileY - 1) != EMazeTile::Wall) Mask |= NorthBit;
    if (CurrentGrid.GetTile(TileX + 1, TileY) != EMazeTile::Wall) Mask |= EastBit;
    if (CurrentGrid.GetTile(TileX, TileY + 1) != EMazeTile::Wall) Mask |= SouthBit;
    if (CurrentGrid.GetTile(TileX - 1, TileY) != EMazeTile::Wall) Mask |= WestBit;
    return Mask;
}

EMazeWallShape AMazeActor::ClassifyWallShape(uint8 NeighborMask) const
{
    const int32 OpenCount =
        ((NeighborMask & NorthBit) ? 1 : 0) +
        ((NeighborMask & EastBit) ? 1 : 0) +
        ((NeighborMask & SouthBit) ? 1 : 0) +
        ((NeighborMask & WestBit) ? 1 : 0);

    if (OpenCount == 0)
    {
        return EMazeWallShape::Pillar;
    }
    if (OpenCount == 1)
    {
        return EMazeWallShape::DeadEnd;
    }
    if (OpenCount == 2)
    {
        const bool bNorthSouth = (NeighborMask & NorthBit) && (NeighborMask & SouthBit);
        const bool bEastWest = (NeighborMask & EastBit) && (NeighborMask & WestBit);
        return (bNorthSouth || bEastWest) ? EMazeWallShape::Straight : EMazeWallShape::Corner;
    }
    return EMazeWallShape::Junction;
}

FRotator AMazeActor::GetWallRotation(uint8 NeighborMask, EMazeWallShape Shape) const
{
    float Yaw = 0.0f;
    switch (Shape)
    {
        case EMazeWallShape::Straight:
        {
            const bool bEastWest = (NeighborMask & EastBit) && (NeighborMask & WestBit);
            Yaw = bEastWest ? 90.0f : 0.0f;
            break;
        }
        case EMazeWallShape::Corner:
        {
            if ((NeighborMask & NorthBit) && (NeighborMask & EastBit)) Yaw = 0.0f;
            else if ((NeighborMask & EastBit) && (NeighborMask & SouthBit)) Yaw = 90.0f;
            else if ((NeighborMask & SouthBit) && (NeighborMask & WestBit)) Yaw = 180.0f;
            else if ((NeighborMask & WestBit) && (NeighborMask & NorthBit)) Yaw = 270.0f;
            break;
        }
        case EMazeWallShape::DeadEnd:
        {
            if (NeighborMask & NorthBit) Yaw = 0.0f;
            else if (NeighborMask & EastBit) Yaw = 90.0f;
            else if (NeighborMask & SouthBit) Yaw = 180.0f;
            else if (NeighborMask & WestBit) Yaw = 270.0f;
            break;
        }
        case EMazeWallShape::Junction:
        {
            if (!(NeighborMask & SouthBit)) Yaw = 0.0f;
            else if (!(NeighborMask & WestBit)) Yaw = 90.0f;
            else if (!(NeighborMask & NorthBit)) Yaw = 180.0f;
            else if (!(NeighborMask & EastBit)) Yaw = 270.0f;
            break;
        }
        default:
            break;
    }

    return FRotator(0.0f, Yaw, 0.0f);
}

UInstancedStaticMeshComponent* AMazeActor::GetWallISMC(EMazeWallShape Shape) const
{
    switch (Shape)
    {
        case EMazeWallShape::Straight: return WallStraightISMC;
        case EMazeWallShape::Corner: return WallCornerISMC;
        case EMazeWallShape::Junction: return WallJunctionISMC;
        case EMazeWallShape::DeadEnd: return WallDeadEndISMC;
        case EMazeWallShape::Pillar: return WallPillarISMC;
        default: return WallISMC;
    }
}

void AMazeActor::ApplyGoalTint()
{
    if (!GoalISMC)
    {
        return;
    }

    UMaterialInterface* BaseMaterial = GoalISMC->GetMaterial(0);
    if (!BaseMaterial)
    {
        return;
    }

    GoalTintMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!GoalTintMID)
    {
        return;
    }

    const FLinearColor BaseColor = TileSet ? TileSet->GoalBaseColor : FLinearColor::White;
    const FLinearColor GoldColor = TileSet ? TileSet->GoalGoldColor : FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
    const FName ParamName = TileSet ? TileSet->GoalTintParameter : FName(TEXT("TintColor"));
    const FLinearColor Tint = FMath::Lerp(BaseColor, GoldColor, GoalTintLevelProgress);
    GoalTintMID->SetVectorParameterValue(ParamName, Tint);
    GoalISMC->SetMaterial(0, GoalTintMID);
}

void AMazeActor::SpawnRubbleAt(const FVector& WorldLocation)
{
    if (!GetWorld())
    {
        return;
    }

    AActor* RubbleActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), WorldLocation, FRotator::ZeroRotator);
    if (!IsValid(RubbleActor))
    {
        return;
    }

    USceneComponent* Root = NewObject<USceneComponent>(RubbleActor, TEXT("RubbleRoot"));
    Root->SetMobility(EComponentMobility::Movable);
    Root->RegisterComponent();
    RubbleActor->SetRootComponent(Root);

    UProceduralMeshComponent* RubbleMesh = NewObject<UProceduralMeshComponent>(RubbleActor, TEXT("RubbleMesh"));
    RubbleMesh->SetupAttachment(Root);
    RubbleMesh->bUseAsyncCooking = true;
    RubbleMesh->RegisterComponent();

    const float HalfExtent = TileSize * 0.25f;
    const TArray<FVector> Vertices = {
        FVector(-HalfExtent, -HalfExtent, -HalfExtent),
        FVector(HalfExtent, -HalfExtent, -HalfExtent),
        FVector(HalfExtent, HalfExtent, -HalfExtent),
        FVector(-HalfExtent, HalfExtent, -HalfExtent),
        FVector(-HalfExtent, -HalfExtent, HalfExtent),
        FVector(HalfExtent, -HalfExtent, HalfExtent),
        FVector(HalfExtent, HalfExtent, HalfExtent),
        FVector(-HalfExtent, HalfExtent, HalfExtent)
    };

    const TArray<int32> Triangles = {
        0, 2, 1, 0, 3, 2,
        4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 5, 4,
        1, 2, 6, 1, 6, 5,
        2, 3, 7, 2, 7, 6,
        3, 0, 4, 3, 4, 7
    };

    TArray<FVector> Normals;
    Normals.Init(FVector::UpVector, Vertices.Num());

    TArray<FVector2D> UV0;
    UV0.Init(FVector2D::ZeroVector, Vertices.Num());

    TArray<FColor> VertexColors;
    VertexColors.Init(FColor::White, Vertices.Num());

    TArray<FProcMeshTangent> Tangents;
    Tangents.Init(FProcMeshTangent(), Vertices.Num());

    RubbleMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
    RubbleMesh->SetSimulatePhysics(true);
    RubbleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UMaterialInterface* WallMaterial = nullptr;
    if (TileSet)
    {
        const FMazeTileVisual* WallVisual = TileSet->GetWallVisual(EMazeWallShape::Straight);
        WallMaterial = (WallVisual && WallVisual->Material) ? WallVisual->Material : nullptr;
    }

    if (WallMaterial)
    {
        RubbleMesh->SetMaterial(0, WallMaterial);
    }

    URadialForceComponent* RadialForce = NewObject<URadialForceComponent>(RubbleActor, TEXT("RubbleForce"));
    RadialForce->SetupAttachment(Root);
    RadialForce->Radius = TileSize * 0.75f;
    RadialForce->ImpulseStrength = 2000.0f;
    RadialForce->bImpulseVelChange = true;
    RadialForce->RegisterComponent();
    RadialForce->FireImpulse();

    FTimerHandle DestroyHandle;
    GetWorldTimerManager().SetTimer(
        DestroyHandle,
        FTimerDelegate::CreateWeakLambda(RubbleActor, [RubbleActor]()
        {
            if (IsValid(RubbleActor))
            {
                RubbleActor->Destroy();
            }
        }),
        3.0f,
        false
    );
}