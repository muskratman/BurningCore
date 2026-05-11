#include "AI/PlatformerEnemyElite.h"

#include "Animation/PlatformerEnemyAnimGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/PlatformerEnemyArchetypeAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Projectiles/Combat/EnemyProjectile.h"

namespace
{
	const float EliteCapsuleHalfHeight = 116.0f;
	const float EliteCapsuleRadius = 40.0f;
	const FVector EliteMeshRelativeLocation(0.0f, 0.0f, -115.0f);
	const FVector EliteMeshScale(1.3f, 1.3f, 1.3f);
}

APlatformerEnemyElite::APlatformerEnemyElite(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileClass = AEnemyProjectile::StaticClass();
	ProjectileMaxDistance = FMath::Max(ProjectileMaxDistance, RangedAttackRange);
	ApplyEliteCollisionDefaults();
	ApplyEliteMeshDefaults();
	UpdateHealthWidgetPlacement();
}

void APlatformerEnemyElite::SetEliteCombatProfile(EPlatformerEnemyCombatProfile InCombatProfile)
{
	CombatProfile = InCombatProfile;
}

void APlatformerEnemyElite::SetEnemyProjectileSpeed(float InProjectileSpeed)
{
	ProjectileSpeed = FMath::Max(InProjectileSpeed, 0.0f);
}

void APlatformerEnemyElite::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyEliteCollisionDefaults();
	ApplyEliteMeshDefaults();
	UpdateHealthWidgetPlacement();
}

float APlatformerEnemyElite::GetAttackRange() const
{
	if (HasMeleeCapability() && HasRangedCapability())
	{
		return FMath::Max(GetMeleeAttackRange(), GetRangedAttackRange());
	}

	if (HasRangedCapability())
	{
		return GetRangedAttackRange();
	}

	return GetMeleeAttackRange();
}

float APlatformerEnemyElite::GetAttackCooldown() const
{
	if (HasRangedCapability() && !HasMeleeCapability())
	{
		return GetAttackCooldownForMode(EResolvedEliteAttackMode::Ranged);
	}

	return GetAttackCooldownForMode(EResolvedEliteAttackMode::Melee);
}

float APlatformerEnemyElite::GetAttackDamageAmount() const
{
	if (HasRangedCapability() && !HasMeleeCapability())
	{
		return GetAttackDamageAmountForMode(EResolvedEliteAttackMode::Ranged);
	}

	return GetAttackDamageAmountForMode(EResolvedEliteAttackMode::Melee);
}

bool APlatformerEnemyElite::CanAttackTarget(const APlatformerCombatCharacterBase* TargetActor) const
{
	if (!TargetActor || !TargetActor->IsAlive() || !IsAlive())
	{
		return false;
	}

	if (IsEnemyAttackInProgress() || IsAttackAnimationPlaying(TargetActor))
	{
		return false;
	}

	const EResolvedEliteAttackMode AttackMode = ResolveAttackModeForTarget(TargetActor);
	if (AttackMode == EResolvedEliteAttackMode::None)
	{
		return false;
	}

	if (GetCombatDistanceToTarget(TargetActor) > GetAttackRangeForMode(AttackMode))
	{
		return false;
	}

	if (const UWorld* World = GetWorld())
	{
		const float AttackCooldown = GetAttackCooldownForMode(AttackMode);
		if (AttackCooldown > 0.0f && World->GetTimeSeconds() - LastAttackWorldTime < AttackCooldown)
		{
			return false;
		}
	}

	return true;
}

bool APlatformerEnemyElite::ApplyAttackHit(APlatformerCombatCharacterBase* TargetActor)
{
	const FGameplayTag PendingAttackTag = GetPendingAttackAnimationTag();
	if (PendingAttackTag == PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_MeleeAttack)
	{
		return PerformMeleeAttack(TargetActor);
	}

	if (PendingAttackTag == PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_RangedAttack)
	{
		return PerformRangedAttack(TargetActor);
	}

	switch (ResolveAttackModeForTarget(TargetActor))
	{
	case EResolvedEliteAttackMode::Melee:
		return PerformMeleeAttack(TargetActor);
	case EResolvedEliteAttackMode::Ranged:
		return PerformRangedAttack(TargetActor);
	default:
		return false;
	}
}

FGameplayTag APlatformerEnemyElite::GetAttackAnimationTagForTarget(const APlatformerCombatCharacterBase* TargetActor) const
{
	switch (ResolveAttackModeForTarget(TargetActor))
	{
	case EResolvedEliteAttackMode::Melee:
		return PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_MeleeAttack;
	case EResolvedEliteAttackMode::Ranged:
		return PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_RangedAttack;
	default:
		return FGameplayTag();
	}
}

bool APlatformerEnemyElite::IsAttackAnimationPlaying(const APlatformerCombatCharacterBase* TargetActor) const
{
	return IsAttackAnimationTagPlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_MeleeAttack)
		|| IsAttackAnimationTagPlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_RangedAttack);
}

void APlatformerEnemyElite::ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype)
{
	if (!Archetype)
	{
		return;
	}

	if (Archetype->ProjectileClass)
	{
		ProjectileClass = Archetype->ProjectileClass;
	}

	const APlatformerEnemyElite* ClassDefaults = GetClass()->GetDefaultObject<APlatformerEnemyElite>();
	if (!ClassDefaults || FMath::IsNearlyEqual(ProjectileSpeed, ClassDefaults->ProjectileSpeed))
	{
		ProjectileSpeed = FMath::Max(0.0f, Archetype->ProjectileSpeed);
	}

	ProjectileLifetime = FMath::Max(0.0f, Archetype->ProjectileLifetime);
	if (Archetype->ProjectileMaxDistance > 0.0f
		&& (!ClassDefaults || FMath::IsNearlyEqual(GetEnemyProjectileDistance(), ClassDefaults->GetEnemyProjectileDistance())))
	{
		SetEnemyProjectileDistance(Archetype->ProjectileMaxDistance);
	}

	if (Archetype->CombatAttackRange > 0.0f && HasRangedCapability())
	{
		RangedAttackRange = Archetype->CombatAttackRange;
	}

	SetEnemyProjectileDistance(FMath::Max(GetEnemyProjectileDistance(), GetRangedAttackRange()));
}

void APlatformerEnemyElite::ApplyEliteCollisionDefaults()
{
	if (UCapsuleComponent* CapsuleComponent = GetCapsuleComponent())
	{
		CapsuleComponent->SetCapsuleSize(EliteCapsuleRadius, EliteCapsuleHalfHeight);
	}
}

void APlatformerEnemyElite::ApplyEliteMeshDefaults()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRelativeLocation(EliteMeshRelativeLocation);
		MeshComponent->SetRelativeScale3D(EliteMeshScale);
	}
}

bool APlatformerEnemyElite::HasMeleeCapability() const
{
	return CombatProfile == EPlatformerEnemyCombatProfile::Melee
		|| CombatProfile == EPlatformerEnemyCombatProfile::Hybrid;
}

bool APlatformerEnemyElite::HasRangedCapability() const
{
	return CombatProfile == EPlatformerEnemyCombatProfile::Ranged
		|| CombatProfile == EPlatformerEnemyCombatProfile::Hybrid;
}

float APlatformerEnemyElite::GetMeleeAttackRange() const
{
	return FMath::Max(Super::GetAttackRange(), MeleeTraceDistance);
}

float APlatformerEnemyElite::GetRangedAttackRange() const
{
	return FMath::Max(Super::GetAttackRange(), RangedAttackRange);
}

float APlatformerEnemyElite::GetAttackRangeForMode(EResolvedEliteAttackMode AttackMode) const
{
	switch (AttackMode)
	{
	case EResolvedEliteAttackMode::Melee:
		return GetMeleeAttackRange();
	case EResolvedEliteAttackMode::Ranged:
		return GetRangedAttackRange();
	default:
		return 0.0f;
	}
}

float APlatformerEnemyElite::GetAttackCooldownForMode(EResolvedEliteAttackMode AttackMode) const
{
	if (!AttributeSet)
	{
		return Super::GetAttackCooldown();
	}

	if (AttackMode == EResolvedEliteAttackMode::Melee && AttributeSet->GetMeleeAttackDelay() > 0.0f)
	{
		return AttributeSet->GetMeleeAttackDelay();
	}

	if (AttackMode == EResolvedEliteAttackMode::Ranged && AttributeSet->GetRangeAttackDelay() > 0.0f)
	{
		return AttributeSet->GetRangeAttackDelay();
	}

	return Super::GetAttackCooldown();
}

float APlatformerEnemyElite::GetAttackDamageAmountForMode(EResolvedEliteAttackMode AttackMode) const
{
	if (!AttributeSet)
	{
		return Super::GetAttackDamageAmount();
	}

	if (AttackMode == EResolvedEliteAttackMode::Melee)
	{
		return FMath::Max(AttributeSet->GetMeleeAttackDamage(), AttributeSet->GetBaseDamage());
	}

	if (AttackMode == EResolvedEliteAttackMode::Ranged)
	{
		return FMath::Max(AttributeSet->GetRangeBaseAttackDamage(), AttributeSet->GetBaseDamage());
	}

	return Super::GetAttackDamageAmount();
}

APlatformerEnemyElite::EResolvedEliteAttackMode APlatformerEnemyElite::ResolveAttackModeForTarget(const APlatformerCombatCharacterBase* TargetActor) const
{
	if (!TargetActor)
	{
		return EResolvedEliteAttackMode::None;
	}

	if (CombatProfile == EPlatformerEnemyCombatProfile::Melee)
	{
		return EResolvedEliteAttackMode::Melee;
	}

	if (CombatProfile == EPlatformerEnemyCombatProfile::Ranged)
	{
		return EResolvedEliteAttackMode::Ranged;
	}

	return GetCombatDistanceToTarget(TargetActor) <= GetMeleeAttackRange()
		? EResolvedEliteAttackMode::Melee
		: EResolvedEliteAttackMode::Ranged;
}

bool APlatformerEnemyElite::PerformMeleeAttack(APlatformerCombatCharacterBase* TargetActor)
{
	if (!TargetActor || !GetWorld())
	{
		return false;
	}

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.5f : 50.0f);

	FVector AttackDirection = TargetActor->GetActorLocation() - Start;
	AttackDirection.Y = 0.0f;
	if (AttackDirection.IsNearlyZero())
	{
		return false;
	}

	AttackDirection.Normalize();
	const FVector End = Start + AttackDirection * MeleeTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlatformerEnemyEliteMeleeAttack), false, this);
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	const bool bHasHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(MeleeTraceRadius),
		QueryParams);

	if (bHasHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			if (HitResult.GetActor() == TargetActor)
			{
				return ApplyCombatDamageToActor(TargetActor, GetAttackDamageAmountForMode(EResolvedEliteAttackMode::Melee), HitResult, AttackDamageEffectClass);
			}
		}
	}

	FHitResult FallbackHit;
	FallbackHit.bBlockingHit = true;
	FallbackHit.ImpactPoint = TargetActor->GetActorLocation();
	FallbackHit.Location = TargetActor->GetActorLocation();
	return ApplyCombatDamageToActor(TargetActor, GetAttackDamageAmountForMode(EResolvedEliteAttackMode::Melee), FallbackHit, AttackDamageEffectClass);
}

bool APlatformerEnemyElite::PerformRangedAttack(APlatformerCombatCharacterBase* TargetActor)
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
		MakeCombatDamageEffectSpec(GetAttackDamageAmountForMode(EResolvedEliteAttackMode::Ranged), DamageHitResult, AttackDamageEffectClass);
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
