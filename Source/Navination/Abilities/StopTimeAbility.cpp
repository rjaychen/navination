#include "Abilities/StopTimeAbility.h"
#include "GameFramework/WorldSettings.h"

double UStopTimeAbility::LastActivationRealTime = -1.0;

UStopTimeAbility::UStopTimeAbility()
{
    MaxCharges = 2;
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UStopTimeAbility::ActivateAbility()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    AWorldSettings* WorldSettings = World->GetWorldSettings();
    if (!WorldSettings)
    {
        return false;
    }

    const float ClampedDilation = FMath::Clamp(TimeDilation, 0.05f, 1.0f);
    WorldSettings->SetTimeDilation(ClampedDilation);
    LastActivationRealTime = World->GetRealTimeSeconds();
    SlowEndRealTime = LastActivationRealTime + SlowDuration;

    if (AActor* OwnerActor = GetOwner())
    {
        OwnerActor->CustomTimeDilation = 1.0f / ClampedDilation;
    }

    SetComponentTickEnabled(true);

    BP_OnActivated();
    BP_PlayScreenEffect();
    return true;
}

bool UStopTimeAbility::WasStopTimeActivatedRecently(const UWorld* World, float WindowSeconds)
{
    if (!World || LastActivationRealTime < 0.0)
    {
        return false;
    }

    const double CurrentTime = World->GetRealTimeSeconds();
    return (CurrentTime - LastActivationRealTime) <= WindowSeconds;
}

void UStopTimeAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (UWorld* World = GetWorld())
    {
        if (World->GetRealTimeSeconds() >= SlowEndRealTime)
        {
            RestoreTimeDilation();
        }
    }
}

void UStopTimeAbility::RestoreTimeDilation()
{
    SlowEndRealTime = -1.0;
    SetComponentTickEnabled(false);

    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            WorldSettings->SetTimeDilation(1.0f);
        }
    }

    if (AActor* OwnerActor = GetOwner())
    {
        OwnerActor->CustomTimeDilation = 1.0f;
    }
}
