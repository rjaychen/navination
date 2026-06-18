#include "Abilities/DashAbility.h"
#include "Abilities/StopTimeAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"

UDashAbility::UDashAbility()
{
    MaxCharges = 3;
    CooldownSeconds = 1.5f;
}

bool UDashAbility::ActivateAbility()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return false;
    }

    bool bBulletTimeDash = false;
    if (UWorld* World = GetWorld())
    {
        if (AWorldSettings* WorldSettings = World->GetWorldSettings())
        {
            const bool bTimeDilated = WorldSettings->GetEffectiveTimeDilation() < 1.0f;
            bBulletTimeDash = bTimeDilated && UStopTimeAbility::WasStopTimeActivatedRecently(World, 0.5f);
        }
    }

    FVector ViewLocation = OwnerCharacter->GetActorLocation();
    FRotator ViewRotation = OwnerCharacter->GetActorRotation();
    if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }

    const FVector DashDirection = ViewRotation.Vector().GetSafeNormal();
    const float DashScale = bBulletTimeDash ? BulletTimeDashMultiplier : 1.0f;
    OwnerCharacter->LaunchCharacter(DashDirection * DashDistance * DashScale, true, true);

    bIsInvincible = true;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InvincibilityHandle);
        World->GetTimerManager().SetTimer(
            InvincibilityHandle,
            this,
            &UDashAbility::ClearInvincibility,
            InvincibilityDuration,
            false
        );
    }

    BP_OnActivated();
    if (bBulletTimeDash)
    {
        BP_PlayBulletTimeEffect(BulletTimeEffectDuration);
    }
    else
    {
        BP_PlayScreenEffect();
    }
    return true;
}

void UDashAbility::ClearInvincibility()
{
    bIsInvincible = false;
}
