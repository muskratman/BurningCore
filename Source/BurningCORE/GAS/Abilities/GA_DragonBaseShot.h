#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_DragonBaseShot.generated.h"

class UGameplayEffect;

/**
 * UGA_DragonBaseShot
 * Fires a projectile based on the current Form active in the Character's FormComponent.
 */
UCLASS()
class BURNINGCORE_API UGA_DragonBaseShot : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_DragonBaseShot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> BaseDamageEffectClass;
};
