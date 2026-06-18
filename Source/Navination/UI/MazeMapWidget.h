#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MazeMapWidget.generated.h"

class AMazeActor;
class AMazeCharacter;
class AMazeGameState;

UCLASS()
class UMazeMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetMazeReferences(AMazeGameState* GameState, AMazeActor* MazeActor, AMazeCharacter* MazeCharacter);
    void SetTeleportMarker(const FIntPoint& Tile, bool bEnabled);

protected:
    virtual void NativeDestruct() override;
    virtual int32 NativePaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;

private:
    TWeakObjectPtr<AMazeGameState> GameStateRef;
    TWeakObjectPtr<AMazeActor> MazeActorRef;
    TWeakObjectPtr<AMazeCharacter> MazeCharacterRef;
    bool bTeleportMarkerEnabled = false;
    FIntPoint TeleportMarkerTile = FIntPoint(-1, -1);

    UFUNCTION()
    void HandleExplorationUpdated();
};
