#include "MazeCharacter.h"
#include "Abilities/AbilityComponent.h"
#include "Abilities/BreakWallAbility.h"
#include "Abilities/DashAbility.h"
#include "Abilities/StopTimeAbility.h"
#include "Abilities/TeleportAbility.h"
#include "Animation/AnimInstance.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "MazeGameMode.h"
#include "MazeGameState.h"
#include "Navination.h"

AMazeCharacter::AMazeCharacter()
{
    JumpMaxCount = 0;
    GetCharacterMovement()->JumpZVelocity = 0.0f;

    AbilityUnlockLevels = { 7, 1, 3, 5 };

    AbilitySlots.SetNum(4);
    AbilitySlots[0] = CreateDefaultSubobject<UStopTimeAbility>(TEXT("StopTimeAbility"));
    AbilitySlots[1] = CreateDefaultSubobject<UDashAbility>(TEXT("DashAbility"));
    AbilitySlots[2] = CreateDefaultSubobject<UBreakWallAbility>(TEXT("BreakWallAbility"));
    AbilitySlots[3] = CreateDefaultSubobject<UTeleportAbility>(TEXT("TeleportAbility"));

    HeartbeatAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudio"));
    if (HeartbeatAudio)
    {
        HeartbeatAudio->SetupAttachment(GetRootComponent());
        HeartbeatAudio->bAutoActivate = true;
    }
}

void AMazeCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = MazeWalkSpeed;
}

void AMazeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMazeCharacter::MoveInput);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMazeCharacter::LookInput);
        EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMazeCharacter::LookInput);

        EnhancedInputComponent->BindAction(Ability1Action, ETriggerEvent::Started, this, &AMazeCharacter::HandleAbility1);
        EnhancedInputComponent->BindAction(Ability2Action, ETriggerEvent::Started, this, &AMazeCharacter::HandleAbility2);
        EnhancedInputComponent->BindAction(Ability3Action, ETriggerEvent::Started, this, &AMazeCharacter::HandleAbility3);
        EnhancedInputComponent->BindAction(Ability4Action, ETriggerEvent::Started, this, &AMazeCharacter::HandleAbility4);
        EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AMazeCharacter::HandlePause);
    }
    else
    {
        UE_LOG(LogNavination, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
    }
}

bool AMazeCharacter::TryActivateAbility(int32 AbilityIndex)
{
    if (!AbilitySlots.IsValidIndex(AbilityIndex) || !IsValid(AbilitySlots[AbilityIndex]))
    {
        return false;
    }

    int32 CurrentLevel = 1;
    if (const AMazeGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMazeGameState>() : nullptr)
    {
        CurrentLevel = GameState->CurrentLevel;
    }

    if (!IsAbilityUnlocked(AbilityIndex, CurrentLevel))
    {
        return false;
    }

    const bool bActivated = AbilitySlots[AbilityIndex]->TryActivate();
    if (bActivated)
    {
        if (AMazeGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMazeGameState>() : nullptr)
        {
            TArray<int32> Charges;
            GetAbilityCharges(Charges);
            GameState->SetAbilityCharges(Charges);
        }
    }

    return bActivated;
}

void AMazeCharacter::ResetAbilityCharges()
{
    for (UAbilityComponent* Ability : AbilitySlots)
    {
        if (IsValid(Ability))
        {
            Ability->ResetCharges();
        }
    }
}

void AMazeCharacter::ApplyAbilityUnlocks(int32 CurrentLevel)
{
    for (int32 Index = 0; Index < AbilitySlots.Num(); ++Index)
    {
        UAbilityComponent* Ability = AbilitySlots.IsValidIndex(Index) ? AbilitySlots[Index] : nullptr;
        if (!IsValid(Ability))
        {
            continue;
        }

        if (IsAbilityUnlocked(Index, CurrentLevel))
        {
            Ability->ResetCharges();
        }
        else
        {
            Ability->SetCurrentCharges(0);
        }
    }
}

void AMazeCharacter::GetAbilityCharges(TArray<int32>& OutCharges) const
{
    int32 CurrentLevel = 1;
    if (const AMazeGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMazeGameState>() : nullptr)
    {
        CurrentLevel = GameState->CurrentLevel;
    }

    OutCharges.Reset();
    OutCharges.Reserve(AbilitySlots.Num());
    for (int32 Index = 0; Index < AbilitySlots.Num(); ++Index)
    {
        const UAbilityComponent* Ability = AbilitySlots[Index];
        if (!IsAbilityUnlocked(Index, CurrentLevel))
        {
            OutCharges.Add(0);
            continue;
        }

        OutCharges.Add(IsValid(Ability) ? Ability->GetCurrentCharges() : 0);
    }
}

UAbilityComponent* AMazeCharacter::GetAbility(int32 AbilityIndex) const
{
    return AbilitySlots.IsValidIndex(AbilityIndex) ? AbilitySlots[AbilityIndex] : nullptr;
}

void AMazeCharacter::PlayAbilityMontage(UAnimMontage* Montage)
{
    if (!Montage)
    {
        return;
    }

    if (USkeletalMeshComponent* MeshComponent = GetFirstPersonMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
        {
            AnimInstance->Montage_Play(Montage);
        }
    }
}

bool AMazeCharacter::IsAbilityUnlocked(int32 AbilityIndex, int32 CurrentLevel) const
{
    const int32 RequiredLevel = GetAbilityUnlockLevel(AbilityIndex);
    return CurrentLevel >= RequiredLevel;
}

int32 AMazeCharacter::GetAbilityUnlockLevel(int32 AbilityIndex) const
{
    if (AbilityUnlockLevels.IsValidIndex(AbilityIndex))
    {
        return AbilityUnlockLevels[AbilityIndex];
    }

    return 1;
}

void AMazeCharacter::StartBreadcrumbTrail()
{
    // TODO: Attach a particle trail to the player that fades after 5 seconds.
}

void AMazeCharacter::StopBreadcrumbTrail()
{
    // TODO: Stop and cleanup breadcrumb particle trail components.
}

void AMazeCharacter::RecordGhostSample()
{
    // TODO: Record per-frame positions for a speedrun ghost replay.
}

void AMazeCharacter::TriggerMazeSpeaks(const FText& Subtitle)
{
    // TODO: Display cryptic subtitle text with a low probability on ability use.
}

void AMazeCharacter::HandleAbility1()
{
    ActivateAbilityByIndex(0);
}

void AMazeCharacter::HandleAbility2()
{
    ActivateAbilityByIndex(1);
}

void AMazeCharacter::HandleAbility3()
{
    ActivateAbilityByIndex(2);
}

void AMazeCharacter::HandleAbility4()
{
    ActivateAbilityByIndex(3);
}

void AMazeCharacter::HandlePause()
{
    if (AMazeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMazeGameMode>() : nullptr)
    {
        GameMode->TogglePauseMenu();
    }
}

void AMazeCharacter::ActivateAbilityByIndex(int32 AbilityIndex)
{
    TryActivateAbility(AbilityIndex);
}
