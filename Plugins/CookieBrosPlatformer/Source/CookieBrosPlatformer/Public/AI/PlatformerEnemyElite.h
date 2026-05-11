#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyBase.h"
#include "AI/PlatformerEnemyCombatProfile.h"
#include "PlatformerEnemyElite.generated.h"

class AEnemyProjectile;

/**
 * APlatformerEnemyElite
 * Reusable strengthened non-boss enemy shell with configurable combat capability.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerEnemyElite : public APlatformerEnemyBase
{
	GENERATED_BODY()

	enum class EResolvedEliteAttackMode : uint8
	{
		None,
		Melee,
		Ranged
	};

public:
	APlatformerEnemyElite(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category="Combat|Elite")
	EPlatformerEnemyCombatProfile GetEliteCombatProfile() const { return CombatProfile; }

	UFUNCTION(BlueprintCallable, Category="Combat|Elite")
	void SetEliteCombatProfile(EPlatformerEnemyCombatProfile InCombatProfile);

	UFUNCTION(BlueprintPure, Category="Combat|Ranged")
	float GetEnemyProjectileSpeed() const { return ProjectileSpeed; }

	UFUNCTION(BlueprintCallable, Category="Combat|Ranged")
	void SetEnemyProjectileSpeed(float InProjectileSpeed);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Elite")
	EPlatformerEnemyCombatProfile CombatProfile = EPlatformerEnemyCombatProfile::Hybrid;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Melee", meta=(ClampMin=0.0, Units="cm"))
	float MeleeTraceDistance = 140.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Melee", meta=(ClampMin=0.0, Units="cm"))
	float MeleeTraceRadius = 45.0f;

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

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual float GetAttackRange() const override;
	virtual float GetAttackCooldown() const override;
	virtual float GetAttackDamageAmount() const override;
	virtual bool CanAttackTarget(const APlatformerCombatCharacterBase* TargetActor) const override;
	virtual bool ApplyAttackHit(APlatformerCombatCharacterBase* TargetActor) override;
	virtual FGameplayTag GetAttackAnimationTagForTarget(const APlatformerCombatCharacterBase* TargetActor) const override;
	virtual bool IsAttackAnimationPlaying(const APlatformerCombatCharacterBase* TargetActor) const override;
	virtual void ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype) override;

private:
	void ApplyEliteCollisionDefaults();
	void ApplyEliteMeshDefaults();
	bool HasMeleeCapability() const;
	bool HasRangedCapability() const;
	float GetMeleeAttackRange() const;
	float GetRangedAttackRange() const;
	float GetAttackRangeForMode(EResolvedEliteAttackMode AttackMode) const;
	float GetAttackCooldownForMode(EResolvedEliteAttackMode AttackMode) const;
	float GetAttackDamageAmountForMode(EResolvedEliteAttackMode AttackMode) const;
	EResolvedEliteAttackMode ResolveAttackModeForTarget(const APlatformerCombatCharacterBase* TargetActor) const;
	bool PerformMeleeAttack(APlatformerCombatCharacterBase* TargetActor);
	bool PerformRangedAttack(APlatformerCombatCharacterBase* TargetActor);
};
