#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerBaseShot.h"
#include "GA_DragonBaseShot.generated.h"

class UGameplayEffect;

/**
 * UGA_DragonBaseShot
 * Fires a projectile based on the current Form active in the Character's FormComponent.
 */
UCLASS()
class DRAGONSLAYER_API UGA_DragonBaseShot : public UGA_PlatformerBaseShot
{
	GENERATED_BODY()
	
public:
	UGA_DragonBaseShot();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> BaseDamageEffectClass;

	virtual bool BuildBaseShotData(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FPlatformerProjectileShotData& OutShotData) const override;
};
