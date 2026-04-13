#include "GAS/Abilities/GA_PlatformerJump.h"

#include "Character/PlatformerCharacterBase.h"
#include "Character/SideViewMovementComponent.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_PlatformerJump::UGA_PlatformerJump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Jump"))));
	// SetActivationOwnedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Movement.Airborne"))));
}

bool UGA_PlatformerJump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	return Character && Character->CanJump();
}

void UGA_PlatformerJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			const float DirectionSign = ResolveHorizontalJumpDirection(Character);
			Character->JumpMaxHoldTime = FMath::Max(0.0f, JumpMaxHoldTime);
			Character->Jump();

			if (UWorld* World = Character->GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick(
					FTimerDelegate::CreateUObject(
						this,
						&UGA_PlatformerJump::ApplyHorizontalJumpSpeedDeferred,
						TWeakObjectPtr<ACharacter>(Character),
						DirectionSign));
			}
		}
	}
}

void UGA_PlatformerJump::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			Character->StopJumping();
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_PlatformerJump::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UGA_PlatformerJump::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->StopJumping();
	}
}

void UGA_PlatformerJump::ApplyHorizontalJumpSpeedDeferred(TWeakObjectPtr<ACharacter> Character, float DirectionSign)
{
	if (ACharacter* ResolvedCharacter = Character.Get())
	{
		ApplyHorizontalJumpSpeed(ResolvedCharacter, DirectionSign);
	}
}

void UGA_PlatformerJump::ApplyHorizontalJumpSpeed(ACharacter* Character, float DirectionSign) const
{
	if (!Character)
	{
		return;
	}

	const EPlatformerJumpHorizontalSpeedMode ResolvedJumpHorizontalSpeedMode = ResolveHorizontalJumpSpeedMode(Character);
	if (ResolvedJumpHorizontalSpeedMode == EPlatformerJumpHorizontalSpeedMode::None)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (FMath::IsNearlyZero(DirectionSign))
	{
		return;
	}

	const float ResolvedJumpHorizontalSpeed = ResolveHorizontalJumpSpeed(Character);
	if (ResolvedJumpHorizontalSpeed <= 0.0f)
	{
		return;
	}

	FVector Velocity = MovementComponent->Velocity;
	const float CurrentAlignedSpeed = Velocity.X * DirectionSign;

	switch (ResolvedJumpHorizontalSpeedMode)
	{
	case EPlatformerJumpHorizontalSpeedMode::ClampMin:
		Velocity.X = DirectionSign * FMath::Max(CurrentAlignedSpeed, ResolvedJumpHorizontalSpeed);
		break;

	case EPlatformerJumpHorizontalSpeedMode::Override:
		Velocity.X = DirectionSign * ResolvedJumpHorizontalSpeed;
		break;

	case EPlatformerJumpHorizontalSpeedMode::None:
	default:
		return;
	}

	MovementComponent->Velocity = Velocity;

	if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
	{
		SideViewMovementComponent->NotifyJumpHorizontalSpeedApplied(FMath::Abs(Velocity.X), DirectionSign);
	}
}

EPlatformerJumpHorizontalSpeedMode UGA_PlatformerJump::ResolveHorizontalJumpSpeedMode(const ACharacter* Character) const
{
	EPlatformerJumpHorizontalSpeedMode ResolvedJumpHorizontalSpeedMode = JumpHorizontalSpeedMode;

	if (ResolvedJumpHorizontalSpeedMode == EPlatformerJumpHorizontalSpeedMode::None)
	{
		if (const APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(Character))
		{
			if (PlatformerCharacter->HasDeveloperJumpHorizontalSpeedOverride())
			{
				ResolvedJumpHorizontalSpeedMode = EPlatformerJumpHorizontalSpeedMode::Override;
			}
		}
	}

	return ResolvedJumpHorizontalSpeedMode;
}

float UGA_PlatformerJump::ResolveHorizontalJumpSpeed(const ACharacter* Character) const
{
	float ResolvedJumpHorizontalSpeed = JumpHorizontalSpeed;

	if (const APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(Character))
	{
		ResolvedJumpHorizontalSpeed = PlatformerCharacter->ResolveDeveloperJumpHorizontalSpeed(ResolvedJumpHorizontalSpeed);
	}

	return FMath::Max(ResolvedJumpHorizontalSpeed, 0.0f);
}

float UGA_PlatformerJump::ResolvePreviewHorizontalJumpSpeed(
	const ACharacter* Character,
	float CurrentHorizontalSpeed,
	float DirectionSign) const
{
	const EPlatformerJumpHorizontalSpeedMode ResolvedJumpHorizontalSpeedMode = ResolveHorizontalJumpSpeedMode(Character);
	if (ResolvedJumpHorizontalSpeedMode == EPlatformerJumpHorizontalSpeedMode::None)
	{
		return FMath::Max(CurrentHorizontalSpeed, 0.0f);
	}

	if (FMath::IsNearlyZero(DirectionSign))
	{
		return FMath::Max(CurrentHorizontalSpeed, 0.0f);
	}

	const float ResolvedJumpHorizontalSpeed = ResolveHorizontalJumpSpeed(Character);
	if (ResolvedJumpHorizontalSpeed <= 0.0f)
	{
		return FMath::Max(CurrentHorizontalSpeed, 0.0f);
	}

	const float CurrentAlignedSpeed = CurrentHorizontalSpeed * FMath::Sign(DirectionSign);

	switch (ResolvedJumpHorizontalSpeedMode)
	{
	case EPlatformerJumpHorizontalSpeedMode::ClampMin:
		return FMath::Max(CurrentAlignedSpeed, ResolvedJumpHorizontalSpeed);

	case EPlatformerJumpHorizontalSpeedMode::Override:
		return ResolvedJumpHorizontalSpeed;

	case EPlatformerJumpHorizontalSpeedMode::None:
	default:
		return FMath::Max(CurrentHorizontalSpeed, 0.0f);
	}
}

float UGA_PlatformerJump::ResolveHorizontalJumpDirection(const ACharacter* Character) const
{
	if (!Character)
	{
		return 0.0f;
	}

	if (const UPlatformerTraversalMovementComponent* TraversalMovementComponent = Cast<UPlatformerTraversalMovementComponent>(Character->GetCharacterMovement()))
	{
		const float TraversalInputX = TraversalMovementComponent->GetTraversalInputVector().X;
		if (!FMath::IsNearlyZero(TraversalInputX))
		{
			return FMath::Sign(TraversalInputX);
		}

		return 0.0f;
	}

	const FVector PendingMovementInput = Character->GetPendingMovementInputVector();
	if (!FMath::IsNearlyZero(PendingMovementInput.X))
	{
		return FMath::Sign(PendingMovementInput.X);
	}

	return 0.0f;
}
