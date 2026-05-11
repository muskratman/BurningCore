#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyBase.h"
#include "PlatformerEnemyRanged.generated.h"

class AEnemyProjectile;

/**
 * APlatformerEnemyRanged
 * Generic ranged enemy shell.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerEnemyRanged : public APlatformerEnemyBase
{
	GENERATED_BODY()

public:
	APlatformerEnemyRanged(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category="Combat|Ranged")
	float GetEnemyProjectileSpeed() const { return ProjectileSpeed; }

	UFUNCTION(BlueprintCallable, Category="Combat|Ranged")
	void SetEnemyProjectileSpeed(float InProjectileSpeed);

protected:
	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged")
	FName ProjectileSpawnSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged", meta=(ClampMin=0.0, Units="cm"))
	float ProjectileSpawnForwardOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged", meta=(ClampMin=0.0, Units="cm"))
	float ProjectileSpawnUpOffset = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged", meta=(ClampMin=0.0, Units="cm"))
	float RangedAttackRange = 1200.0f;

	UPROPERTY(EditAnywhere, Category="Combat|Ranged", meta=(ClampMin=0.0, Units="cm/s"))
	float ProjectileSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged", meta=(ClampMin=0.0, Units="s"))
	float ProjectileLifetime = 5.0f;

	virtual float GetAttackRange() const override;
	virtual float GetAttackCooldown() const override;
	virtual float GetAttackDamageAmount() const override;
	virtual bool ApplyAttackHit(APlatformerCombatCharacterBase* TargetActor) override;
	virtual FGameplayTag GetAttackAnimationTagForTarget(const APlatformerCombatCharacterBase* TargetActor) const override;
	virtual void ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype) override;
};
