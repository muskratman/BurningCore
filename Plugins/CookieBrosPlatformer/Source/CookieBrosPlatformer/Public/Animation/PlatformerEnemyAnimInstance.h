#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "PlatformerEnemyAnimInstance.generated.h"

class APlatformerEnemyBase;
class UAbilitySystemComponent;
class UAnimMontage;
class UCharacterMovementComponent;
class UPlatformerEnemyAnimDataAsset;

/**
 * Base C++ AnimInstance for reusable platformer enemies.
 *
 * This class is intentionally independent from UPlatformerAnimInstance because
 * enemies usually need compact script-driven locomotion, flight, and
 * attack states instead of the playable character traversal contract.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Platformer|Enemy Animation")
	float PlayEnemyMontage(FGameplayTag AnimTag, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Platformer|Enemy Animation")
	void StopEnemyMontage(FGameplayTag AnimTag, float BlendOutTime = 0.25f);

	UFUNCTION(BlueprintPure, Category="Platformer|Enemy Animation")
	UAnimMontage* ResolveEnemyMontage(const FGameplayTag& AnimTag) const;

protected:
	virtual void CacheEnemyReferences();
	virtual void UpdateMovementProperties(float DeltaSeconds);
	virtual void UpdateAIProperties();
	virtual void UpdateGameplayTagProperties();
	virtual void UpdateEnemyMontageProperties();
	virtual void UpdateDerivedStateProperties();

	// === Movement State ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	float AirSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	float VerticalVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	float MovementDirectionX = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	float FlightDirectionZ = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bHasAcceleration = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bIsGrounded = true;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bIsJumping = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Movement")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Flight")
	bool bIsFlying = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Flight")
	bool bIsFlyingMoving = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Enemy|Movement", meta=(ClampMin="0.0", Units="cm/s"))
	float MovementStateSpeedThreshold = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Enemy|Movement", meta=(ClampMin="0.0", Units="cm/s"))
	float VerticalMovementStateThreshold = 3.0f;

	// === AI State ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|AI")
	bool bHasCombatTarget = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|AI")
	bool bIsChasing = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|AI")
	bool bIsInAttackRange = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|AI")
	float CombatTargetDistance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|AI")
	float AttackRange = 0.0f;

	// === Combat State ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsMeleeAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsRangedAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsSpecialAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsDeathAnimating = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Combat")
	bool bIsPlayingEnemyMontage = false;

	// === Derived Animation Helpers ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldIdle = true;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldJump = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldFall = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldAttack = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldHitReact = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Derived")
	bool bShouldDie = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Flight|Derived")
	bool bShouldFlyIdle = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Flight|Derived")
	bool bShouldFlyMove = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Enemy|Flight|Derived")
	bool bShouldFlyAttack = false;

	// === Animation Data ===

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Enemy|Data")
	TObjectPtr<UPlatformerEnemyAnimDataAsset> EnemyAnimData;

private:
	void CacheEnemyAnimDataFromEnemy();
	bool IsEnemyMontagePlaying(const FGameplayTag& AnimTag) const;
	bool IsAnyEnemyMontagePlaying() const;

	UPROPERTY(Transient)
	TObjectPtr<APlatformerEnemyBase> CachedEnemy;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

	bool bCachedDataInitialized = false;
};
