#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DragonOverdriveComponent.generated.h"

class UAbilitySystemComponent;

/**
 * UDragonOverdriveComponent
 * Manages resource filling, draining, and activation threshold for Overdrive mode.
 */
UCLASS(ClassGroup="Dragon", meta=(BlueprintSpawnableComponent))
class BURNINGCORE_API UDragonOverdriveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDragonOverdriveComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Adds energy to the threshold */
	UFUNCTION(BlueprintCallable, Category="Overdrive")
	void AddOverdriveEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category="Overdrive")
	bool TryActivateOverdrive();

	UFUNCTION(BlueprintCallable, Category="Overdrive")
	void DeactivateOverdrive();

	UFUNCTION(BlueprintPure, Category="Overdrive")
	bool IsOverdriveActive() const;

	UFUNCTION(BlueprintPure, Category="Overdrive")
	float GetOverdrivePercent() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverdriveChanged, bool, bActive);
	
	UPROPERTY(BlueprintAssignable, Category="Overdrive")
	FOnOverdriveChanged OnOverdriveStateChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Overdrive", meta=(ClampMin=0, ClampMax=100))
	float ActivationThreshold = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category="Overdrive", meta=(ClampMin=0, ClampMax=50))
	float DrainRate = 10.0f;

	bool bIsActive = false;

	/** Helper to avoid repeated component fetching */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
};
