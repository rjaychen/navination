#pragma once

#include "CoreMinimal.h"
#include "Abilities/AbilityComponent.h"
#include "DashAbility.generated.h"

class UAnimMontage;

UCLASS(ClassGroup=(Abilities), meta=(BlueprintSpawnableComponent))
class NAVINATION_API UDashAbility : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UDashAbility();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Dash")
    float DashDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Dash")
    float InvincibilityDuration = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Dash")
    float BulletTimeDashMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Dash")
    float BulletTimeEffectDuration = 0.3f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability|Dash")
    bool bIsInvincible = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Animation")
    TObjectPtr<UAnimMontage> AbilityMontage;

    UFUNCTION(BlueprintImplementableEvent, Category="Ability", meta=(DisplayName="On Activated"))
    void BP_OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category="Ability|Effects", meta=(DisplayName="On Screen Effect"))
    void BP_PlayScreenEffect();

    UFUNCTION(BlueprintImplementableEvent, Category="Ability|Effects", meta=(DisplayName="On Bullet-Time Dash"))
    void BP_PlayBulletTimeEffect(float EffectDuration);

protected:
    virtual bool ActivateAbility() override;

private:
    FTimerHandle InvincibilityHandle;

    void ClearInvincibility();
};
