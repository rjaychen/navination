#include "Abilities/TeleportAbility.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

UTeleportAbility::UTeleportAbility()
{
    MaxCharges = 1;
    CooldownSeconds = 0.0f;
}

bool UTeleportAbility::ActivateAbility()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return false;
    }

    if (!bIsMarked)
    {
        MarkedLocation = OwnerActor->GetActorLocation();
        bIsMarked = true;
        SpawnBeacon();
    }
    else
    {
        OwnerActor->TeleportTo(MarkedLocation, OwnerActor->GetActorRotation());
        bIsMarked = false;
        DestroyBeacon();
    }

    CurrentCharges = MaxCharges + 1;

    BP_OnActivated();
    BP_PlayScreenEffect();
    return true;
}

void UTeleportAbility::ClearMarkedLocation()
{
    bIsMarked = false;
    MarkedLocation = FVector::ZeroVector;
    DestroyBeacon();
}

void UTeleportAbility::SpawnBeacon()
{
    if (!BeaconMesh || !GetWorld())
    {
        return;
    }

    AActor* SpawnedBeacon = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), MarkedLocation, FRotator::ZeroRotator);
    if (!SpawnedBeacon)
    {
        return;
    }

    USceneComponent* RootComponent = NewObject<USceneComponent>(SpawnedBeacon);
    RootComponent->RegisterComponent();
    SpawnedBeacon->SetRootComponent(RootComponent);

    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(SpawnedBeacon);
    MeshComponent->SetStaticMesh(BeaconMesh);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->RegisterComponent();
    MeshComponent->SetWorldScale3D(FVector(BeaconScale));

    BeaconActor = SpawnedBeacon;
}

void UTeleportAbility::DestroyBeacon()
{
    if (BeaconActor.IsValid())
    {
        BeaconActor->Destroy();
    }
    BeaconActor = nullptr;
}
