#pragma once

#include "CoreMinimal.h"
#include "Combat/PlatformerCombatCharacterBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "PlatformerEnemyBase.generated.h"

class UAIPerceptionComponent;
class UPlatformerEnemyArchetypeAsset;
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
	virtual bool TryAttackCurrentTarget();
	virtual bool TryAttackTarget(APlatformerCombatCharacterBase* TargetActor);
	virtual void SetCombatTarget(APlatformerCombatCharacterBase* NewTarget);

	FORCEINLINE UStateTreeComponent* GetStateTreeComponent() const { return StateTreeComponent; }
	FORCEINLINE UAIPerceptionComponent* GetPerceptionComponent() const { return PerceptionComponent; }
	FORCEINLINE APlatformerCombatCharacterBase* GetCombatTarget() const { return CurrentCombatTarget; }
	FORCEINLINE bool HasCombatTarget() const { return CurrentCombatTarget && CurrentCombatTarget->IsAlive(); }

protected:
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
	virtual bool PerformAttack(APlatformerCombatCharacterBase* TargetActor);
	virtual void ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype);
	virtual void OnCombatTargetChanged(APlatformerCombatCharacterBase* PreviousTarget, APlatformerCombatCharacterBase* NewTarget);
	float GetCombatDistanceToTarget(const APlatformerCombatCharacterBase* TargetActor) const;

	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta=(DisplayName="On Combat Target Changed"))
	void BP_OnCombatTargetChanged(AActor* PreviousTarget, AActor* NewTarget);

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true", ClampMin=0.0, Units="cm"))
	float PlatformerHealthWidgetVerticalPadding = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Projectile", meta=(AllowPrivateAccess="true", ClampMin=0.0, Units="cm"))
	float ProjectileMaxDistance = 1500.0f;
};
