#pragma once

#include "CoreMinimal.h"
#include "NavinationCharacter.h"
#include "MazeCharacter.generated.h"

class UAbilityComponent;
class UAnimMontage;
class UAudioComponent;
class UBreakWallAbility;
class UDashAbility;
class UInputAction;
class UStopTimeAbility;
class UTeleportAbility;

UCLASS()
class AMazeCharacter : public ANavinationCharacter
{
    GENERATED_BODY()
public:
    AMazeCharacter();

    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool TryActivateAbility(int32 AbilityIndex);

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void ResetAbilityCharges();

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void ApplyAbilityUnlocks(int32 CurrentLevel);

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void GetAbilityCharges(TArray<int32>& OutCharges) const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    UAbilityComponent* GetAbility(int32 AbilityIndex) const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool IsAbilityUnlocked(int32 AbilityIndex, int32 CurrentLevel) const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    int32 GetAbilityUnlockLevel(int32 AbilityIndex) const;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    void PlayAbilityMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category="Audio")
    UAudioComponent* GetHeartbeatAudio() const { return HeartbeatAudio; }

    UFUNCTION(BlueprintCallable, Category="Audio")
    FName GetHeartbeatParameterName() const { return HeartbeatProximityParameter; }

    UFUNCTION(BlueprintCallable, Category="Extras")
    void StartBreadcrumbTrail();

    UFUNCTION(BlueprintCallable, Category="Extras")
    void StopBreadcrumbTrail();

    UFUNCTION(BlueprintCallable, Category="Extras")
    void RecordGhostSample();

    UFUNCTION(BlueprintCallable, Category="Extras")
    void TriggerMazeSpeaks(const FText& Subtitle);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditAnywhere, Category="Movement")
    float MazeWalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* Ability1Action;

    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* Ability2Action;

    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* Ability3Action;

    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* Ability4Action;

    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* PauseAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    TArray<TObjectPtr<UAbilityComponent>> AbilitySlots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
    TObjectPtr<UAudioComponent> HeartbeatAudio;

    UPROPERTY(EditAnywhere, Category="Audio")
    FName HeartbeatProximityParameter = FName(TEXT("Proximity"));

    UPROPERTY(EditAnywhere, Category="Abilities")
    TArray<int32> AbilityUnlockLevels;

private:
    void HandleAbility1();
    void HandleAbility2();
    void HandleAbility3();
    void HandleAbility4();
    void HandlePause();

    void ActivateAbilityByIndex(int32 AbilityIndex);
};
