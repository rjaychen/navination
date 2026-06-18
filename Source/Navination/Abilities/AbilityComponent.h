#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityComponent.generated.h"

class IAbility
{
public:
    virtual ~IAbility() = default;
    virtual bool ActivateAbility() = 0;
    virtual void OnCooldownEnd() {}
};

UCLASS()
class UAbilityComponent : public UActorComponent, public IAbility
{
    GENERATED_BODY()
    public:
        UAbilityComponent();

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability")
        int32 MaxCharges = 3;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability")
        float CooldownSeconds = 1.0f;

        UFUNCTION(BlueprintCallable, Category="Ability")
        bool TryActivate();

        UFUNCTION(BlueprintCallable, Category="Ability")
        void ResetCharges();

    UFUNCTION(BlueprintCallable, Category="Ability")
    void SetCurrentCharges(int32 NewCharges, bool bClearCooldown = true);

        UFUNCTION(BlueprintCallable, Category="Ability")
        int32 GetCurrentCharges() const;

        UFUNCTION(BlueprintCallable, Category="Ability")
        bool IsOnCooldown() const;

    protected:
        virtual void BeginPlay() override;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
        int32 CurrentCharges = 0;

        virtual bool ActivateAbility() override;
        virtual void OnCooldownEnd() override;

    private:
        bool bIsOnCooldown = false;
        FTimerHandle CooldownHandle;

        void HandleCooldownFinished();
};