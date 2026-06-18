#include "Abilities/BreakWallAbility.h"
#include "Maze/MazeActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UBreakWallAbility::UBreakWallAbility()
{
    MaxCharges = 2;
}

bool UBreakWallAbility::ActivateAbility()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return false;
    }

    APlayerController* PlayerController = Cast<APlayerController>(OwnerActor->GetInstigatorController());
    if (!PlayerController)
    {
        PlayerController = Cast<APlayerController>(OwnerActor->GetWorld()->GetFirstPlayerController());
    }

    FVector ViewLocation = OwnerActor->GetActorLocation();
    FRotator ViewRotation = OwnerActor->GetActorRotation();
    if (PlayerController)
    {
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }

    const FVector TraceStart = ViewLocation;
    const FVector TraceEnd = TraceStart + (ViewRotation.Vector() * TraceRange);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerActor);

    if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        return false;
    }

    AMazeActor* MazeActor = Cast<AMazeActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMazeActor::StaticClass()));
    if (!MazeActor)
    {
        return false;
    }

    const FVector LocalHit = Hit.ImpactPoint - MazeActor->GetActorLocation();
    const int32 TileX = FMath::FloorToInt(LocalHit.X / MazeActor->TileSize);
    const int32 TileY = FMath::FloorToInt(LocalHit.Y / MazeActor->TileSize);

    if (!MazeActor->DestroyWallAt(TileX, TileY))
    {
        return false;
    }

    BP_OnActivated();
    BP_PlayScreenEffect();
    return true;
}
