#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerChargeShot.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "GA_DragonChargeShot.generated.h"

class UGameplayEffect;
class UAnimMontage;
class UDragonFormDataAsset;
class ADragonCharacter;

/**
 * UGA_DragonChargeShot
 * Hold-to-charge, release-to-fire ability.
 */
UCLASS()
class DRAGONSLAYER_API UGA_DragonChargeShot : public UGA_PlatformerChargeShot
{
	GENERATED_BODY()
	
public:
	UGA_DragonChargeShot();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> ChargeDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ChargeLoopMontage;

private:
	virtual UAnimMontage* GetChargeLoopMontage(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual bool GetChargeShotTuning(const FGameplayAbilityActorInfo* ActorInfo, FPlatformerChargeShotTuning& OutChargeTuning) const override;
	virtual bool BuildChargeShotData(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FPlatformerChargeShotTuning& ChargeTuning,
		EPlatformerChargeShotStage ChargeStage,
		FPlatformerProjectileShotData& OutShotData) const override;
};
