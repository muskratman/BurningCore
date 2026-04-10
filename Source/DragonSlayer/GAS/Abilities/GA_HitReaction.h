#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerHitReaction.h"
#include "GA_HitReaction.generated.h"

class UAnimMontage;

/**
 * UGA_HitReaction
 * Played when taking significant damage or stagger. Interrupts current actions.
 */
UCLASS()
class DRAGONSLAYER_API UGA_HitReaction : public UGA_PlatformerHitReaction
{
	GENERATED_BODY()
	
public:
	UGA_HitReaction();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	virtual UAnimMontage* GetHitReactionMontage(const FGameplayAbilityActorInfo* ActorInfo) const override;
};
