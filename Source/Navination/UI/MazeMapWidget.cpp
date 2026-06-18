#include "MazeMapWidget.h"
#include "MazeActor.h"
#include "MazeCharacter.h"
#include "MazeGameState.h"
#include "MazeTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"

void UMazeMapWidget::SetMazeReferences(AMazeGameState* GameState, AMazeActor* MazeActor, AMazeCharacter* MazeCharacter)
{
    if (GameStateRef.IsValid())
    {
        GameStateRef->OnExplorationUpdated.RemoveDynamic(this, &UMazeMapWidget::HandleExplorationUpdated);
    }

    GameStateRef = GameState;
    MazeActorRef = MazeActor;
    MazeCharacterRef = MazeCharacter;

    if (GameStateRef.IsValid())
    {
        GameStateRef->OnExplorationUpdated.AddDynamic(this, &UMazeMapWidget::HandleExplorationUpdated);
    }

    Invalidate(EInvalidateWidgetReason::Paint);
}

void UMazeMapWidget::SetTeleportMarker(const FIntPoint& Tile, bool bEnabled)
{
    TeleportMarkerTile = Tile;
    bTeleportMarkerEnabled = bEnabled;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void UMazeMapWidget::NativeDestruct()
{
    if (GameStateRef.IsValid())
    {
        GameStateRef->OnExplorationUpdated.RemoveDynamic(this, &UMazeMapWidget::HandleExplorationUpdated);
    }

    Super::NativeDestruct();
}

int32 UMazeMapWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    if (!GameStateRef.IsValid() || !MazeActorRef.IsValid())
    {
        return LayerId;
    }

    const int32 GridWidth = GameStateRef->GetGridWidth();
    const int32 GridHeight = GameStateRef->GetGridHeight();
    if (GridWidth <= 0 || GridHeight <= 0)
    {
        return LayerId;
    }

    const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();
    const float MapSize = FMath::Min3(
        static_cast<float>(WidgetSize.X),
        static_cast<float>(WidgetSize.Y),
        200.0f
    );
    const float TileSize = FMath::Max(4.0f, FMath::Min(MapSize / GridWidth, MapSize / GridHeight));
    const FVector2D MapPixelSize(TileSize * GridWidth, TileSize * GridHeight);
    const FVector2D MapOrigin = (WidgetSize - MapPixelSize) * 0.5f;

    const FSlateBrush* Brush = FAppStyle::Get().GetBrush("WhiteBrush");
    const FMazeGrid& Grid = MazeActorRef->CurrentGrid;

    const APawn* Pawn = MazeCharacterRef.IsValid()
        ? Cast<APawn>(MazeCharacterRef.Get())
        : UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector MazeOriginWorld = MazeActorRef->GetActorLocation();
    const float WorldTileSize = MazeActorRef->TileSize;
    const FVector PlayerLocal = Pawn ? Pawn->GetActorLocation() - MazeOriginWorld : FVector::ZeroVector;
    const int32 PlayerTileX = Pawn ? FMath::FloorToInt(PlayerLocal.X / WorldTileSize) : -1;
    const int32 PlayerTileY = Pawn ? FMath::FloorToInt(PlayerLocal.Y / WorldTileSize) : -1;

    const int32 RevealRadius = 2;

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            const int32 Index = (Y * GridWidth) + X;
            const bool bExplored = GameStateRef->ExploredTiles.IsValidIndex(Index) && GameStateRef->ExploredTiles[Index];
            const EMazeTile Tile = Grid.GetTile(X, Y);

            bool bDrawGoal = (Tile == EMazeTile::Goal);
            if (Tile == EMazeTile::Goal && !bExplored)
            {
                const int32 Dist = FMath::Max(FMath::Abs(PlayerTileX - X), FMath::Abs(PlayerTileY - Y));
                bDrawGoal = Dist <= (RevealRadius * 2);
            }

            FLinearColor TileColor(0.05f, 0.05f, 0.05f, 0.9f);
            if (bExplored)
            {
                switch (Tile)
                {
                    case EMazeTile::Floor:
                        TileColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
                        break;
                    case EMazeTile::Wall:
                        TileColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
                        break;
                    case EMazeTile::Start:
                        TileColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
                        break;
                    case EMazeTile::Goal:
                        TileColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
                        break;
                    default:
                        break;
                }
            }
            else if (bDrawGoal)
            {
                TileColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
            }

            const FVector2D TilePos = MapOrigin + FVector2D(X * TileSize, Y * TileSize);
            const FVector2D TileDrawSize(TileSize, TileSize);
            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(TilePos, TileDrawSize),
                Brush,
                ESlateDrawEffect::None,
                TileColor
            );
        }
    }

    if (Pawn && PlayerTileX >= 0 && PlayerTileY >= 0)
    {
        const FVector2D PlayerPos = MapOrigin + FVector2D(PlayerTileX * TileSize, PlayerTileY * TileSize);
        const FVector2D PlayerDotSize(3.0f, 3.0f);
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(PlayerPos + (FVector2D(TileSize, TileSize) * 0.5f) - (PlayerDotSize * 0.5f), PlayerDotSize),
            Brush,
            ESlateDrawEffect::None,
            FLinearColor::White
        );
    }

    if (bTeleportMarkerEnabled && TeleportMarkerTile.X >= 0 && TeleportMarkerTile.Y >= 0)
    {
        const FVector2D MarkerPos = MapOrigin + FVector2D(TeleportMarkerTile.X * TileSize, TeleportMarkerTile.Y * TileSize);
        const FVector2D MarkerSize(3.0f, 3.0f);
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(MarkerPos + (FVector2D(TileSize, TileSize) * 0.5f) - (MarkerSize * 0.5f), MarkerSize),
            Brush,
            ESlateDrawEffect::None,
            FLinearColor(0.1f, 0.9f, 1.0f, 1.0f)
        );
    }

    return LayerId + 1;
}

void UMazeMapWidget::HandleExplorationUpdated()
{
    Invalidate(EInvalidateWidgetReason::Paint);
}
