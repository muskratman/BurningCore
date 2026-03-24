#include "GAS/Abilities/GA_HitReaction.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_HitReaction::UGA_HitReaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Ability triggers on hit event
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Reaction.Hit"))));
	
	// Cancels dash, shoot, charge, etc.
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Shoot")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Dash")));
	
	// Block other actions while recovering
	// SetActivationOwnedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("State.Staggered"))));
}

void UGA_HitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HitReactMontage)
	{
		// Native equivalent of PlayMontageAndWait node
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, HitReactMontage, 1.0f);
			
		Task->OnBlendOut.AddDynamic(this, &UGA_HitReaction::K2_EndAbility);
		Task->OnCompleted.AddDynamic(this, &UGA_HitReaction::K2_EndAbility);
		Task->OnInterrupted.AddDynamic(this, &UGA_HitReaction::K2_EndAbility);
		Task->OnCancelled.AddDynamic(this, &UGA_HitReaction::K2_EndAbility);
		
		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
