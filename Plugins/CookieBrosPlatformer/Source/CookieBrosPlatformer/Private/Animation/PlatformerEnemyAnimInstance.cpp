#include "Animation/PlatformerEnemyAnimInstance.h"

#include "AI/Data/PlatformerEnemyAnimDataAsset.h"
#include "AI/PlatformerEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "Animation/PlatformerEnemyAnimGameplayTags.h"
#include "GAS/PlatformerGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlatformerEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheEnemyReferences();
}

void UPlatformerEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedEnemy)
	{
		CacheEnemyReferences();
		if (!CachedEnemy)
		{
			return;
		}
	}

	if (!CachedASC)
	{
		CachedASC = CachedEnemy->GetAbilitySystemComponent();
	}

	if (!CachedMovementComponent)
	{
		CachedMovementComponent = CachedEnemy->GetCharacterMovement();
	}

	UpdateMovementProperties(DeltaSeconds);
	UpdateAIProperties();
	UpdateGameplayTagProperties();
	UpdateEnemyMontageProperties();
	UpdateDerivedStateProperties();
}

void UPlatformerEnemyAnimInstance::CacheEnemyReferences()
{
	CachedEnemy = Cast<APlatformerEnemyBase>(TryGetPawnOwner());
	CachedASC = CachedEnemy ? CachedEnemy->GetAbilitySystemComponent() : nullptr;
	CachedMovementComponent = CachedEnemy ? CachedEnemy->GetCharacterMovement() : nullptr;

	if (!bCachedDataInitialized)
	{
		CacheEnemyAnimDataFromEnemy();
	}
}

void UPlatformerEnemyAnimInstance::UpdateMovementProperties(float DeltaSeconds)
{
	if (!CachedEnemy)
	{
		return;
	}

	const FVector Velocity = CachedEnemy->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	AirSpeed = Velocity.Size();
	VerticalVelocity = Velocity.Z;

	if (!FMath::IsNearlyZero(Velocity.X))
	{
		MovementDirectionX = FMath::Sign(Velocity.X);
	}

	if (!FMath::IsNearlyZero(Velocity.Z))
	{
		FlightDirectionZ = FMath::Sign(Velocity.Z);
	}

	if (CachedMovementComponent)
	{
		bIsFlying = CachedMovementComponent->MovementMode == MOVE_Flying;
		bIsInAir = CachedMovementComponent->IsFalling();
		bHasAcceleration = bIsFlying
			? CachedMovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f
			: CachedMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.0f;
	}
	else
	{
		bIsFlying = false;
		bIsInAir = false;
		bHasAcceleration = false;
	}

	bIsGrounded = !bIsInAir && !bIsFlying;
	bIsJumping = bIsInAir && VerticalVelocity > VerticalMovementStateThreshold;
	bIsFalling = bIsInAir && VerticalVelocity <= 0.0f;
	bIsMoving = GroundSpeed > MovementStateSpeedThreshold;
	bIsFlyingMoving = bIsFlying && AirSpeed > MovementStateSpeedThreshold;
}

void UPlatformerEnemyAnimInstance::UpdateAIProperties()
{
	bHasCombatTarget = CachedEnemy && CachedEnemy->HasCombatTarget();
	CombatTargetDistance = 0.0f;
	AttackRange = CachedEnemy ? CachedEnemy->GetEnemyAttackRange() : 0.0f;

	if (bHasCombatTarget)
	{
		const AActor* CombatTarget = CachedEnemy->GetCombatTarget();
		if (CombatTarget)
		{
			const FVector SourceLocation = CachedEnemy->GetActorLocation();
			const FVector TargetLocation = CombatTarget->GetActorLocation();
			const FVector FlattenedSourceLocation(SourceLocation.X, 0.0f, SourceLocation.Z);
			const FVector FlattenedTargetLocation(TargetLocation.X, 0.0f, TargetLocation.Z);
			CombatTargetDistance = FVector::Dist(FlattenedSourceLocation, FlattenedTargetLocation);
		}
	}

	bIsInAttackRange = bHasCombatTarget && CombatTargetDistance <= AttackRange;
	bIsChasing = bHasCombatTarget && GroundSpeed > MovementStateSpeedThreshold;
}

void UPlatformerEnemyAnimInstance::UpdateGameplayTagProperties()
{
	bIsDead = (CachedEnemy && !CachedEnemy->IsAlive())
		|| (CachedASC && CachedASC->HasMatchingGameplayTag(PlatformerGameplayTags::State_Combat_Dead));
}

void UPlatformerEnemyAnimInstance::UpdateEnemyMontageProperties()
{
	bIsMeleeAttacking = IsEnemyMontagePlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_MeleeAttack);
	bIsRangedAttacking = IsEnemyMontagePlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_RangedAttack);
	bIsSpecialAttacking = IsEnemyMontagePlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_SpecialAttack);
	bIsHitReacting = IsEnemyMontagePlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_HitReaction);
	bIsDeathAnimating = IsEnemyMontagePlaying(PlatformerEnemyAnimGameplayTags::Anim_Enemy_Combat_Death);
	bIsPlayingEnemyMontage = IsAnyEnemyMontagePlaying();
}

void UPlatformerEnemyAnimInstance::UpdateDerivedStateProperties()
{
	bShouldAttack = !bIsDead && (bIsMeleeAttacking || bIsRangedAttacking || bIsSpecialAttacking);
	bShouldHitReact = !bIsDead && bIsHitReacting;
	bShouldDie = bIsDead;

	bShouldJump = !bIsDead && !bIsFlying && bIsJumping;
	bShouldFall = !bIsDead && !bIsFlying && bIsFalling;

	const bool bCanUseGroundLocomotion = !bIsDead && !bIsFlying && !bIsInAir && !bShouldAttack && !bShouldHitReact;
	bShouldIdle = bCanUseGroundLocomotion && !bIsMoving;
	bShouldMove = bCanUseGroundLocomotion && bIsMoving;

	const bool bCanUseFlightLocomotion = !bIsDead && bIsFlying && !bShouldHitReact;
	bShouldFlyAttack = bCanUseFlightLocomotion && bShouldAttack;
	bShouldFlyMove = bCanUseFlightLocomotion && !bShouldAttack && bIsFlyingMoving;
	bShouldFlyIdle = bCanUseFlightLocomotion && !bShouldAttack && !bIsFlyingMoving;
}

float UPlatformerEnemyAnimInstance::PlayEnemyMontage(FGameplayTag AnimTag, float PlayRate)
{
	UAnimMontage* Montage = ResolveEnemyMontage(AnimTag);
	if (!Montage)
	{
		return 0.0f;
	}

	if (EnemyAnimData)
	{
		if (const FPlatformerEnemyAnimEntry* Entry = EnemyAnimData->FindAnimEntry(AnimTag))
		{
			if (FMath::IsNearlyEqual(PlayRate, 1.0f))
			{
				PlayRate = Entry->DefaultPlayRate;
			}
		}
	}

	return Montage_Play(Montage, PlayRate);
}

void UPlatformerEnemyAnimInstance::StopEnemyMontage(FGameplayTag AnimTag, float BlendOutTime)
{
	UAnimMontage* Montage = ResolveEnemyMontage(AnimTag);
	if (Montage)
	{
		Montage_Stop(BlendOutTime, Montage);
	}
}

UAnimMontage* UPlatformerEnemyAnimInstance::ResolveEnemyMontage(const FGameplayTag& AnimTag) const
{
	if (!EnemyAnimData)
	{
		return nullptr;
	}

	return EnemyAnimData->FindMontage(AnimTag);
}

void UPlatformerEnemyAnimInstance::CacheEnemyAnimDataFromEnemy()
{
	bCachedDataInitialized = true;

	if (!EnemyAnimData && CachedEnemy)
	{
		EnemyAnimData = CachedEnemy->GetEnemyAnimDataAsset();
	}
}

bool UPlatformerEnemyAnimInstance::IsEnemyMontagePlaying(const FGameplayTag& AnimTag) const
{
	UAnimMontage* Montage = ResolveEnemyMontage(AnimTag);
	return Montage && Montage_IsPlaying(Montage);
}

bool UPlatformerEnemyAnimInstance::IsAnyEnemyMontagePlaying() const
{
	if (!EnemyAnimData)
	{
		return false;
	}

	for (const FPlatformerEnemyAnimEntry& Entry : EnemyAnimData->EnemyAnimations)
	{
		if (Entry.Montage && Montage_IsPlaying(Entry.Montage.Get()))
		{
			return true;
		}
	}

	return false;
}
