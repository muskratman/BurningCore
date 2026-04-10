#include "AI/PlatformerEnemyMelee.h"

#include "Components/CapsuleComponent.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"

APlatformerEnemyMelee::APlatformerEnemyMelee(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float APlatformerEnemyMelee::GetAttackRange() const
{
	return FMath::Max(Super::GetAttackRange(), MeleeTraceDistance);
}

float APlatformerEnemyMelee::GetAttackCooldown() const
{
	if (AttributeSet && AttributeSet->GetMeleeAttackDelay() > 0.0f)
	{
		return AttributeSet->GetMeleeAttackDelay();
	}

	return Super::GetAttackCooldown();
}

float APlatformerEnemyMelee::GetAttackDamageAmount() const
{
	if (AttributeSet)
	{
		return FMath::Max(AttributeSet->GetMeleeAttackDamage(), AttributeSet->GetBaseDamage());
	}

	return Super::GetAttackDamageAmount();
}

bool APlatformerEnemyMelee::PerformAttack(APlatformerCombatCharacterBase* TargetActor)
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

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlatformerEnemyMeleeAttack), false, this);
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
				return ApplyCombatDamageToActor(TargetActor, GetAttackDamageAmount(), HitResult, AttackDamageEffectClass);
			}
		}
	}

	FHitResult FallbackHit;
	FallbackHit.bBlockingHit = true;
	FallbackHit.ImpactPoint = TargetActor->GetActorLocation();
	FallbackHit.Location = TargetActor->GetActorLocation();
	return ApplyCombatDamageToActor(TargetActor, GetAttackDamageAmount(), FallbackHit, AttackDamageEffectClass);
}
