#include "GAS/Abilities/GA_PlatformerBaseShot.h"

#include "Animation/PlatformerAnimGameplayTags.h"
#include "Character/PlatformerCharacterBase.h"
#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/PlatformerGameplayTags.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_PlatformerBaseShot::UGA_PlatformerBaseShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
	ActivationBlockedTags.AddTag(PlatformerGameplayTags::State_Movement_Ladder);
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

	const APlatformerCharacterBase* PlatformerCharacter = GetPlatformerCharacter(ActorInfo);
	if (PlatformerCharacter && (PlatformerCharacter->IsOnLadder() || PlatformerCharacter->IsLadderTopFinishActive()))
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

	// Play feedback animation as soon as the ability commits — independent of
	// whether projectile data builds. This makes the visual fire even when the
	// projectile pipeline isn't fully configured (e.g. missing form data),
	// and lets PlayAbilityAnimation surface its own missing-tag warning instead
	// of being silently skipped on shot-build failure.
	PlayAbilityAnimation(ActorInfo, PlatformerAnimGameplayTags::Anim_Combat_RangedShot);

	const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	const FString AvatarName = AvatarActor ? AvatarActor->GetName() : TEXT("<null avatar>");

	FPlatformerProjectileShotData ShotData;
	if (!BuildBaseShotData(Handle, ActorInfo, ShotData))
	{
		UE_LOG(LogPlatformerCombatAbility, Log,
			TEXT("BaseShot[%s]: BuildBaseShotData returned false — projectile data missing (e.g. unset form/projectile class). Animation played, ability ended."),
			*AvatarName);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!SpawnConfiguredCombatProjectile(ActorInfo, ShotData))
	{
		UE_LOG(LogPlatformerCombatAbility, Log,
			TEXT("BaseShot[%s]: SpawnConfiguredCombatProjectile failed — invalid projectile class or world spawn rejected. Animation played, ability ended."),
			*AvatarName);
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
