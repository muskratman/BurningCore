#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_HitReaction.generated.h"

class UAnimMontage;

/**
 * UGA_HitReaction
 * Played when taking significant damage or stagger. Interrupts current actions.
 */
UCLASS()
class BURNINGCORE_API UGA_HitReaction : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_HitReaction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;
};
