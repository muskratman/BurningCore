#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DragonDashBounceDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FDragonDashBounceTuning
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash", meta=(ClampMin="0.0", Units="cm/s"))
	float DashSpeed = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash", meta=(ClampMin="0.0", Units="cm"))
	float DashDistance = 360.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash", meta=(ClampMin="0.0", Units="s"))
	float DashRecovery = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DirectionInputDeadZone = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DiagonalInputThreshold = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Cancel", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JumpCancelMomentumScale = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Bounce", meta=(ClampMin="0"))
	int32 MaxBouncesPerDash = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Bounce", meta=(ClampMin="0.0", Units="cm"))
	float MinRemainingDistanceAfterBounce = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0", Units="cm"))
	float EnemyHitSweepRadius = 42.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0"))
	float EnemyBounceDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0", Units="s"))
	float AerialDecisionWindowDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0"))
	float AerialDecisionGravityScale = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0", Units="s"))
	float HitStopDuration = 0.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Enemy", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HitStopTimeDilation = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Cues")
	FGameplayTag DashCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Cues")
	FGameplayTag BounceCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Cues")
	FGameplayTag EnemyBounceCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dash|Cues")
	FGameplayTag AntiDashCueTag;
};

/**
 * Project-specific Dragon dash tuning. Forms can later point at variants of
 * this asset without moving the mechanic into the reusable platformer plugin.
 */
UCLASS(BlueprintType)
class DRAGONSLAYER_API UDragonDashBounceDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dragon Dash")
	FDragonDashBounceTuning Tuning;
};
