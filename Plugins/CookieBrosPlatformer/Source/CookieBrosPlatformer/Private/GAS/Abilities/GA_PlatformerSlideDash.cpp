#include "GAS/Abilities/GA_PlatformerSlideDash.h"

#include "GameFramework/Character.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"

UGA_PlatformerSlideDash::UGA_PlatformerSlideDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(PlatformerTraversalGameplayTags::Ability_Movement_SlideDash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_SlideDash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
}

bool UGA_PlatformerSlideDash::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UPlatformerTraversalMovementComponent* TraversalMovementComponent =
		Character ? Cast<UPlatformerTraversalMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	return TraversalMovementComponent && Character->GetCharacterMovement()->IsMovingOnGround();
}

void UGA_PlatformerSlideDash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UPlatformerTraversalMovementComponent* TraversalMovementComponent =
		Character ? Cast<UPlatformerTraversalMovementComponent>(Character->GetCharacterMovement()) : nullptr;

	if (!TraversalMovementComponent || !TraversalMovementComponent->StartSlideDash())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
