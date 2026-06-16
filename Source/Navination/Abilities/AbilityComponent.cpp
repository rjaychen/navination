#include "AbilityComponent.h"

UAbilityComponent::UAbilityComponent()
    : Super()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityComponent::TryActivate() {}