#include "GAS/Abilities/GA_PlatformerBaseShot.h"

#include "Character/PlatformerCharacterBase.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_PlatformerBaseShot::UGA_PlatformerBaseShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_SlideDash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
}

bool UGA_PlatformerBaseShot::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return PassesActivationDelay(ActorInfo, GetBaseShotAttackDelay(ActorInfo), LastBaseShotActivationTime);
}

void UGA_PlatformerBaseShot::ActivateAbility(
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

	FPlatformerProjectileShotData ShotData;
	if (!BuildBaseShotData(Handle, ActorInfo, ShotData) || !SpawnConfiguredCombatProjectile(ActorInfo, ShotData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	LastBaseShotActivationTime = GetAbilityWorldTime(ActorInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

float UGA_PlatformerBaseShot::GetBaseShotAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const APlatformerCharacterBase* Character = GetPlatformerCharacter(ActorInfo);
	const UPlatformerCharacterAttributeSet* AttributeSet = GetPlatformerAttributeSet(ActorInfo);
	if (!Character || !Character->HasActiveDeveloperCombatSettings() || !AttributeSet)
	{
		return 0.0f;
	}

	return FMath::Max(AttributeSet->GetRangeAttackDelay(), 0.0f);
}
