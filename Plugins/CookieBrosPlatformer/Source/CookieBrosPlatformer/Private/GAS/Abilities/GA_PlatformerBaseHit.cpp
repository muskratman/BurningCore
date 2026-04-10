#include "GAS/Abilities/GA_PlatformerBaseHit.h"

#include "Character/PlatformerCharacterBase.h"
#include "GameFramework/Character.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_PlatformerBaseHit::UGA_PlatformerBaseHit()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_SlideDash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
}

bool UGA_PlatformerBaseHit::CanActivateAbility(
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

	return PassesActivationDelay(ActorInfo, GetMeleeAttackDelay(ActorInfo), LastMeleeActivationTime);
}

void UGA_PlatformerBaseHit::ActivateAbility(
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

	if (const UAnimMontage* AttackMontage = GetMeleeHitSettings().AttackMontage)
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->PlayAnimMontage(const_cast<UAnimMontage*>(AttackMontage));
		}
	}

	const bool bHitTarget = ExecuteConfiguredMeleeHit(Handle, ActorInfo);
	LastMeleeActivationTime = GetAbilityWorldTime(ActorInfo);

	if (bHitTarget && GetMeleeHitSettings().HitCueTag.IsValid())
	{
		ExecuteGameplayCue(ActorInfo, GetMeleeHitSettings().HitCueTag);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

float UGA_PlatformerBaseHit::GetMeleeAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const APlatformerCharacterBase* Character = GetPlatformerCharacter(ActorInfo);
	const UPlatformerCharacterAttributeSet* AttributeSet = GetPlatformerAttributeSet(ActorInfo);
	if (!Character || !Character->HasActiveDeveloperCombatSettings() || !AttributeSet)
	{
		return 0.0f;
	}

	return FMath::Max(AttributeSet->GetMeleeAttackDelay(), 0.0f);
}

float UGA_PlatformerBaseHit::GetMeleeDamageAmount(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UPlatformerCharacterAttributeSet* AttributeSet = GetPlatformerAttributeSet(ActorInfo);
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return FMath::Max(AttributeSet->GetMeleeAttackDamage(), 0.0f) * FMath::Max(GetMeleeHitSettings().DamageMultiplier, 0.0f);
}

const FPlatformerMeleeHitSettings& UGA_PlatformerBaseHit::GetMeleeHitSettings() const
{
	return MeleeHitSettings;
}

bool UGA_PlatformerBaseHit::ExecuteConfiguredMeleeHit(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	float AdditionalDamageMultiplier) const
{
	return PerformMeleeHit(
		ActorInfo,
		GetMeleeHitSettings(),
		GetMeleeDamageAmount(ActorInfo) * FMath::Max(AdditionalDamageMultiplier, 0.0f),
		DamageEffectClass,
		GetAbilityLevel(Handle, ActorInfo));
}
