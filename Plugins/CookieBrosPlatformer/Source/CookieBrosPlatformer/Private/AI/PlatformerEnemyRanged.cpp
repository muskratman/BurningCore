#include "AI/PlatformerEnemyRanged.h"

#include "Components/SkeletalMeshComponent.h"
#include "Data/PlatformerEnemyArchetypeAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Projectiles/Combat/EnemyProjectile.h"

APlatformerEnemyRanged::APlatformerEnemyRanged(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float APlatformerEnemyRanged::GetAttackRange() const
{
	return FMath::Max(Super::GetAttackRange(), RangedAttackRange);
}

float APlatformerEnemyRanged::GetAttackCooldown() const
{
	if (AttributeSet && AttributeSet->GetRangeAttackDelay() > 0.0f)
	{
		return AttributeSet->GetRangeAttackDelay();
	}

	return Super::GetAttackCooldown();
}

float APlatformerEnemyRanged::GetAttackDamageAmount() const
{
	if (AttributeSet)
	{
		return FMath::Max(AttributeSet->GetRangeBaseAttackDamage(), AttributeSet->GetBaseDamage());
	}

	return Super::GetAttackDamageAmount();
}

bool APlatformerEnemyRanged::PerformAttack(APlatformerCombatCharacterBase* TargetActor)
{
	if (!TargetActor || !ProjectileClass || !GetWorld())
	{
		return false;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();

	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, ProjectileSpawnUpOffset);
	if (USkeletalMeshComponent* MeshComponent = GetMesh();
		MeshComponent && ProjectileSpawnSocketName != NAME_None && MeshComponent->DoesSocketExist(ProjectileSpawnSocketName))
	{
		SpawnLocation = MeshComponent->GetSocketLocation(ProjectileSpawnSocketName);
	}
	else
	{
		FVector ForwardVector = TargetLocation - GetActorLocation();
		ForwardVector.Y = 0.0f;
		if (ForwardVector.IsNearlyZero())
		{
			ForwardVector = GetActorForwardVector();
		}
		else
		{
			ForwardVector.Normalize();
		}

		SpawnLocation += ForwardVector * ProjectileSpawnForwardOffset;
	}

	const FRotator SpawnRotation = (TargetLocation - SpawnLocation).Rotation();

	FHitResult DamageHitResult;
	DamageHitResult.bBlockingHit = true;
	DamageHitResult.ImpactPoint = TargetLocation;
	DamageHitResult.Location = TargetLocation;

	const FGameplayEffectSpecHandle DamageSpec =
		MakeCombatDamageEffectSpec(GetAttackDamageAmount(), DamageHitResult, AttackDamageEffectClass);
	if (!DamageSpec.IsValid())
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyProjectile* Projectile = GetWorld()->SpawnActor<AEnemyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (!Projectile)
	{
		return false;
	}

	Projectile->InitializeProjectile(ProjectileSpeed, ProjectileLifetime, DamageSpec);
	Projectile->ApplyProjectileMaxDistance(GetProjectileMaxDistance());
	return true;
}

void APlatformerEnemyRanged::ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype)
{
	if (!Archetype)
	{
		return;
	}

	if (Archetype->ProjectileClass)
	{
		ProjectileClass = Archetype->ProjectileClass;
	}

	ProjectileSpeed = FMath::Max(0.0f, Archetype->ProjectileSpeed);
	ProjectileLifetime = FMath::Max(0.0f, Archetype->ProjectileLifetime);
	if (Archetype->CombatAttackRange > 0.0f)
	{
		RangedAttackRange = Archetype->CombatAttackRange;
	}
}
