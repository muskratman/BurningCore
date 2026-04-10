#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlatformerDash.generated.h"

/**
 * UGA_PlatformerDash
 * Handles rapid forward movement and applies a Cooldown.Dash tag.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UGA_PlatformerDash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlatformerDash();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashForce = 3000.0f;
};
