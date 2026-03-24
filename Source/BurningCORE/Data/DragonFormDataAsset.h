#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DragonFormDataAsset.generated.h"

class ADragonProjectile;
class UGameplayEffect;
class UMaterialInterface;
class UNiagaraSystem;

/**
 * UDragonFormDataAsset
 * Defines combat behaviors, visuals, and projectiles for a specific form.
 */
UCLASS()
class BURNINGCORE_API UDragonFormDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
	FGameplayTag FormTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<ADragonProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<ADragonProjectile> ChargeProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<UGameplayEffect> OnHitStatusEffect;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	int32 StatusStacksPerHit = 1;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	int32 StatusThreshold = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Overdrive")
	TSubclassOf<ADragonProjectile> OverdriveProjectileClass;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Overdrive")
	float OverdriveDamageMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	float ChargeTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	TObjectPtr<UMaterialInterface> CharacterOverlayMaterial;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	FLinearColor FormColor = FLinearColor::White;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	TObjectPtr<UNiagaraSystem> FormAuraVFX;
};
