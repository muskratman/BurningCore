#pragma once

#include "CoreMinimal.h"
#include "Combat/PlatformerCombatCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "TimerManager.h"
#include "PlatformerEnemyBase.generated.h"

class UAIPerceptionComponent;
class UPlatformerEnemyArchetypeAsset;
class UPlatformerEnemyAnimDataAsset;
class UStateTreeComponent;
class UGameplayEffect;
class UAISenseConfig_Damage;
class UAISenseConfig_Sight;
class AEnemyProjectile;

/**
 * APlatformerEnemyBase
 * Generic GAS-aware enemy shell with perception and StateTree hooks.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerEnemyBase : public APlatformerCombatCharacterBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Patrol", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlatformerPathComponent> PatrolPathComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAISenseConfig_Damage> DamageSenseConfig;

	UPROPERTY(EditDefaultsOnly, Category="Archetype")
	TObjectPtr<UPlatformerEnemyArchetypeAsset> DefaultArchetype;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Targeting", meta=(ClampMin=0.0, Units="cm"))
	float CombatEngageRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Targeting", meta=(ClampMin=0.0, Units="cm"))
	float CombatLoseTargetRange = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Attack", meta=(ClampMin=0.0, Units="cm"))
	float CombatAttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Attack")
	TSubclassOf<UGameplayEffect> AttackDamageEffectClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Targeting", meta=(AllowPrivateAccess="true"))
	TObjectPtr<APlatformerCombatCharacterBase> CurrentCombatTarget;

	float StaggerThreshold = 2.0f;
	float LastAttackWorldTime = -BIG_NUMBER;

public:
	APlatformerEnemyBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeFromArchetype(const UPlatformerEnemyArchetypeAsset* Archetype);
	UFUNCTION(BlueprintCallable, Category="AI|Settings")
	virtual void ApplyEnemyRuntimeSettings();

	virtual bool TryAttackCurrentTarget();
	virtual bool TryAttackTarget(APlatformerCombatCharacterBase* TargetActor);
	virtual void SetCombatTarget(APlatformerCombatCharacterBase* NewTarget);

	UFUNCTION(BlueprintPure, Category="AI|Settings")
	float GetEnemyHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category="AI|Settings")
	void SetEnemyHealth(float InHealth);

	UFUNCTION(BlueprintPure, Category="AI|Settings")
	float GetEnemyMovementSpeed() const { return MovementSpeed; }

	UFUNCTION(BlueprintCallable, Category="AI|Settings")
	void SetEnemyMovementSpeed(float InMovementSpeed);

	UFUNCTION(BlueprintPure, Category="AI|Settings")
	float GetEnemyDamage() const { return Damage; }

	UFUNCTION(BlueprintCallable, Category="AI|Settings")
	void SetEnemyDamage(float InDamage);

	UFUNCTION(BlueprintPure, Category="AI|Settings")
	float GetEnemyHitDelay() const { return HitDelay; }

	UFUNCTION(BlueprintCallable, Category="AI|Settings")
	void SetEnemyHitDelay(float InHitDelay);

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	float GetMovementDelayOnHit() const { return MovementDelayOnHit; }

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	void SetMovementDelayOnHit(float InMovementDelayOnHit);

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	float GetOnHitTakenImpulse() const { return OnHitTakenImpulse; }

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	void SetOnHitTakenImpulse(float InOnHitTakenImpulse);

	UFUNCTION(BlueprintPure, Category="AI|Patrol")
	TArray<FPlatformerPathPoint> GetPatrolPoints() const;

	UFUNCTION(BlueprintCallable, Category="AI|Patrol")
	void SetPatrolPoints(const TArray<FPlatformerPathPoint>& InPatrolPoints);

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	bool GetEnablePlayerChase() const { return bEnablePlayerChase; }

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	void SetEnablePlayerChase(bool bInEnable);

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	float GetChaseAgroRadius() const { return ChaseAgroRadius; }

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	void SetChaseAgroRadius(float InRadius);

	UFUNCTION(BlueprintPure, Category="Combat|Projectile")
	float GetEnemyProjectileDistance() const { return GetProjectileMaxDistance(); }

	UFUNCTION(BlueprintCallable, Category="Combat|Projectile")
	void SetEnemyProjectileDistance(float InProjectileDistance);

	UFUNCTION(BlueprintPure, Category="AI|Animation")
	UPlatformerEnemyAnimDataAsset* GetEnemyAnimDataAsset() const { return EnemyAnimDataAsset; }

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	float GetEnemyAttackRange() const { return GetAttackRange(); }

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	float GetEnemyAttackCooldown() const { return GetAttackCooldown(); }

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	bool IsEnemyAttackInProgress() const { return PendingAttackTarget != nullptr && !bPendingAttackHitApplied; }

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	virtual bool ApplyPendingAttackHit();

	FORCEINLINE UStateTreeComponent* GetStateTreeComponent() const { return StateTreeComponent; }
	FORCEINLINE UAIPerceptionComponent* GetPerceptionComponent() const { return PerceptionComponent; }
	FORCEINLINE UPlatformerPathComponent* GetPatrolPathComponent() const { return PatrolPathComponent; }
	FORCEINLINE APlatformerCombatCharacterBase* GetCombatTarget() const { return CurrentCombatTarget; }
	FORCEINLINE bool HasCombatTarget() const { return CurrentCombatTarget && CurrentCombatTarget->IsAlive(); }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnCombatDamageReceived(float DamageAmount, const FHitResult& HitResult, AActor* DamageInstigatorActor) override;
	virtual void OnCombatDeath(AActor* DamageInstigatorActor) override;
	virtual float GetDefaultMaxHealth() const;
	virtual float GetAttackRange() const;
	virtual float GetAttackCooldown() const;
	virtual float GetAttackDamageAmount() const;
	virtual float GetProjectileMaxDistance() const;
	virtual float GetHealthWidgetVerticalPadding() const override;
	virtual bool CanAttackTarget(const APlatformerCombatCharacterBase* TargetActor) const;
	virtual bool StartAttackAnimation(APlatformerCombatCharacterBase* TargetActor);
	virtual bool ApplyAttackHit(APlatformerCombatCharacterBase* TargetActor);
	virtual FGameplayTag GetAttackAnimationTagForTarget(const APlatformerCombatCharacterBase* TargetActor) const;
	virtual float GetAttackAnimationPlayRate(const APlatformerCombatCharacterBase* TargetActor) const;
	virtual float GetAttackHitFallbackDelay(const APlatformerCombatCharacterBase* TargetActor) const;
	virtual void ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype);
	virtual void OnCombatTargetChanged(APlatformerCombatCharacterBase* PreviousTarget, APlatformerCombatCharacterBase* NewTarget);
	float GetCombatDistanceToTarget(const APlatformerCombatCharacterBase* TargetActor) const;
	bool IsMovementDelayedByHit() const;
	void ApplyMovementDelayOnHit();
	void ApplyOnHitTakenImpulse(const FHitResult& HitResult, AActor* DamageInstigatorActor);
	virtual bool IsAttackAnimationPlaying(const APlatformerCombatCharacterBase* TargetActor) const;
	bool IsAttackAnimationTagPlaying(const FGameplayTag& AnimTag) const;
	const FGameplayTag& GetPendingAttackAnimationTag() const { return PendingAttackAnimationTag; }
	void RefreshEnemyCollisionIgnores();
	void IgnoreCollisionWithEnemy(APlatformerEnemyBase* OtherEnemy);
	void UpdatePatrolMovement(float DeltaTime);
	void UpdateChaseMovement(float DeltaTime);
	FVector GetPatrolPointWorldLocation(int32 PatrolPointIndex) const;
	void AdvancePatrolTargetFromReachedPoint(int32 ReachedPatrolPointIndex);
	bool IsPatrolEndpoint(int32 PatrolPointIndex) const;
	void StopPatrolMovement(UCharacterMovementComponent* MovementComponent) const;
	void ApplyFacingForCurrentPatrolSegment();
	void ApplyFacingFromDirection(const FVector& MovementDirection);
	void ApplyDefaultMeshFacing();
	void ClearPendingAttack();
	float PlayEnemyAnimationMontage(const FGameplayTag& AnimTag, float PlayRate = 1.0f);
	bool PlayAttackAnimationMontage(APlatformerCombatCharacterBase* TargetActor);
	void SchedulePendingAttackHitFallback(APlatformerCombatCharacterBase* TargetActor);
	void HandlePendingAttackHitFallback();
	void ScheduleEnemyDeathDestroy(float DelaySeconds);
	void HandleEnemyDeathDestroy();
	const TArray<FPlatformerPathPoint>& GetRuntimePatrolPoints() const;

	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta=(DisplayName="On Combat Target Changed"))
	void BP_OnCombatTargetChanged(AActor* PreviousTarget, AActor* NewTarget);

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true", ClampMin=0.0, Units="cm"))
	float PlatformerHealthWidgetVerticalPadding = 20.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Settings", meta=(ClampMin=1.0))
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Settings", meta=(ClampMin=1.0, Units="cm/s"))
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Settings", meta=(ClampMin=0.0))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Settings", meta=(ClampMin=0.0, Units="s"))
	float HitDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(ClampMin=0.0, Units="s"))
	float MovementDelayOnHit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(ClampMin=0.0, Units="cm/s"))
	float OnHitTakenImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Patrol")
	bool bEnableNativePatrol = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Combat")
	bool bEnablePlayerChase = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Combat", meta=(ClampMin=0.0, Units="cm"))
	float ChaseAgroRadius = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Patrol", meta=(ClampMin=0.0, Units="cm"))
	float PatrolAcceptanceRadius = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Projectile", meta=(ClampMin=0.0, Units="cm"))
	float ProjectileMaxDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Attack", meta=(ClampMin=0.0, Units="s"))
	float AttackHitFallbackDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UPlatformerEnemyAnimDataAsset> EnemyAnimDataAsset;

	UPROPERTY(Transient)
	TObjectPtr<APlatformerCombatCharacterBase> PendingAttackTarget;

	UPROPERTY(Transient)
	FGameplayTag PendingAttackAnimationTag;

	UPROPERTY(Transient)
	float HitMovementDelayEndTime = -BIG_NUMBER;

	UPROPERTY(Transient)
	FVector PatrolOriginLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	int32 CurrentPatrolPointIndex = 0;

	UPROPERTY(Transient)
	int32 PatrolDirection = 1;

	UPROPERTY(Transient)
	float PatrolDelayRemaining = 0.0f;

	UPROPERTY(Transient)
	int32 PatrolSegmentStartPointIndex = INDEX_NONE;

	UPROPERTY(Transient)
	bool bNeedsPatrolSegmentFacingUpdate = true;

	UPROPERTY(Transient)
	bool bPatrolPathCompleted = false;

	FTimerHandle PendingAttackHitFallbackTimerHandle;

	FTimerHandle EnemyDeathDestroyTimerHandle;

	bool bPendingAttackHitApplied = false;

};
