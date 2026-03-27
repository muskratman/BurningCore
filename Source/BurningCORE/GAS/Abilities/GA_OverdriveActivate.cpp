#include "GAS/Abilities/GA_OverdriveActivate.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonOverdriveComponent.h"

UGA_OverdriveActivate::UGA_OverdriveActivate()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Overdrive.Activate"))));
	
	// Can't press it if already in overdrive
	// SetActivationBlockedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("State.Overdrive"))));
}

bool UGA_OverdriveActivate::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	ADragonCharacter* Character = Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetOverdriveComponent())
	{
		return Character->GetOverdriveComponent()->GetOverdrivePercent() >= 1.0f; // Needs 100% to activate
	}

	return false;
}

void UGA_OverdriveActivate::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADragonCharacter* Character = Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetOverdriveComponent())
	{
		if (Character->GetOverdriveComponent()->TryActivateOverdrive())
		{
			// Apply GameplayEffect containing the "State.Overdrive" tag and any stat buffs
			// Play VFX/SFX via Montage or GameplayCue
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
