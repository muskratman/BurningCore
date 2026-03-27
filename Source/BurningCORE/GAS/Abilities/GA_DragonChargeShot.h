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
class BURNINGCORE_API UGA_DragonChargeShot : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_DragonChargeShot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> ChargeDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ChargeLoopMontage;
};
