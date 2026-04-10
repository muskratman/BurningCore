#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "DragonFormComponent.generated.h"

class UDragonFormDataAsset;

/**
 * UDragonFormComponent
 * Manages the active form and handles form switching logic.
 */
UCLASS(ClassGroup="Dragon", meta=(BlueprintSpawnableComponent))
class DRAGONSLAYER_API UDragonFormComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDragonFormComponent();

	UFUNCTION(BlueprintCallable, Category="Form")
	void SwitchForm(FGameplayTag NewFormTag);

	UFUNCTION(BlueprintPure, Category="Form")
	const UDragonFormDataAsset* GetActiveFormData() const;

	bool TryGetResolvedChargeShotTuning(FPlatformerChargeShotTuning& OutChargeShotTuning) const;
	void SetDeveloperChargeShotTuningOverride(const FPlatformerChargeShotTuning& InChargeShotTuning);
	void ClearDeveloperChargeShotTuningOverride();
	bool HasDeveloperChargeShotTuningOverride() const { return bHasDeveloperChargeShotTuningOverride; }

	UFUNCTION(BlueprintPure, Category="Form")
	FGameplayTag GetActiveFormTag() const { return ActiveFormTag; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFormChanged, FGameplayTag, OldForm, FGameplayTag, NewForm);
	
	UPROPERTY(BlueprintAssignable, Category="Form")
	FOnFormChanged OnFormChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Form")
	TMap<FGameplayTag, TObjectPtr<UDragonFormDataAsset>> FormRegistry;

	UPROPERTY(VisibleAnywhere, Category="Form")
	FGameplayTag ActiveFormTag;

	UPROPERTY(Transient)
	bool bHasDeveloperChargeShotTuningOverride = false;

	UPROPERTY(Transient)
	FPlatformerChargeShotTuning DeveloperChargeShotTuningOverride;
};
