#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DragonDashBounceSurfaceComponent.generated.h"

/**
 * Marks a DragonSlayer-specific platforming surface as dash-bounceable.
 * Useful for jump pads, springs, and air-only section reset objects.
 */
UCLASS(ClassGroup=(DragonSlayer), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class DRAGONSLAYER_API UDragonDashBounceSurfaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool AllowsDragonDashBounce() const { return bAllowsDragonDashBounce; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool ShouldResetDragonAirDash() const { return bResetAirDash; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool TryGetBounceDirection(FVector& OutDirection) const;

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetSpeedMultiplier() const { return FMath::Max(SpeedMultiplier, 0.0f); }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetDistanceMultiplier() const { return FMath::Max(DistanceMultiplier, 0.0f); }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bAllowsDragonDashBounce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bResetAirDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bOverrideBounceDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true", EditCondition="bOverrideBounceDirection"))
	FVector BounceDirection = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float DistanceMultiplier = 1.0f;
};
