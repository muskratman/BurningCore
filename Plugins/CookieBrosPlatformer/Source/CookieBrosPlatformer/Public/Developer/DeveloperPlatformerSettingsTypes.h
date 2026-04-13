#pragma once

#include "CoreMinimal.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "DeveloperPlatformerSettingsTypes.generated.h"

UENUM(BlueprintType)
enum class EPlatformerCameraProjectionMode : uint8
{
	Perspective = 0,
	Orthographic
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FDeveloperPlatformerCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperSpringArmArmLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FVector DeveloperSpringArmLocation = FVector(0.0f, 0.0f, 70.0f);

	// Stored as Pitch/Yaw/Roll in X/Y/Z to stay compatible with the generic vector widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FVector DeveloperSpringArmRotation = FVector(0.0f, -7.0f, -90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FVector DeveloperCameraLocation = FVector::ZeroVector;

	// Stored as Pitch/Yaw/Roll in X/Y/Z to stay compatible with the generic vector widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FVector DeveloperCameraRotation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FDeveloperPlatformerCharacterMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementMaxWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementMaxFlySpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementMaxAcceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementBrakingDecelerationWalking = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementJumpZVelocity = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementJumpApexGravityMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementJumpHorizontalSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementCrouchCapsuleScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool DeveloperMovementShowJumpTrajectory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementMass = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementBrakingFrictionFactor = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementGroundFriction = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperMovementAirControl = 0.85f;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FDeveloperPlatformerCombatSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatMaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatCurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatMeleeAttackDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatMeleeAttackDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatRangeBaseAttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatRangeBaseAttackSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatRangeChargedAttackDamage = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatRangeChargedAttackSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCombatRangeAttackDelay = 0.0f;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FDeveloperPlatformerCharacterSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCameraSettings DeveloperCameraSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCharacterMovementSettings DeveloperCharacterMovementSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCombatSettings DeveloperCombatSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FPlatformerChargeShotTuning DeveloperChargeShotSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FPlatformerLedgeTraversalSettings DeveloperLedgeSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FPlatformerSlideDashSettings DeveloperSlideDashSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FPlatformerWallTraversalSettings DeveloperWallSettings;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FDeveloperPlatformerCameraManagerSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	EPlatformerCameraProjectionMode DeveloperCameraProjectionMode = EPlatformerCameraProjectionMode::Perspective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerOrthographicWidth = 2048.0f;

	// Stored in m/s for designer-facing editing and converted to the manager's cm/s runtime property.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerIdleSpeedThreshold = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerHorizontalOffset = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerHorizontalOffsetInterpSpeedStart = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerVerticalOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerVerticalOffsetInterpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerDeadZoneWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerDeadZoneHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerBoundBoxWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerBoundBoxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	float DeveloperCameraManagerCrouchInterpSpeed = 8.0f;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerDeveloperSettingsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCharacterSettings CharacterSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCameraManagerSettings CameraManagerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedCameraManagerSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedCombatSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedChargeShotSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedJumpHorizontalSpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedCrouchCapsuleScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedJumpTrajectoryPreview = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedTraversalSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bAutoRestartLevel = false;
};

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerDeveloperSettingsSlotDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FGuid SlotId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDateTime CreatedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDateTime UpdatedAtUtc;
};
