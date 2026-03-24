#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_OverdriveActivate.generated.h"

/**
 * UGA_OverdriveActivate
 * Toggles the Overdrive mode on the Dragon if enough energy is stored.
 */
UCLASS()
class BURNINGCORE_API UGA_OverdriveActivate : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_OverdriveActivate();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
