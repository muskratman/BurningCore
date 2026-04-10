#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerDash.h"
#include "GA_PlatformerSlideDash.generated.h"

/**
 * GAS wrapper for the traversal slide-dash custom movement state.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UGA_PlatformerSlideDash : public UGA_PlatformerDash
{
	GENERATED_BODY()

public:
	UGA_PlatformerSlideDash();

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
};
