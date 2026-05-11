#pragma once
//
#include "Combat/PlatformerCombatCharacterBase.h"
#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "PlatformerCharacterBase.generated.h"


class UPlatformerAbilitySet;
class UPlatformerAnimDataAsset;
class UPlatformerLocomotionAnimSet;
class UGA_PlatformerJump;
class USpringArmComponent;
class UCameraComponent;
class ADeveloperJumpTrajectory;
class APlatformerLadder;
class APlatformerLedgeGrab;
class UDamageType;

/**
 * Discrete states the character can be in with respect to a ladder.
 * Single source of truth — all ladder behavior, animation, and input checks
 * derive from this enum. See APlatformerCharacterBase ladder section.
 */
UENUM(BlueprintType)
enum class EPlatformerLadderState : uint8
{
	None             UMETA(DisplayName = "None"),
	Mounting         UMETA(DisplayName = "Mounting"),
	Climbing         UMETA(DisplayName = "Climbing"),
	JumpBoost        UMETA(DisplayName = "Jump Boost"),
	CrouchBoost      UMETA(DisplayName = "Crouch Boost"),
	TopFinish        UMETA(DisplayName = "Top Finish")
};

/**
 * APlatformerCharacterBase
 * Generic playable platformer shell with camera rig, side-view movement, and
 * combat bootstrap.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerCharacterBase
    : public APlatformerCombatCharacterBase {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<USpringArmComponent> CameraBoom;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UCameraComponent> FollowCamera;

  UPROPERTY(EditDefaultsOnly, Category = "Abilities")
  TObjectPtr<UPlatformerAbilitySet> DefaultAbilitySet;

  UPROPERTY(EditDefaultsOnly, Category = "Developer|Jump Preview")
  TSubclassOf<ADeveloperJumpTrajectory> DeveloperJumpTrajectoryClass;

public:
  APlatformerCharacterBase(const FObjectInitializer &ObjectInitializer);

  virtual void InitializeAbilities(const UPlatformerAbilitySet *AbilitySet);
  virtual void NotifyLadderAvailable(APlatformerLadder *Ladder);
  virtual void NotifyLadderUnavailable(APlatformerLadder *Ladder);
  virtual void NotifyLedgeGrabAvailable(APlatformerLedgeGrab *LedgeGrab);
  virtual void NotifyLedgeGrabUnavailable(APlatformerLedgeGrab *LedgeGrab);
  virtual bool EnterLadder(APlatformerLadder *Ladder);
  virtual void ExitLadder(APlatformerLadder *Ladder = nullptr);
  /** Starts the top-finish phase after reaching the ladder top.
   *  Called when the character reaches the top of a ladder (from TopEntryVolume
   *  overlap or position check). Movement is pinned until the ladder's
   *  TopFinishDuration expires so the end animation has a stable owner state. */
  virtual void FinishLadderClimbAtTop(APlatformerLadder *Ladder);
  virtual bool TryEnterAvailableLadder(float DesiredClimbInput = 1.0f);
  virtual bool HandleLadderClimbInput(float InClimbInput,
                                      float InputDeadZone = 0.0f);
  virtual bool HandleLadderHorizontalExitInput(float InHorizontalInput,
                                               float InputDeadZone = 0.0f);
  virtual void SetLadderClimbInput(float InClimbInput);
  virtual bool PerformLadderJump();
  virtual bool PerformLadderCrouch();
  UFUNCTION(BlueprintPure, Category = "Ladder|Animation")
  bool ShouldPlayLadderStartAnimation() const;
  UFUNCTION(BlueprintPure, Category = "Ladder|Animation")
  bool ShouldPlayLadderLoopAnimation() const;
  UFUNCTION(BlueprintPure, Category = "Ladder|Animation")
  bool ShouldPlayLadderEndAnimation() const;
  UFUNCTION(BlueprintPure, Category = "Ladder|Movement")
  bool IsLadderTopFinishActive() const;
  virtual void
  GetAvailableLedgeGrabs(TArray<APlatformerLedgeGrab *> &OutLedgeGrabs) const;
  virtual void ApplyDeveloperCharacterSettings(
      const FDeveloperPlatformerCharacterSettings &DeveloperSettings);
  virtual void ApplyDeveloperSettingsSnapshot(
      const FPlatformerDeveloperSettingsSnapshot &DeveloperSettingsSnapshot);
  virtual void ApplyCameraRigSettings(
      const FDeveloperPlatformerCameraSettings &CameraSettings);
  virtual FDeveloperPlatformerCharacterSettings
  CaptureDeveloperCharacterSettings() const;
  virtual FDeveloperPlatformerCameraSettings CaptureCameraRigSettings() const;
  virtual FPlatformerDeveloperSettingsSnapshot
  CaptureDeveloperSettingsSnapshot() const;
  virtual FVector GetPlatformerCameraFocusLocation() const;
  UFUNCTION(BlueprintCallable, Category = "Developer|Jump Preview")
  ADeveloperJumpTrajectory *SpawnJumpTrajectorySnapshotActor();
  UFUNCTION(BlueprintCallable, Category = "Developer|Debug")
  void SetPrintDeveloperCharacterStateEveryTick(bool bInPrintEveryTick);
  UFUNCTION(BlueprintCallable, Category = "Developer|Debug")
  void PrintDeveloperCharacterState() const;
  UFUNCTION(BlueprintPure, Category = "Developer|Debug")
  FString GetDeveloperCharacterStateDebugString() const;

  FORCEINLINE UCameraComponent *GetFollowCamera() const { return FollowCamera; }
  FORCEINLINE USpringArmComponent *GetCameraBoom() const { return CameraBoom; }
  FORCEINLINE UPlatformerAnimDataAsset *GetAnimDataAsset() const { return AnimDataAsset; }
  FORCEINLINE UPlatformerLocomotionAnimSet *GetLocomotionAnimSet() const { return LocomotionAnimSet; }
  FORCEINLINE EPlatformerLadderState GetLadderState() const { return LadderState; }
  /** Character is mounted on a ladder and can drive climb input. Excludes TopFinish. */
  bool IsOnLadder() const;
  FORCEINLINE APlatformerLadder *GetActiveLadder() const {
    return ActiveLadder;
  }
  FORCEINLINE APlatformerLadder *GetAvailableLadder() const {
    return AvailableLadder;
  }
  UFUNCTION(BlueprintPure, Category = "Ladder|Movement")
  float GetLadderClimbInput() const;
  FORCEINLINE bool HasDeveloperCrouchCapsuleScaleOverride() const {
    return bHasDeveloperCrouchCapsuleScaleOverride;
  }
  FORCEINLINE bool HasDeveloperJumpHorizontalSpeedOverride() const {
    return bHasDeveloperJumpHorizontalSpeedOverride;
  }
  float
  ResolveDeveloperCrouchCapsuleScale(float DefaultCrouchCapsuleScale) const;
  float
  ResolveDeveloperJumpHorizontalSpeed(float DefaultJumpHorizontalSpeed) const;
  bool HasActiveDeveloperCombatSettings() const;
  const FDeveloperPlatformerCombatSettings &
  GetActiveDeveloperCombatSettings() const;

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void
  SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;
  virtual void FellOutOfWorld(const UDamageType &DamageType) override;
  virtual void OnCombatDeath(AActor *DamageInstigatorActor) override;
  virtual void OnCombatRevived() override;
  virtual void HandleCombatDeathAftermath(AActor *DamageInstigatorActor) override;
  virtual void OnEnteredLadder(APlatformerLadder *Ladder);
  virtual void OnExitedLadder(APlatformerLadder *Ladder);
  virtual void ApplyDeveloperCameraSettings(
      const FDeveloperPlatformerCameraSettings &DeveloperCameraSettings);
  virtual FDeveloperPlatformerCameraSettings
  CaptureDeveloperCameraSettings() const;
  virtual void ApplyDeveloperCharacterMovementSettings(
      const FDeveloperPlatformerCharacterMovementSettings
          &DeveloperCharacterMovementSettings);
  virtual FDeveloperPlatformerCharacterMovementSettings
  CaptureDeveloperCharacterMovementSettings() const;
  virtual void ApplyDeveloperCombatSettings(
      const FDeveloperPlatformerCombatSettings &DeveloperCombatSettings);
  virtual FDeveloperPlatformerCombatSettings
  CaptureDeveloperCombatSettings() const;
  virtual float GetHealthWidgetVerticalPadding() const override;
  void SetShowJumpTrajectoryPreview(bool bInShowJumpTrajectoryPreview);
  ADeveloperJumpTrajectory *EnsureDeveloperJumpTrajectoryActor();
  void DestroyDeveloperJumpTrajectoryActor();
  void SetDeveloperCrouchCapsuleScaleOverride(float InCrouchCapsuleScale);
  void ClearDeveloperCrouchCapsuleScaleOverride();
  void SetDeveloperJumpHorizontalSpeedOverride(float InJumpHorizontalSpeed);
  void ClearDeveloperJumpHorizontalSpeedOverride();
  float CaptureDeveloperCrouchCapsuleScale() const;
  float CaptureDeveloperJumpHorizontalSpeed() const;
  float ResolveDefaultCrouchCapsuleScale() const;
  float ResolveStandingCapsuleHalfHeight() const;
  void ApplyResolvedCrouchCapsuleScale();

  // === Ladder FSM internals ===
  // Single state-machine entry point. All ladder transitions flow through this.
  void SetLadderState(EPlatformerLadderState NewState);
  // Per-tick driver for the active ladder state.
  void TickLadderState(float DeltaTime);
  // Per-state velocity application; expects the active ladder is valid.
  void ApplyLadderVelocity();
  // Snap the character to the active ladder once (called on Mount).
  void SnapCharacterToActiveLadder();
  // Save / restore character movement state around the ladder lifecycle.
  void SaveCharacterMovementForLadder();
  void RestoreCharacterMovementFromLadder();
  // Position predicates against the active ladder bounds.
  bool IsCharacterAtOrBelowLadderBottom(const APlatformerLadder *Ladder) const;
  bool IsCharacterAtOrAboveLadderTop(const APlatformerLadder *Ladder) const;
  /** True when CharacterMovement::FindFloor reports a walkable floor within MaxDistance below feet.
   *  Used to dismount the ladder when descent stalls because the ladder bottom
   *  is below the actual ground. */
  bool HasWalkableFloorClose(float MaxDistance) const;
  // Snap character feet to top surface after a top dismount.
  void SnapCharacterToLadderTopSurface(const APlatformerLadder *Ladder);
  float GetCharacterFeetWorldZ() const;
  float GetWorldTimeSafe() const;
  // Reentry cooldown after a dismount (any path).
  bool IsLadderReentryOnCooldown() const;
  void ArmLadderReentryCooldown();
  /** Mark that ladder input must return to neutral before being honored again.
   *  Set after TopFinish and bottom dismount so a held stick doesn't auto-remount. */
  void ArmLadderInputReleaseGate();
  /** Update the input-release gate based on current input magnitude. Clears gate
   *  when input has fallen back through the neutral zone. */
  void TickLadderInputReleaseGate(float CurrentInputMagnitude);
  const UGA_PlatformerJump *FindGrantedJumpAbility() const;
  void RefreshJumpTrajectoryPreview();
  FDeveloperPlatformerCombatSettings
  ResolveDeveloperCombatSettingsForApplication(
      const FDeveloperPlatformerCombatSettings &DeveloperCombatSettings) const;
  void LoadAndApplyDeveloperSettings();
  void
  SetHasActiveDeveloperCombatSettings(bool bInHasActiveDeveloperCombatSettings);
  void DisablePlayerInputForDeath();
  float PlayCombatDeathAnimation();
  void StartCombatDeathSequence(AActor *DamageInstigatorActor);
  void FinishCombatDeathSequence();

  UPROPERTY(Transient)
  TObjectPtr<APlatformerLadder> ActiveLadder;

  UPROPERTY(Transient)
  TObjectPtr<APlatformerLadder> AvailableLadder;

  UPROPERTY(Transient)
  TArray<TWeakObjectPtr<APlatformerLedgeGrab>> AvailableLedgeGrabs;

  // === Ladder FSM state ===

  UPROPERTY(BlueprintReadOnly, Transient, Category = "Ladder|State")
  EPlatformerLadderState LadderState = EPlatformerLadderState::None;

  /** Time remaining for the current state's timer (Mounting / JumpBoost / CrouchBoost / TopFinish). */
  UPROPERTY(Transient)
  float LadderStateTimeRemaining = 0.0f;

  UPROPERTY(Transient)
  float LadderClimbInput = 0.0f;

  /** WorldTime at which the reentry cooldown expires (-1.0 = no cooldown active). */
  UPROPERTY(Transient)
  float LadderReentryCooldownEndTime = -1.0f;

  /** When true, ladder climb input is held in a "must release first" state.
   *  Set on TopFinish entry and on bottom-dismount so a held analog stick
   *  can't immediately auto-remount the ladder the player just left. Cleared
   *  by TickLadderInputReleaseGate when input falls back through neutral. */
  UPROPERTY(Transient)
  bool bRequiresLadderInputRelease = false;


  // Saved character movement state captured at Mount, restored on dismount.
  UPROPERTY(Transient)
  float SavedPreLadderGravityScale = 1.0f;

  UPROPERTY(Transient)
  TEnumAsByte<EMovementMode> SavedPreLadderMovementMode = MOVE_Walking;

  UPROPERTY(Transient)
  bool bHadPreLadderGravityOverride = false;

  UPROPERTY(Transient)
  float PreLadderGravityOverride = 1.0f;

  UPROPERTY(Transient)
  float SavedPreLadderMaxFlySpeed = 0.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ladder",
            meta = (ClampMin = 0.0, Units = "cm"))
  float LadderBottomExitTolerance = 4.0f;

  UPROPERTY(Transient)
  bool bHasActiveDeveloperCombatSettings = false;

  UPROPERTY(Transient)
  FDeveloperPlatformerCombatSettings ActiveDeveloperCombatSettings;

  UPROPERTY(Transient)
  bool bHasDeveloperCrouchCapsuleScaleOverride = false;

  UPROPERTY(Transient)
  float DeveloperCrouchCapsuleScaleOverride = 0.0f;

  UPROPERTY(Transient)
  bool bHasDeveloperJumpHorizontalSpeedOverride = false;

  UPROPERTY(Transient)
  float DeveloperJumpHorizontalSpeedOverride = 0.0f;

  UPROPERTY(Transient)
  bool bShowJumpTrajectoryPreview = false;

  UPROPERTY(Transient)
  TObjectPtr<ADeveloperJumpTrajectory> DeveloperJumpTrajectoryActor;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer|Debug")
  bool bPrintDeveloperCharacterStateEveryTick = true;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI",
            meta = (ClampMin = 0.0, Units = "cm"))
  float PlatformerHealthWidgetVerticalPadding = 20.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Death",
            meta = (ClampMin = 0.0, Units = "s"))
  float DeathPostAnimationDelay = 0.3f;

  /** Data-driven mapping of GameplayTag → AnimMontage for ability animations. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
  TObjectPtr<UPlatformerAnimDataAsset> AnimDataAsset;

  /** Locomotion sequences and blendspaces for this character's skeleton. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
  TObjectPtr<UPlatformerLocomotionAnimSet> LocomotionAnimSet;

  /** Subclass of AnimBP to link as anim layers at BeginPlay (optional). */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
  TSubclassOf<UAnimInstance> LinkedAnimLayerClass;

  UPROPERTY(Transient)
  FTimerHandle DeathSequenceTimerHandle;

  UPROPERTY(Transient)
  TWeakObjectPtr<AActor> PendingDeathInstigatorActor;
};
