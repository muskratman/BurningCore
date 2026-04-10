#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/Damageable.h"
#include "PlatformerCombatCharacterBase.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UPlatformerCharacterAttributeSet;
class UPlatformerHealthWidget;
class UWidgetComponent;
struct FOnAttributeChangeData;

/**
 * Shared combat shell for any GAS-driven platformer character.
 * Keeps damage/death rules in one place so player and enemy layers stay thin.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerCombatCharacterBase : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlatformerCharacterAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UPlatformerHealthWidget> HealthWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	FVector2D HealthWidgetDrawSize = FVector2D(120.0f, 16.0f);

	UPROPERTY(EditDefaultsOnly, Category="Combat|Death")
	bool bDisableMovementOnDeath = true;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Death", meta=(ClampMin=0.0, Units="s"))
	float DeathLifeSpan = 0.0f;

public:
	APlatformerCombatCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	static FName GetAttributeSetSubobjectName();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ~begin Damageable interface
	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) override;
	virtual bool IsAlive() const override;
	// ~end Damageable interface

	virtual FGameplayEffectSpecHandle MakeCombatDamageEffectSpec(
		float DamageAmount,
		const FHitResult& HitResult,
		TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr,
		float EffectLevel = 1.0f) const;

	virtual bool ApplyCombatDamageToActor(
		AActor* TargetActor,
		float DamageAmount,
		const FHitResult& HitResult,
		TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr,
		float EffectLevel = 1.0f) const;

	FORCEINLINE UPlatformerCharacterAttributeSet* GetPlatformerAttributeSet() const { return AttributeSet; }
	FORCEINLINE UWidgetComponent* GetHealthWidgetComponent() const { return HealthWidgetComponent; }

protected:
	virtual void BeginPlay() override;

	virtual void OnCombatDamageReceived(float DamageAmount, const FHitResult& HitResult, AActor* DamageInstigatorActor);
	virtual void OnCombatDeath(AActor* DamageInstigatorActor);
	virtual void OnCombatRevived();
	virtual float GetHealthWidgetVerticalPadding() const;
	void RefreshHealthWidget();
	void UpdateHealthWidgetPlacement();
	void SyncCombatLifeStateFromAttributes();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta=(DisplayName="On Combat Damage Received"))
	void BP_OnCombatDamageReceived(float DamageAmount, FVector ImpactPoint, AActor* DamageInstigatorActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta=(DisplayName="On Combat Death"))
	void BP_OnCombatDeath(AActor* DamageInstigatorActor);

private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;

	TWeakObjectPtr<AActor> LastDamageInstigatorActor;

	bool bCombatDeathHandled = false;
};
