#include "Projectiles/Combat/EnemyProjectile.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AEnemyProjectile::AEnemyProjectile()
{
	Speed = 1500.0f;
	Lifetime = 5.0f;
	bPiercing = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded() && MeshComp)
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
		MeshComp->SetRelativeScale3D(FVector(0.3f));
	}
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
