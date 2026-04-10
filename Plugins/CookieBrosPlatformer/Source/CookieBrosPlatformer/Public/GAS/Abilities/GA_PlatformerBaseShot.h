#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"
#include "GA_PlatformerBaseShot.generated.h"

/**
 * Generic projectile shot ability flow for platformer characters.
 * Derived classes only need to resolve projectile/data specifics.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UGA_PlatformerBaseShot : public UGA_PlatformerCombatAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlatformerBaseShot();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	virtual bool BuildBaseShotData(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FPlatformerProjectileShotData& OutShotData) const PURE_VIRTUAL(UGA_PlatformerBaseShot::BuildBaseShotData, return false;);

	virtual float GetBaseShotAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const;

	mutable float LastBaseShotActivationTime = -1.0f;
};
