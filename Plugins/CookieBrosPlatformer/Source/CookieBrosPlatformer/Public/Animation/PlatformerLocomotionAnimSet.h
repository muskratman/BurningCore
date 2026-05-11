#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlatformerLocomotionAnimSet.generated.h"

class UBlendSpace;
class UAimOffsetBlendSpace;
class UAnimSequence;

/**
 * UPlatformerLocomotionAnimSet
 * Data-driven locomotion sequences and blendspaces for a platformer character.
 *
 * Assign one instance per skeleton/character. The AnimBP reads sequences and
 * blendspaces from this asset at runtime — no Blueprint rewiring needed when
 * swapping characters.
 *
 * Nullable fields act as feature toggles: if a sequence group is left null
 * the AnimBP state machine skips those states entirely. For example, leaving
 * all Dash sequences null disables dash animation without touching the state
 * machine logic.
 */
UCLASS(BlueprintType)
class COOKIEBROSPLATFORMER_API UPlatformerLocomotionAnimSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// =========================================================================
	// Ground Locomotion
	// =========================================================================

	/** Idle → Walk → Run blend space. X axis = GroundSpeed (cm/s). Required. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground")
	TObjectPtr<UBlendSpace> LocomotionBlendSpace;

	/** CrouchIdle → CrouchWalk blend space. Nullable — falls back to static crouch pose. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ground")
	TObjectPtr<UBlendSpace> CrouchBlendSpace;

	// =========================================================================
	// Air
	// =========================================================================

	/** Played at the start of a jump (rising phase). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Air")
	TObjectPtr<UAnimSequence> JumpStartSequence;

	/** Looped while falling (negative vertical velocity). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Air")
	TObjectPtr<UAnimSequence> FallLoopSequence;

	/**
	 * Short landing recovery pose played after touching ground from air.
	 * Nullable — disables the Land state; character goes straight to Idle/Run.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Air")
	TObjectPtr<UAnimSequence> LandSequence;

	// =========================================================================
	// Traversal | Dash
	// All three nullable. Null = dash animation disabled in the state machine.
	// =========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Dash")
	TObjectPtr<UAnimSequence> DashStartSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Dash")
	TObjectPtr<UAnimSequence> DashLoopSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Dash")
	TObjectPtr<UAnimSequence> DashEndSequence;

	// =========================================================================
	// Traversal | Wall
	// Nullable. Null = wall-slide / wall-jump animation disabled.
	// =========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Wall")
	TObjectPtr<UAnimSequence> WallSlideSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Wall")
	TObjectPtr<UAnimSequence> WallJumpSequence;

	// =========================================================================
	// Traversal | Ladder
	// All three nullable. Null = ladder animation disabled.
	// =========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Ladder")
	TObjectPtr<UAnimSequence> LadderStartSequence;

	/**
	 * Blend space for ladder climbing. X axis = LadderSpeed (actual climb speed,
	 * Abs(Velocity.Z) while on ladder). At X=0 the character holds the grab pose;
	 * at X=MaxClimbSpeed it plays the climb animation. Using actual speed (not raw
	 * input) mirrors the Idle↔Run pattern and ensures the pose freezes correctly
	 * when the character stops at the top/bottom of the ladder despite held input.
	 * Nullable — disables ladder loop animation.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Ladder")
	TObjectPtr<UBlendSpace> LadderLoopBlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|Ladder")
	TObjectPtr<UAnimSequence> LadderEndSequence;

	// =========================================================================
	// Traversal | LedgeGrab
	// All three nullable. Null = ledge-grab animation disabled.
	// =========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|LedgeGrab")
	TObjectPtr<UAnimSequence> LedgeGrabStartSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|LedgeGrab")
	TObjectPtr<UAnimSequence> LedgeGrabLoopSequence;

	/** Played during the climb-over mantle at the end of a ledge grab. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal|LedgeGrab")
	TObjectPtr<UAnimSequence> LedgeClimbSequence;

	// =========================================================================
	// Look / Aim Offset
	// Nullable. Null = no procedural look system applied.
	// =========================================================================

	/**
	 * AimOffset blend space for head/upper-body look.
	 * Expected axes: Yaw (-90..90), Pitch (-90..90).
	 * Driven by LookYaw and LookPitch from UPlatformerAnimInstance.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Look")
	TObjectPtr<UAimOffsetBlendSpace> LookAimOffset;
};
