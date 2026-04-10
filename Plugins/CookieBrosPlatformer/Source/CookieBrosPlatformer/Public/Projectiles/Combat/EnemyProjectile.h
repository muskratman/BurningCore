#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Projectiles/Combat/CombatProjectile.h"
#include "EnemyProjectile.generated.h"

/**
 * AEnemyProjectile
 * Base class for all hostile projectiles.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API AEnemyProjectile : public ACombatProjectile
{
	GENERATED_BODY()

public:
	AEnemyProjectile();

	void InitializeProjectile(
		float InSpeed,
		float InLifetime,
		const FGameplayEffectSpecHandle& InDamageEffectSpec,
		const FGameplayEffectSpecHandle& InStatusEffectSpec = FGameplayEffectSpecHandle());
};
