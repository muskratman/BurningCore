#include "GAS/Abilities/GA_PlatformerDash.h"

#include "Character/PlatformerCharacterBase.h"
#include "GameFramework/Character.h"
#include "GAS/PlatformerGameplayTags.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"

UGA_PlatformerDash::UGA_PlatformerDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(PlatformerTraversalGameplayTags::Ability_Movement_Dash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	ActivationBlockedTags.AddTag(PlatformerGameplayTags::State_Movement_Ladder);
}

bool UGA_PlatformerDash::CanActivateAbility(
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
	const APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(Character);
	if (PlatformerCharacter && (PlatformerCharacter->IsOnLadder() || PlatformerCharacter->IsLadderTopFinishActive()))
	{
		return false;
	}

	const UPlatformerTraversalMovementComponent* TraversalMovementComponent =
		Character ? Cast<UPlatformerTraversalMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	return TraversalMovementComponent
		&& (bAllowAirDash || Character->GetCharacterMovement()->IsMovingOnGround());
}

void UGA_PlatformerDash::ActivateAbility(
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

	if (!TraversalMovementComponent || !TraversalMovementComponent->StartDash())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
