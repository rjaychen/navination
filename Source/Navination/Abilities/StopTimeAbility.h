#pragma once

#include "CoreMinimal.h"
#include "Abilities/AbilityComponent.h"
#include "StopTimeAbility.generated.h"

class UAnimMontage;

UCLASS(ClassGroup=(Abilities), meta=(BlueprintSpawnableComponent))
class NAVINATION_API UStopTimeAbility : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UStopTimeAbility();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Timing")
    float SlowDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Timing")
    float TimeDilation = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Animation")
    TObjectPtr<UAnimMontage> AbilityMontage;

    UFUNCTION(BlueprintImplementableEvent, Category="Ability", meta=(DisplayName="On Activated"))
    void BP_OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category="Ability|Effects", meta=(DisplayName="On Screen Effect"))
    void BP_PlayScreenEffect();

    static bool WasStopTimeActivatedRecently(const UWorld* World, float WindowSeconds);

    void RestoreTimeDilation();

protected:
    virtual bool ActivateAbility() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    double SlowEndRealTime = -1.0;
    static double LastActivationRealTime;
};
