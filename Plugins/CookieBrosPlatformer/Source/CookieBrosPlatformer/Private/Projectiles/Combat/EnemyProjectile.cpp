#include "Projectiles/Combat/EnemyProjectile.h"

AEnemyProjectile::AEnemyProjectile()
{
	Speed = 1500.0f;
	Lifetime = 5.0f;
	bPiercing = false;
}

void AEnemyProjectile::InitializeProjectile(
	float InSpeed,
	float InLifetime,
	const FGameplayEffectSpecHandle& InDamageEffectSpec,
	const FGameplayEffectSpecHandle& InStatusEffectSpec)
{
	Speed = FMath::Max(0.0f, InSpeed);
	Lifetime = FMath::Max(0.0f, InLifetime);
	DamageEffectSpec = InDamageEffectSpec;
	StatusEffectSpec = InStatusEffectSpec;

	ApplyDeveloperProjectileSpeed(Speed);
	SetLifeSpan(Lifetime);
}
