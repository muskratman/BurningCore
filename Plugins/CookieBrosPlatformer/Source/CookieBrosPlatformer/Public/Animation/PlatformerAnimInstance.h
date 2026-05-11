#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "PlatformerAnimInstance.generated.h"

class APlatformerCharacterBase;
class UAbilitySystemComponent;
class UAnimMontage;
class UAnimSequence;
class UBlendSpace;
class UAimOffsetBlendSpace;
class UPlatformerAnimDataAsset;
class UPlatformerLocomotionAnimSet;
class UPlatformerTraversalMovementComponent;

/**
 * UPlatformerAnimInstance
 * Base C++ AnimInstance for platformer characters.
 *
 * Automatically reads movement, traversal, and combat state from
 * the owning character and its GAS AbilitySystemComponent every frame.
 *
 * Provides data-driven montage playback API used by Gameplay Abilities
 * to resolve animations through UPlatformerAnimDataAsset.
 *
 * Projects create Blueprint AnimBPs with this as the parent class.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// === Data-Driven Montage API ===

	/**
	 * Play an ability montage resolved from AnimData by GameplayTag.
	 * @param AnimTag - Tag to look up in AnimDataAsset
	 * @param PlayRate - Playback speed multiplier
	 * @return Duration of the montage, or 0 if not found/played
	 */
	UFUNCTION(BlueprintCallable, Category="Platformer|Animation")
	float PlayAbilityMontage(FGameplayTag AnimTag, float PlayRate = 1.0f);

	/**
	 * Stop an ability montage resolved from AnimData by GameplayTag.
	 * @param AnimTag - Tag to look up in AnimDataAsset
	 * @param BlendOutTime - Blend-out duration
	 */
	UFUNCTION(BlueprintCallable, Category="Platformer|Animation")
	void StopAbilityMontage(FGameplayTag AnimTag, float BlendOutTime = 0.25f);

	/**
	 * Resolve a montage from AnimData by GameplayTag without playing it.
	 * @return The montage, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category="Platformer|Animation")
	UAnimMontage* ResolveAbilityMontage(const FGameplayTag& AnimTag) const;

protected:
	/** Update movement-related properties from the owning character. */
	virtual void UpdateMovementProperties(float DeltaSeconds);

	/** Update gameplay tag-driven state properties from the ASC. */
	virtual void UpdateGameplayTagProperties();

	/** Update derived transition helpers after raw movement/tag values are refreshed. */
	virtual void UpdateDerivedStateProperties();

	/** Update dash phase helpers from traversal runtime data. */
	virtual void UpdateDashStateProperties(float DeltaSeconds);

	/** Update ladder phase helpers from the owning character runtime data. */
	virtual void UpdateLadderStateProperties();

	/** Update ledge grab phase helpers from traversal runtime data. */
	virtual void UpdateLedgeGrabStateProperties(float DeltaSeconds);

	/**
	 * Update landing state. Must be called after UpdateLadderStateProperties
	 * and UpdateLedgeGrabStateProperties so their end-states can suppress a
	 * spurious land trigger.
	 */
	virtual void UpdateLandingStateProperties(float DeltaSeconds);

	/** Update LookYaw / LookPitch from the owning character's control rotation. */
	virtual void UpdateLookProperties();

	/** Update montage-driven ability state helpers from AnimData mappings. */
	virtual void UpdateAbilityMontageProperties();

	// === Movement State ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	float VerticalVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	bool bIsOnLadder = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	float LadderClimbInput = 0.0f;

	/** Actual ladder climb speed: Abs(Velocity.Z) while on ladder, 0 otherwise.
	 *  Use this as the BlendSpace axis for ladder loop, not LadderClimbInput,
	 *  so the animation freezes when the character stops despite held input. */
	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	float LadderSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	float MovementDirectionX = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement")
	bool bHasAcceleration = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Movement", meta=(ClampMin="0.0", Units="cm/s"))
	float MovementStateSpeedThreshold = 3.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldIdle = true;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldJump = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldFall = false;

	/** True for LandStateDuration seconds after touching ground from air. Only
	 *  active when LocomotionAnimSet provides a LandSequence. */
	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldLand = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Movement",
		meta=(ClampMin="0.0", Units="s"))
	float LandStateDuration = 0.15f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldCrouchIdle = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Movement|Derived")
	bool bShouldCrouchMove = false;

	// === Look / AimOffset ===

	/** Delta yaw between ControlRotation and ActorRotation, clamped to -90..90. */
	UPROPERTY(BlueprintReadOnly, Category="Platformer|Look")
	float LookYaw = 0.0f;

	/** Delta pitch from ControlRotation, clamped to -90..90. */
	UPROPERTY(BlueprintReadOnly, Category="Platformer|Look")
	float LookPitch = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Traversal|Dash", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DashStartProgressThreshold = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Traversal|Dash", meta=(ClampMin="0.0", Units="s"))
	float DashEndStateDuration = 0.12f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Dash")
	float DashElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Dash")
	float DashExpectedDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Dash")
	float DashTimeAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Dash")
	float DashDistanceAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Dash")
	float DashProgressAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldDashStart = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldDashLoop = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldDashEnd = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLadderStart = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLadderLoop = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLadderEnd = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Traversal|Ledge", meta=(ClampMin="0.0", Units="s"))
	float LedgeGrabStartStateDuration = 0.16f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Traversal|Ledge", meta=(ClampMin="0.0", Units="s"))
	float LedgeGrabEndStateDuration = 0.12f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Ledge")
	float LedgeGrabEndElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Ledge")
	float LedgeGrabEndExpectedDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Ledge")
	float LedgeGrabEndTimeAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLedgeGrabStart = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLedgeGrabLoop = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldLedgeGrabEnd = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldWallSlide = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal|Derived")
	bool bShouldWallJump = false;

	// === Traversal State (from Gameplay Tags) ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal")
	bool bIsDashing = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal")
	bool bIsWallSliding = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal")
	bool bIsWallJumping = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal")
	bool bIsLedgeHanging = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Traversal")
	bool bIsLedgeClimbing = false;

	// === Combat State (from Gameplay Tags) ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Combat")
	bool bIsCharging = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Combat")
	bool bIsChargePartial = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Combat")
	bool bIsChargeFull = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Combat")
	bool bIsDead = false;

	// === Ability Animation State (from AnimData montage playback) ===

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsMeleeAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsRangedAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsMeleeCharging = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsRangedCharging = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsDeathAnimating = false;

	UPROPERTY(BlueprintReadOnly, Category="Platformer|Ability Animation")
	bool bIsPlayingAbilityMontage = false;

	// === Animation Data ===

	/** GameplayTag → AnimMontage mapping for ability-driven animations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Platformer|Data")
	TObjectPtr<UPlatformerAnimDataAsset> AnimData;

	/** Locomotion sequences and blendspaces; cached from the owning character. */
	UPROPERTY(BlueprintReadOnly, Category="Platformer|Data")
	TObjectPtr<UPlatformerLocomotionAnimSet> LocomotionAnimSet;

	// === Cached Locomotion Sequences (populated from LocomotionAnimSet at init) ===
	// Use these directly in AnimBP state machine nodes — no Blueprint boilerplate needed.

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UBlendSpace> CachedLocomotionBS;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UBlendSpace> CachedCrouchBS;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedJumpStartSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedFallLoopSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLandSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedDashStartSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedDashLoopSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedDashEndSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedWallSlideSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedWallJumpSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLadderStartSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UBlendSpace> CachedLadderLoopBS;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLadderEndSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLedgeGrabStartSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLedgeGrabLoopSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAnimSequence> CachedLedgeClimbSequence;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	TObjectPtr<UAimOffsetBlendSpace> CachedLookAimOffset;

	// === Locomotion Feature Toggles (true when the corresponding sequence group is set) ===

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasDash = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasWall = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasLadder = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasLedgeGrab = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasLand = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Platformer|Locomotion")
	bool bHasLook = false;

private:
	/** Initialize AnimData from the owning character's AnimDataAsset, if available. */
	void CacheAnimDataFromCharacter();
	bool IsAbilityMontagePlaying(const FGameplayTag& AnimTag) const;
	bool IsAnyAbilityMontagePlaying() const;

	UPROPERTY(Transient)
	TObjectPtr<APlatformerCharacterBase> CachedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY(Transient)
	TObjectPtr<UPlatformerTraversalMovementComponent> CachedTraversalMovementComponent;

	bool bCachedDataInitialized = false;
	bool bWasDashingLastFrame = false;
	bool bWasLedgeHangingLastFrame = false;
	bool bWasLedgeClimbingLastFrame = false;
	bool bWasInAirLastFrame = false;
	float DashEndStateTimeRemaining = 0.0f;
	float LedgeGrabStartStateTimeRemaining = 0.0f;
	float LedgeGrabEndStateTimeRemaining = 0.0f;
	float LandStateTimeRemaining = 0.0f;
	float PreviousFrameVerticalVelocity = 0.0f;
};
