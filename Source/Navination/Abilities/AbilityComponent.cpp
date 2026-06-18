#include "AbilityComponent.h"
#include "TimerManager.h"

UAbilityComponent::UAbilityComponent()
    : Super()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentCharges = MaxCharges;
}

void UAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentCharges = MaxCharges;
    bIsOnCooldown = false;
}

bool UAbilityComponent::TryActivate()
{
    if (bIsOnCooldown || CurrentCharges <= 0)
    {
        return false;
    }

    if (!ActivateAbility())
    {
        return false;
    }

    CurrentCharges = FMath::Max(CurrentCharges - 1, 0);

    if (CooldownSeconds > 0.0f)
    {
        bIsOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(
            CooldownHandle,
            this,
            &UAbilityComponent::HandleCooldownFinished,
            CooldownSeconds,
            false
        );
    }

    return true;
}

void UAbilityComponent::ResetCharges()
{
    CurrentCharges = MaxCharges;
    bIsOnCooldown = false;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CooldownHandle);
    }
}

void UAbilityComponent::SetCurrentCharges(int32 NewCharges, bool bClearCooldown)
{
    CurrentCharges = FMath::Clamp(NewCharges, 0, FMath::Max(MaxCharges, 0));
    if (bClearCooldown)
    {
        bIsOnCooldown = false;
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(CooldownHandle);
        }
    }
}

int32 UAbilityComponent::GetCurrentCharges() const
{
    return CurrentCharges;
}

bool UAbilityComponent::IsOnCooldown() const
{
    return bIsOnCooldown;
}

bool UAbilityComponent::ActivateAbility()
{
    return false;
}

void UAbilityComponent::OnCooldownEnd()
{
}

void UAbilityComponent::HandleCooldownFinished()
{
    bIsOnCooldown = false;
    OnCooldownEnd();
}