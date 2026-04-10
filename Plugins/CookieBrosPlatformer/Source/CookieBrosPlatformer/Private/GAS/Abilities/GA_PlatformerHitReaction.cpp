#include "GAS/Abilities/GA_PlatformerHitReaction.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/PlatformerGameplayTags.h"

UGA_PlatformerHitReaction::UGA_PlatformerHitReaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData HitTrigger;
	HitTrigger.TriggerTag = PlatformerGameplayTags::Event_Combat_HitReceived;
	HitTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(HitTrigger);
}

void UGA_PlatformerHitReaction::ActivateAbility(
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

	if (UAnimMontage* HitReactionMontage = GetHitReactionMontage(ActorInfo))
	{
		if (UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			HitReactionMontage,
			1.0f))
		{
			Task->OnBlendOut.AddDynamic(this, &UGA_PlatformerHitReaction::K2_EndAbility);
			Task->OnCompleted.AddDynamic(this, &UGA_PlatformerHitReaction::K2_EndAbility);
			Task->OnInterrupted.AddDynamic(this, &UGA_PlatformerHitReaction::K2_EndAbility);
			Task->OnCancelled.AddDynamic(this, &UGA_PlatformerHitReaction::K2_EndAbility);
			Task->ReadyForActivation();
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UAnimMontage* UGA_PlatformerHitReaction::GetHitReactionMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return nullptr;
}
