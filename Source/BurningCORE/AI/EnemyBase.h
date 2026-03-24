#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/Damageable.h"
#include "EnemyBase.generated.h"

class UAbilitySystemComponent;
class UEnemyAttributeSet;
class UStateTreeComponent;
class UAIPerceptionComponent;
class UEnemyArchetypeAsset;

/**
 * AEnemyBase
 * Foundational class for all enemies with integrated GAS and StateTree.
 */
UCLASS(abstract)
class BURNINGCORE_API AEnemyBase : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UEnemyAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

public:
	AEnemyBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) override;
	virtual bool IsAlive() const override;

	void InitializeFromArchetype(const UEnemyArchetypeAsset* Archetype);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Archetype")
	TObjectPtr<UEnemyArchetypeAsset> DefaultArchetype;

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void HandleDeath();

	float StaggerThreshold = 2.0f;
};
