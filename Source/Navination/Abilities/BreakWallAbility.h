#pragma once

#include "CoreMinimal.h"
#include "Abilities/AbilityComponent.h"
#include "BreakWallAbility.generated.h"

class UAnimMontage;

UCLASS(ClassGroup=(Abilities), meta=(BlueprintSpawnableComponent))
class NAVINATION_API UBreakWallAbility : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UBreakWallAbility();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|BreakWall")
    float TraceRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Animation")
    TObjectPtr<UAnimMontage> AbilityMontage;

    UFUNCTION(BlueprintImplementableEvent, Category="Ability", meta=(DisplayName="On Activated"))
    void BP_OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category="Ability|Effects", meta=(DisplayName="On Screen Effect"))
    void BP_PlayScreenEffect();

protected:
    virtual bool ActivateAbility() override;
};
