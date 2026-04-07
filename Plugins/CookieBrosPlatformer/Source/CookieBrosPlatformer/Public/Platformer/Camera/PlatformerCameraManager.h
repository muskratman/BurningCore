// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "PlatformerCameraManager.generated.h"

class APawn;

/**
 *  Simple platformer camera with smooth scrolling and horizontal bounds
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	/** Overrides the default camera view target calculation */
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	virtual void ApplyDeveloperCameraManagerSettings(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings);
	virtual FDeveloperPlatformerCameraManagerSettings CaptureDeveloperCameraManagerSettings() const;

protected:

	/** Resolves the baseline camera pose from the pawn's actual camera setup in the Blueprint hierarchy. */
	void ResolveBaseCameraPose(const APawn& TargetPawn, FVector& OutLocation, FRotator& OutRotation, float& OutFOV) const;

	/** Converts current movement into a normalized look-ahead direction in the visible platforming plane. */
	FVector2D ResolveMovementDirection2D(const FVector& Velocity) const;
	void LoadAndApplyDeveloperCameraManagerSettings();

public:

	/** How close we want to stay to the view target */
	UPROPERTY(EditAnywhere, Category="Platformer Camera", meta=(ClampMin=0, ClampMax=10000, Units="cm"))
	float CurrentZoom = 1000.0f;

	/** How far above the target do we want the camera to focus */
	UPROPERTY(EditAnywhere, Category="Platformer Camera", meta=(ClampMin=0, ClampMax=10000, Units="cm"))
	float CameraZOffset = 100.0f;

	/** Default rotation used only if the pawn has no camera-related components. */
	UPROPERTY(EditAnywhere, Category="Platformer Camera", meta=(ClampMin=-180.0f, ClampMax=180.0f))
	FRotator FallbackRotation = FRotator(0.0f, -90.0f, 0.0f);

	/** Minimum camera scrolling bounds in world space */
	UPROPERTY(EditAnywhere, Category="Platformer Camera", meta=(ClampMin=-100000, ClampMax=100000, Units="cm"))
	float CameraXMinBounds = -400.0f;

	/** Maximum camera scrolling bounds in world space */
	UPROPERTY(EditAnywhere, Category="Platformer Camera", meta=(ClampMin=-100000, ClampMax=100000, Units="cm"))
	float CameraXMaxBounds = 10000.0f;

	/** Movement speeds below this threshold are treated as idle to avoid tiny physics jitter offsets. */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=500.0f, Units="cm/s"))
	float IdleSpeedThreshold = 5.0f;

	/** How much the camera is offset from the character based on movement direction (look-ahead) */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=1000.0f))
	float HorizontalOffset = 500.0f;

	/** How fast the camera shifts horizontally when horizontal movement begins (idle -> moving) */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=100.0f))
	float HorizontalOffsetInterpSpeedStart = 6.0f;

	/** How fast the camera shifts horizontally back to idle when horizontal movement ends (moving -> idle) */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=100.0f))
	float HorizontalOffsetInterpSpeedEnd = 2.0f;

	/** How much the camera is offset vertically in the movement direction (jump / fall look-ahead) */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=1000.0f))
	float VerticalOffset = 350.0f;

	/** How fast the camera shifts vertically when the character starts or stops moving up/down quickly */
	UPROPERTY(EditAnywhere, Category="Platformer Camera|Smoothing", meta=(ClampMin=0.0f, ClampMax=100.0f))
	float VerticalOffsetInterpSpeed = 2.0f;

protected:

	/** Current interpolated horizontal offset for look-ahead functionality */
	float CurrentHorizontalOffset = 0.0f;

	/** Current interpolated vertical offset for look-ahead functionality */
	float CurrentVerticalOffset = 0.0f;
};
