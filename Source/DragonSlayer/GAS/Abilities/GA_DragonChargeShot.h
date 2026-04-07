#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_DragonChargeShot.generated.h"

class UGameplayEffect;
class UAnimMontage;

/**
 * UGA_DragonChargeShot
 * Hold-to-charge, release-to-fire ability.
 */
UCLASS()
class DRAGONSLAYER_API UGA_DragonChargeShot : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_DragonChargeShot();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> ChargeDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ChargeLoopMontage;

	UPROPERTY(Transient)
	mutable float LastDeveloperChargedShotActivationTime = -1.0f;
};
