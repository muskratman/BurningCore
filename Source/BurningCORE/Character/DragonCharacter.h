#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/Damageable.h"
#include "DragonCharacter.generated.h"

class UAbilitySystemComponent;
class UDragonAttributeSet;
class USideViewMovementComponent;
class UDragonFormComponent;
class UDragonOverdriveComponent;
class UDragonAbilitySet;
class USpringArmComponent;
class UCameraComponent;

/**
 * ADragonCharacter
 * Base class for the primary playable hero.
 */
UCLASS(abstract)
class BURNINGCORE_API ADragonCharacter : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    
	UPROPERTY()
	TObjectPtr<UDragonAttributeSet> AttributeSet;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDragonFormComponent> FormComponent;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDragonOverdriveComponent> OverdriveComponent;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCamera;
    
public:
	ADragonCharacter(const FObjectInitializer& ObjectInitializer);
    
	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
	// IDamageable
	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) override;
	virtual bool IsAlive() const override;
    
	void InitializeAbilities(const UDragonAbilitySet* AbilitySet);
    
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
	UPROPERTY(EditDefaultsOnly, Category="Abilities")
	TObjectPtr<UDragonAbilitySet> DefaultAbilitySet;
    
public:
	FORCEINLINE UDragonFormComponent* GetFormComponent() const { return FormComponent; }
	FORCEINLINE UDragonOverdriveComponent* GetOverdriveComponent() const { return OverdriveComponent; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
