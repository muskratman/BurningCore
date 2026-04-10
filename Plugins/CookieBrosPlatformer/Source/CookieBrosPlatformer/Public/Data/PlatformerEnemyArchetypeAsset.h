#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PlatformerEnemyArchetypeAsset.generated.h"

class APlatformerEnemyBase;
class AEnemyProjectile;
class UGameplayAbility;
class UGameplayEffect;
class UStateTree;

/**
 * UPlatformerEnemyArchetypeAsset
 * Generic runtime configuration for reusable platformer enemies.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerEnemyArchetypeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Identity")
	FGameplayTag EnemyTypeTag;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSubclassOf<APlatformerEnemyBase> EnemyClass;

	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float BaseHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float BaseDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float MoveSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UStateTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float CombatEngageRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float CombatLoseTargetRange = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float CombatAttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged")
	float ProjectileSpeed = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Ranged")
	float ProjectileLifetime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
};
