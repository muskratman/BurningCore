#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "DragonDashEnemyTargetComponent.generated.h"

/**
 * Project-specific marker for enemies that customize Dragon Dash contact rules.
 */
UCLASS(ClassGroup=(DragonSlayer), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class DRAGONSLAYER_API UDragonDashEnemyTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool AllowsDashBounce() const { return bAllowsDashBounce; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool HasAntiDash() const { return bAntiDash; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool ShouldResetAirDash() const { return bResetAirDash; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool ShouldTriggerHitStop() const { return bTriggerHitStop; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetDamageMultiplier() const { return FMath::Max(DamageMultiplier, 0.0f); }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool TryGetBounceDirection(FVector& OutDirection) const;

	UFUNCTION(BlueprintImplementableEvent, Category="Dragon Dash", meta=(DisplayName="On Dragon Dash Hit"))
	void BP_OnDragonDashHit(AActor* DragonActor, const FHitResult& HitResult);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bAllowsDashBounce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bAntiDash = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bResetAirDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bTriggerHitStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true"))
	bool bOverrideBounceDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash", meta=(AllowPrivateAccess="true", EditCondition="bOverrideBounceDirection"))
	FVector BounceDirection = FVector::UpVector;
};
