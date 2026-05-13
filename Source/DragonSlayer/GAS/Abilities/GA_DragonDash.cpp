#include "GAS/Abilities/GA_DragonDash.h"

#include "Character/PlatformerCharacterBase.h"
#include "GameFramework/Character.h"
#include "GAS/PlatformerGameplayTags.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_DragonDash::UGA_DragonDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(PlatformerTraversalGameplayTags::Ability_Movement_Dash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
	ActivationBlockedTags.AddTag(PlatformerGameplayTags::State_Movement_Ladder);
}

bool UGA_DragonDash::CanActivateAbility(
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

	const APlatformerCharacterBase* PlatformerCharacter = ActorInfo ? Cast<APlatformerCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (PlatformerCharacter && (PlatformerCharacter->IsOnLadder() || PlatformerCharacter->IsLadderTopFinishActive()))
	{
		return false;
	}

	const UDragonDashMovementComponent* DragonDashMovementComponent = GetDragonDashMovementComponent(ActorInfo);
	return DragonDashMovementComponent && DragonDashMovementComponent->CanStartDragonDash();
}

void UGA_DragonDash::ActivateAbility(
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

	UDragonDashMovementComponent* DragonDashMovementComponent = GetDragonDashMovementComponent(ActorInfo);
	if (!DragonDashMovementComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FDragonDashRequest DashRequest;
	DashRequest.InputVector = DragonDashMovementComponent->GetTraversalInputVector();
	if (!DragonDashMovementComponent->StartDragonDash(DashRequest))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	DragonDashMovementComponent->OnDragonDashStateChanged.AddDynamic(this, &UGA_DragonDash::HandleDragonDashStateChanged);
}

void UGA_DragonDash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UDragonDashMovementComponent* DragonDashMovementComponent = GetDragonDashMovementComponent(ActorInfo))
	{
		DragonDashMovementComponent->OnDragonDashStateChanged.RemoveDynamic(this, &UGA_DragonDash::HandleDragonDashStateChanged);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DragonDash::HandleDragonDashStateChanged(EDragonDashState PreviousState, EDragonDashState NewState)
{
	if (NewState == EDragonDashState::GroundDash
		|| NewState == EDragonDashState::AirDash
		|| NewState == EDragonDashState::BounceDash)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, false);
}

UDragonDashMovementComponent* UGA_DragonDash::GetDragonDashMovementComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	return Character ? Cast<UDragonDashMovementComponent>(Character->GetCharacterMovement()) : nullptr;
}
