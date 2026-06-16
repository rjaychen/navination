#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityComponent.generated.h"

UCLASS()
class UAbilityComponent : public UActorComponent
{
    GENERATED_BODY()
    public:
        UAbilityComponent();

        UPROPERTY(EditAnywhere)
        int32 NumCharges = 3;

        virtual void TryActivate();

    private:
        bool bIsOnCooldown = false;
};