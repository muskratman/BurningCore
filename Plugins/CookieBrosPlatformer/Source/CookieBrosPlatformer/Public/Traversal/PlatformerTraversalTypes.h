#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PlatformerTraversalTypes.generated.h"

UENUM(BlueprintType)
enum class EPlatformerTraversalState : uint8
{
	None,
	LedgeHang,
	LedgeClimb,
	WallSlide,
	SlideDash
};

UENUM(BlueprintType)
enum class EPlatformerTraversalCustomMode : uint8
{
	None,
	LedgeHang,
	LedgeClimb,
	WallSlide,
	SlideDash
};

UENUM(BlueprintType)
enum class EPlatformerChargeShotStage : uint8
{
	None,
	Partial,
	Full
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerLedgeTraversalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Ledge")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="cm"))
	float DetectionDistance = 72.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="cm"))
	float MaxReachHeight = 140.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="cm"))
	float MinHangHeight = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="cm"))
	float ForwardProbeRadius = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="cm"))
	float TopSurfaceProbeForwardOffset = 52.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(Units="cm"))
	float HangForwardOffset = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(Units="cm"))
	float HangVerticalOffset = -24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="1.0", Units="cm/s"))
	float ClimbSpeed = 240.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="s"))
	float ForgivenessWindow = 0.16f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Ledge", meta=(ClampMin="0.0", Units="s"))
	float RegrabCooldown = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Ledge|Cues")
	FGameplayTag HangCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Ledge|Cues")
	FGameplayTag ClimbCueTag;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerSlideDashSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="SlideDash")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="SlideDash", meta=(ClampMin="0.0", Units="cm/s"))
	float DashSpeed = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="SlideDash", meta=(ClampMin="0.0", Units="cm"))
	float DashDistance = 360.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="SlideDash", meta=(ClampMin="0.01", Units="s"))
	float DashDuration = 0.26f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="SlideDash", meta=(ClampMin="0.0", Units="s"))
	float DashRecovery = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="SlideDash", meta=(ClampMin="0.2", ClampMax="1.0"))
	float DashHitboxScale = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="SlideDash|Cues")
	FGameplayTag DashCueTag;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerWallTraversalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Wall")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="cm"))
	float ProbeDistance = 42.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="cm"))
	float ProbeHeightOffset = 36.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="cm/s"))
	float SlideSpeed = 240.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="cm/s"))
	float WallJumpForce = 980.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", ClampMax="89.0", Units="deg"))
	float WallJumpAngleDegrees = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="s"))
	float ClingTime = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="s"))
	float SameWallReattachCooldown = 0.22f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Wall", meta=(ClampMin="0.0", Units="cm/s"))
	float MinFallSpeedForSlide = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Wall|Cues")
	FGameplayTag WallSlideCueTag;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerChargeShotTuning
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Charge", meta=(ClampMin="0.0", Units="s"))
	float PartialChargeTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Charge", meta=(ClampMin="0.0", Units="s"))
	float FullChargeTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Charge", meta=(ClampMin="1.0"))
	float PartialDamageMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Charge", meta=(ClampMin="0.0"))
	float PartialProjectileSpeedMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, SaveGame, Category="Charge", meta=(ClampMin="0.0"))
	float FullProjectileSpeedMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Charge|Cues")
	FGameplayTag PartialChargeCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Charge|Cues")
	FGameplayTag FullChargeCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame, Category="Charge|Cues")
	FGameplayTag ReleaseCueTag;

	float GetSafePartialChargeTime() const
	{
		return FMath::Max(PartialChargeTime, 0.0f);
	}

	float GetSafeFullChargeTime() const
	{
		return FMath::Max(FullChargeTime, GetSafePartialChargeTime());
	}
};
