#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"
#include "GA_PlatformerHitReaction.generated.h"

class UAnimMontage;

/**
 * Generic hit-reaction ability that reacts to the shared combat hit event and optionally plays a montage.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UGA_PlatformerHitReaction : public UGA_PlatformerCombatAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlatformerHitReaction();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	virtual UAnimMontage* GetHitReactionMontage(const FGameplayAbilityActorInfo* ActorInfo) const;
};
