#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_FormSwitch.generated.h"

/**
 * UGA_FormSwitch
 * Cycles to the next available form or switches to a specific form.
 */
UCLASS()
class BURNINGCORE_API UGA_FormSwitch : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_FormSwitch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Form")
	FGameplayTag TargetFormTag; // If empty, cycles through available
};
