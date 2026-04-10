#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"
#include "GA_PlatformerBaseHit.generated.h"

class UGameplayEffect;

/**
 * Standard close-range melee strike for a platformer character.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UGA_PlatformerBaseHit : public UGA_PlatformerCombatAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlatformerBaseHit();

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
	virtual float GetMeleeAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const;
	virtual float GetMeleeDamageAmount(const FGameplayAbilityActorInfo* ActorInfo) const;
	virtual const FPlatformerMeleeHitSettings& GetMeleeHitSettings() const;
	virtual bool ExecuteConfiguredMeleeHit(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		float AdditionalDamageMultiplier = 1.0f) const;

	UPROPERTY(EditDefaultsOnly, Category="Melee")
	FPlatformerMeleeHitSettings MeleeHitSettings;

	UPROPERTY(EditDefaultsOnly, Category="Melee|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	mutable float LastMeleeActivationTime = -1.0f;
};
