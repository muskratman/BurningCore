
#include "Character/PlatformerCharacterBase.h"
//
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/PlatformerAnimDataAsset.h"
#include "Animation/PlatformerAnimGameplayTags.h"
#include "Animation/PlatformerAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Character/SideViewMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/PlatformerDeveloperSettingsSubsystem.h"
#include "Core/PlatformerPlayerControllerBase.h"
#include "Developer/DeveloperJumpTrajectory.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GAS/Abilities/GA_PlatformerJump.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/PlatformerAbilitySet.h"
#include "GAS/PlatformerGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Platformer/Environment/PlatformerLadder.h"
#include "Platformer/Environment/PlatformerLedgeGrab.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "Traversal/PlatformerTraversalTypes.h"

namespace PlatformerCharacterDebug {
FString GetCustomMovementModeName(uint8 CustomMovementMode) {
  switch (static_cast<EPlatformerTraversalCustomMode>(CustomMovementMode)) {
  case EPlatformerTraversalCustomMode::LedgeHang:
    return TEXT("Custom:LedgeHang");
  case EPlatformerTraversalCustomMode::LedgeClimb:
    return TEXT("Custom:LedgeClimb");
  case EPlatformerTraversalCustomMode::WallSlide:
    return TEXT("Custom:WallSlide");
  case EPlatformerTraversalCustomMode::Dash:
    return TEXT("Custom:Dash");
  case EPlatformerTraversalCustomMode::None:
  default:
    return FString::Printf(TEXT("Custom:%d"), CustomMovementMode);
  }
}

FString GetMovementModeName(const UCharacterMovementComponent *Movement) {
  if (!Movement) {
    return TEXT("NoMovementComponent");
  }

  switch (Movement->MovementMode) {
  case MOVE_None:
    return TEXT("None");
  case MOVE_Walking:
    return TEXT("Walking");
  case MOVE_NavWalking:
    return TEXT("NavWalking");
  case MOVE_Falling:
    return TEXT("Falling");
  case MOVE_Swimming:
    return TEXT("Swimming");
  case MOVE_Flying:
    return TEXT("Flying");
  case MOVE_Custom:
    return GetCustomMovementModeName(Movement->CustomMovementMode);
  default:
    return FString::Printf(TEXT("Unknown:%d"),
                           static_cast<int32>(Movement->MovementMode));
  }
}

const TCHAR *BoolName(bool bValue) {
  return bValue ? TEXT("true") : TEXT("false");
}
} // namespace PlatformerCharacterDebug

APlatformerCharacterBase::APlatformerCharacterBase(
    const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer
                .SetDefaultSubobjectClass<USideViewMovementComponent>(
                    ACharacter::CharacterMovementComponentName)) {
  PrimaryActorTick.bCanEverTick = true;

  GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
  GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 800.0f;
  CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
  CameraBoom->bDoCollisionTest = false;
  CameraBoom->bUsePawnControlRotation = false;
  CameraBoom->bInheritPitch = false;
  CameraBoom->bInheritYaw = false;
  CameraBoom->bInheritRoll = false;

  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
  FollowCamera->bUsePawnControlRotation = false;

  DeveloperJumpTrajectoryClass = ADeveloperJumpTrajectory::StaticClass();

  if (AttributeSet) {
    AttributeSet->InitMaxHealth(100.0f);
    AttributeSet->InitHealth(100.0f);
    AttributeSet->InitRangeBaseAttackDamage(25.0f);
    AttributeSet->InitRangeChargedAttackDamage(75.0f);
  }

  UpdateHealthWidgetPlacement();
}

void APlatformerCharacterBase::InitializeAbilities(
    const UPlatformerAbilitySet *AbilitySet) {
  if (!AbilitySystemComponent || !AbilitySet || !HasAuthority()) {
    return;
  }

  for (const FPlatformerAbilitySet_Ability &AbilityData :
       AbilitySet->Abilities) {
    if (AbilityData.AbilityClass) {
      FGameplayAbilitySpec AbilitySpec(AbilityData.AbilityClass,
                                       AbilityData.AbilityLevel, INDEX_NONE,
                                       this);
      if (AbilityData.InputTag.IsValid()) {
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityData.InputTag);
      }

      AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
  }

  FGameplayEffectContextHandle ContextHandle =
      AbilitySystemComponent->MakeEffectContext();
  ContextHandle.AddInstigator(this, this);

  for (const FPlatformerAbilitySet_Effect &EffectData : AbilitySet->Effects) {
    if (!EffectData.EffectClass) {
      continue;
    }

    FGameplayEffectSpecHandle EffectSpec =
        AbilitySystemComponent->MakeOutgoingSpec(
            EffectData.EffectClass, EffectData.EffectLevel, ContextHandle);
    if (EffectSpec.IsValid()) {
      AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
          *EffectSpec.Data.Get());
    }
  }
}

bool APlatformerCharacterBase::TryActivateAbilityByInputTag(
    const FGameplayTag &InputTag) {
  if (!AbilitySystemComponent || !InputTag.IsValid()) {
    return false;
  }

  bool bActivatedAbility = false;
  TArray<FGameplayAbilitySpec> &ActivatableAbilities =
      AbilitySystemComponent->GetActivatableAbilities();
  for (FGameplayAbilitySpec &AbilitySpec : ActivatableAbilities) {
    if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) {
      bActivatedAbility |=
          AbilitySystemComponent->TryActivateAbility(AbilitySpec.Handle);
    }
  }

  return bActivatedAbility;
}

bool APlatformerCharacterBase::ReleaseAbilityInputTag(
    const FGameplayTag &InputTag) {
  if (!AbilitySystemComponent || !InputTag.IsValid()) {
    return false;
  }

  bool bReleasedAbility = false;
  TArray<FGameplayAbilitySpec> &ActivatableAbilities =
      AbilitySystemComponent->GetActivatableAbilities();
  for (FGameplayAbilitySpec &AbilitySpec : ActivatableAbilities) {
    if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) {
      AbilitySystemComponent->AbilitySpecInputReleased(AbilitySpec);
      bReleasedAbility = true;
    }
  }

  return bReleasedAbility;
}

bool APlatformerCharacterBase::HasAbilityInputTag(
    const FGameplayTag &InputTag) const {
  if (!AbilitySystemComponent || !InputTag.IsValid()) {
    return false;
  }

  const TArray<FGameplayAbilitySpec> &ActivatableAbilities =
      AbilitySystemComponent->GetActivatableAbilities();
  for (const FGameplayAbilitySpec &AbilitySpec : ActivatableAbilities) {
    if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) {
      return true;
    }
  }

  return false;
}

void APlatformerCharacterBase::ApplyDeveloperCharacterSettings(
    const FDeveloperPlatformerCharacterSettings &DeveloperSettings) {
  ApplyDeveloperCameraSettings(DeveloperSettings.DeveloperCameraSettings);
  ApplyDeveloperCharacterMovementSettings(
      DeveloperSettings.DeveloperCharacterMovementSettings);
  ApplyDeveloperCombatSettings(DeveloperSettings.DeveloperCombatSettings);
}

void APlatformerCharacterBase::ApplyDeveloperSettingsSnapshot(
    const FPlatformerDeveloperSettingsSnapshot &DeveloperSettingsSnapshot) {
  const bool bPreviousShowJumpTrajectoryPreview = bShowJumpTrajectoryPreview;

  ApplyDeveloperCameraSettings(
      DeveloperSettingsSnapshot.CharacterSettings.DeveloperCameraSettings);
  ApplyDeveloperCharacterMovementSettings(
      DeveloperSettingsSnapshot.CharacterSettings
          .DeveloperCharacterMovementSettings);

  if (!DeveloperSettingsSnapshot.bHasSavedJumpHorizontalSpeed) {
    ClearDeveloperJumpHorizontalSpeedOverride();
  }

  if (!DeveloperSettingsSnapshot.bHasSavedCrouchCapsuleScale) {
    ClearDeveloperCrouchCapsuleScaleOverride();
  }

  if (!DeveloperSettingsSnapshot.bHasSavedJumpTrajectoryPreview) {
    SetShowJumpTrajectoryPreview(bPreviousShowJumpTrajectoryPreview);
    RefreshJumpTrajectoryPreview();
  }

  if (DeveloperSettingsSnapshot.bHasSavedCombatSettings) {
    ApplyDeveloperCombatSettings(ResolveDeveloperCombatSettingsForApplication(
        DeveloperSettingsSnapshot.CharacterSettings.DeveloperCombatSettings));
  } else {
    SetHasActiveDeveloperCombatSettings(false);
    ActiveDeveloperCombatSettings = FDeveloperPlatformerCombatSettings();
  }
}

void APlatformerCharacterBase::ApplyCameraRigSettings(
    const FDeveloperPlatformerCameraSettings &CameraSettings) {
  if (CameraBoom) {
    CameraBoom->TargetArmLength = CameraSettings.DeveloperSpringArmArmLength;
    CameraBoom->SetRelativeLocation(CameraSettings.DeveloperSpringArmLocation);
    CameraBoom->SetRelativeRotation(
        FRotator(CameraSettings.DeveloperSpringArmRotation.X,
                 CameraSettings.DeveloperSpringArmRotation.Y,
                 CameraSettings.DeveloperSpringArmRotation.Z));
  }

  if (FollowCamera) {
    FollowCamera->SetRelativeLocation(CameraSettings.DeveloperCameraLocation);
    FollowCamera->SetRelativeRotation(
        FRotator(CameraSettings.DeveloperCameraRotation.X,
                 CameraSettings.DeveloperCameraRotation.Y,
                 CameraSettings.DeveloperCameraRotation.Z));
  }
}

FDeveloperPlatformerCharacterSettings
APlatformerCharacterBase::CaptureDeveloperCharacterSettings() const {
  FDeveloperPlatformerCharacterSettings DeveloperSettings;
  DeveloperSettings.DeveloperCameraSettings = CaptureDeveloperCameraSettings();
  DeveloperSettings.DeveloperCharacterMovementSettings =
      CaptureDeveloperCharacterMovementSettings();
  DeveloperSettings.DeveloperCombatSettings = CaptureDeveloperCombatSettings();
  return DeveloperSettings;
}

FDeveloperPlatformerCameraSettings
APlatformerCharacterBase::CaptureCameraRigSettings() const {
  return CaptureDeveloperCameraSettings();
}

FPlatformerDeveloperSettingsSnapshot
APlatformerCharacterBase::CaptureDeveloperSettingsSnapshot() const {
  FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
  DeveloperSettingsSnapshot.CharacterSettings =
      CaptureDeveloperCharacterSettings();
  DeveloperSettingsSnapshot.bHasSavedCombatSettings =
      HasActiveDeveloperCombatSettings();
  DeveloperSettingsSnapshot.bHasSavedJumpHorizontalSpeed =
      bHasDeveloperJumpHorizontalSpeedOverride;
  DeveloperSettingsSnapshot.bHasSavedCrouchCapsuleScale =
      bHasDeveloperCrouchCapsuleScaleOverride;
  DeveloperSettingsSnapshot.bHasSavedJumpTrajectoryPreview = true;
  return DeveloperSettingsSnapshot;
}

FVector APlatformerCharacterBase::GetPlatformerCameraFocusLocation() const {
  return GetActorLocation();
}

void APlatformerCharacterBase::SetPrintDeveloperCharacterStateEveryTick(
    bool bInPrintEveryTick) {
  bPrintDeveloperCharacterStateEveryTick = bInPrintEveryTick;
}

void APlatformerCharacterBase::PrintDeveloperCharacterState() const {
  UKismetSystemLibrary::PrintString(
      this, GetDeveloperCharacterStateDebugString(), true, false,
      FLinearColor::Yellow, 0.0f, GetFName());
}

FString
APlatformerCharacterBase::GetDeveloperCharacterStateDebugString() const {
  const UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  const UPlatformerTraversalMovementComponent *TraversalMovementComponent =
      Cast<UPlatformerTraversalMovementComponent>(MovementComponent);

  const bool bIsDeadState =
      AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
                                    PlatformerGameplayTags::State_Combat_Dead);
  const bool bIsLedgeHanging = TraversalMovementComponent &&
                               TraversalMovementComponent->IsHangingOnLedge();
  const bool bIsLedgeClimbing = TraversalMovementComponent &&
                                TraversalMovementComponent->IsClimbingLedge();
  const bool bIsDashing =
      TraversalMovementComponent && TraversalMovementComponent->IsDashing();
  const bool bIsWallSliding =
      TraversalMovementComponent && TraversalMovementComponent->IsWallSliding();
  const bool bIsWallJumping =
      (TraversalMovementComponent &&
       TraversalMovementComponent->IsWallJumping()) ||
      (AbilitySystemComponent &&
       AbilitySystemComponent->HasMatchingGameplayTag(
           PlatformerTraversalGameplayTags::State_Movement_WallJump));
  const bool bIsFalling =
      MovementComponent && MovementComponent->MovementMode == MOVE_Falling;
  const bool bIsGrounded =
      MovementComponent && MovementComponent->IsMovingOnGround();
  const FVector CurrentVelocity = GetVelocity();
  const float GroundSpeed =
      FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f).Size();
  constexpr float GroundSpeedThreshold = 3.0f;
  const bool bIsLadderActive = IsOnLadder();

  FString StateName = TEXT("Idle");
  if (bIsDeadState) {
    StateName = TEXT("Dead");
  } else if (bIsLadderActive) {
    StateName = TEXT("Ladder");
  } else if (bIsLedgeClimbing) {
    StateName = TEXT("LedgeGrabClimb");
  } else if (bIsLedgeHanging) {
    StateName = TEXT("LedgeGrabHang");
  } else if (bIsDashing) {
    StateName = TEXT("Dash");
  } else if (bIsWallJumping) {
    StateName = TEXT("WallJump");
  } else if (bIsWallSliding) {
    StateName = TEXT("WallSlide");
  } else if (bIsFalling) {
    StateName = CurrentVelocity.Z > 0.0f ? TEXT("Jump") : TEXT("Fall");
  } else if (bIsCrouched) {
    StateName = GroundSpeed > GroundSpeedThreshold ? TEXT("CrouchRun")
                                                   : TEXT("CrouchIdle");
  } else if (bIsGrounded && GroundSpeed > GroundSpeedThreshold) {
    StateName = TEXT("Run");
  }

  int32 ValidLedgeGrabCount = 0;
  for (const TWeakObjectPtr<APlatformerLedgeGrab> &AvailableLedgeGrab :
       AvailableLedgeGrabs) {
    if (AvailableLedgeGrab.IsValid()) {
      ++ValidLedgeGrabCount;
    }
  }

  const FVector2D TraversalInput =
      TraversalMovementComponent
          ? TraversalMovementComponent->GetTraversalInputVector()
          : FVector2D::ZeroVector;

  return FString::Printf(
      TEXT("CharacterState: %s\nMoveMode=%s InAir=%s Grounded=%s Crouched=%s "
           "Ladder=%s\nTraversal: LedgeHang=%s LedgeClimb=%s Dash=%s "
           "WallJump=%s WallSlide=%s AvailableLedges=%d\nVel=(X=%.1f Z=%.1f) "
           "GroundSpeed=%.1f Input=(X=%.2f Y=%.2f)"),
      *StateName,
      *PlatformerCharacterDebug::GetMovementModeName(MovementComponent),
      PlatformerCharacterDebug::BoolName(bIsFalling),
      PlatformerCharacterDebug::BoolName(bIsGrounded),
      PlatformerCharacterDebug::BoolName(bIsCrouched),
      PlatformerCharacterDebug::BoolName(bIsLadderActive),
      PlatformerCharacterDebug::BoolName(bIsLedgeHanging),
      PlatformerCharacterDebug::BoolName(bIsLedgeClimbing),
      PlatformerCharacterDebug::BoolName(bIsDashing),
      PlatformerCharacterDebug::BoolName(bIsWallJumping),
      PlatformerCharacterDebug::BoolName(bIsWallSliding), ValidLedgeGrabCount,
      CurrentVelocity.X, CurrentVelocity.Z, GroundSpeed, TraversalInput.X,
      TraversalInput.Y);
}

float APlatformerCharacterBase::ResolveDeveloperCrouchCapsuleScale(
    float DefaultCrouchCapsuleScale) const {
  return bHasDeveloperCrouchCapsuleScaleOverride
             ? DeveloperCrouchCapsuleScaleOverride
             : FMath::Max(DefaultCrouchCapsuleScale, 0.0f);
}

float APlatformerCharacterBase::ResolveDeveloperJumpHorizontalSpeed(
    float DefaultJumpHorizontalSpeed) const {
  return bHasDeveloperJumpHorizontalSpeedOverride
             ? DeveloperJumpHorizontalSpeedOverride
             : FMath::Max(DefaultJumpHorizontalSpeed, 0.0f);
}

void APlatformerCharacterBase::NotifyLadderAvailable(
    APlatformerLadder *Ladder) {
  if (!IsValid(Ladder)) {
    return;
  }

  AvailableLadder = Ladder;
}

void APlatformerCharacterBase::NotifyLadderUnavailable(
    APlatformerLadder *Ladder) {
  if (!Ladder) {
    return;
  }

  if (AvailableLadder == Ladder) {
    AvailableLadder = nullptr;
  }

  if (ActiveLadder == Ladder) {
    // Tick-order race-condition guard: TickLadderState checks position from
    // the PREVIOUS frame. The CMC tick then moves the character by
    // ClimbSpeed * dt (~5 cm at 60 fps), which can overshoot the trigger
    // window and exit the ClimbVolume before the actor tick catches it.
    // When that happens, arrive here while still climbing upward → finish
    // the climb via the push-up mechanic instead of falling.
    if (IsOnLadder() && LadderClimbInput > 0.0f) {
      FinishLadderClimbAtTop(Ladder);
      return;
    }

    SetLadderState(EPlatformerLadderState::None);
  }
}

void APlatformerCharacterBase::NotifyLedgeGrabAvailable(
    APlatformerLedgeGrab *LedgeGrab) {
  if (!LedgeGrab) {
    return;
  }

  AvailableLedgeGrabs.AddUnique(LedgeGrab);
}

void APlatformerCharacterBase::NotifyLedgeGrabUnavailable(
    APlatformerLedgeGrab *LedgeGrab) {
  if (!LedgeGrab) {
    return;
  }

  AvailableLedgeGrabs.RemoveAll(
      [LedgeGrab](
          const TWeakObjectPtr<APlatformerLedgeGrab> &AvailableLedgeGrab) {
        return !AvailableLedgeGrab.IsValid() ||
               AvailableLedgeGrab.Get() == LedgeGrab;
      });
}

// =====================================================================================
// Ladder FSM
// =====================================================================================
//
// State graph (single source of truth: LadderState).
//
//   None --(EnterLadder ok)--> Mounting
//   Mounting --(MountDuration elapsed)--> Climbing
//   Climbing --(PerformLadderJump)--> JumpBoost
//   Climbing --(PerformLadderCrouch)--> CrouchBoost
//   JumpBoost --(JumpClimbDuration elapsed)--> Climbing
//   CrouchBoost --(CrouchClimbDuration elapsed)--> Climbing
//
//   Climbing | JumpBoost --(reach top)--> TopFinish
//   Climbing | CrouchBoost --(reach bottom)--> None (DismountFalling)
//   Climbing --(horizontal exit input above threshold)--> None
//   (DismountFalling) Any active state --(active ladder destroyed /
//   unavailable)--> None (DismountFalling)
//
//   TopFinish --(TopFinishDuration elapsed)--> None
//
// IsOnLadder() returns true for {Mounting, Climbing, JumpBoost, CrouchBoost}.
// IsLadderTopFinishActive() returns true only for TopFinish.
//
// Animation derivations (read by UPlatformerAnimInstance):
//   bShouldLadderStart  iff state == Mounting
//   bShouldLadderLoop   iff state in {Climbing, JumpBoost, CrouchBoost}
//   bShouldLadderEnd    iff state == TopFinish
//
// Reentry cooldown: armed on every transition to None (any path). Blocks
// TryEnterAvailableLadder until the active ladder's ReentryCooldownDuration
// elapses, preventing the auto-mount loop right after a sideways/bottom
// dismount.

bool APlatformerCharacterBase::IsOnLadder() const {
  switch (LadderState) {
  case EPlatformerLadderState::Mounting:
  case EPlatformerLadderState::Climbing:
  case EPlatformerLadderState::JumpBoost:
  case EPlatformerLadderState::CrouchBoost:
    return true;
  default:
    return false;
  }
}

bool APlatformerCharacterBase::IsLadderTopFinishActive() const {
  return LadderState == EPlatformerLadderState::TopFinish;
}

bool APlatformerCharacterBase::ShouldPlayLadderStartAnimation() const {
  return LadderState == EPlatformerLadderState::Mounting;
}

bool APlatformerCharacterBase::ShouldPlayLadderLoopAnimation() const {
  return LadderState == EPlatformerLadderState::Climbing ||
         LadderState == EPlatformerLadderState::JumpBoost ||
         LadderState == EPlatformerLadderState::CrouchBoost;
}

bool APlatformerCharacterBase::ShouldPlayLadderEndAnimation() const {
  return LadderState == EPlatformerLadderState::TopFinish;
}

float APlatformerCharacterBase::GetLadderClimbInput() const {
  switch (LadderState) {
  case EPlatformerLadderState::JumpBoost:
    return 1.0f;
  case EPlatformerLadderState::CrouchBoost:
    return -1.0f;
  case EPlatformerLadderState::Climbing:
  case EPlatformerLadderState::Mounting:
    return LadderClimbInput;
  default:
    return 0.0f;
  }
}

void APlatformerCharacterBase::SetLadderClimbInput(float InClimbInput) {
  LadderClimbInput = FMath::Clamp(InClimbInput, -1.0f, 1.0f);
}

bool APlatformerCharacterBase::EnterLadder(APlatformerLadder *Ladder) {
  if (!IsValid(Ladder)) {
    return false;
  }

  // Already mounted on the same ladder — treat as success.
  if (IsOnLadder() && ActiveLadder == Ladder) {
    return true;
  }

  // TopFinish is a hard lock; any re-entry must wait for it to expire.
  if (LadderState == EPlatformerLadderState::TopFinish) {
    return false;
  }

  if (AvailableLadder != Ladder) {
    return false;
  }

  // Switching ladders mid-climb: cleanly leave the previous one first.
  if (IsOnLadder()) {
    SetLadderState(EPlatformerLadderState::None);
  }

  if (bIsCrouched) {
    UnCrouch();
  }

  ActiveLadder = Ladder;
  SetLadderState(EPlatformerLadderState::Mounting);

  if (LadderState != EPlatformerLadderState::Mounting) {
    // SetLadderState rejected — Mount setup failed (e.g., no movement
    // component).
    ActiveLadder = nullptr;
    return false;
  }

  OnEnteredLadder(Ladder);
  return true;
}

void APlatformerCharacterBase::ExitLadder(APlatformerLadder *Ladder) {
  if (!IsOnLadder()) {
    return;
  }

  if (Ladder && ActiveLadder != Ladder) {
    return;
  }

  // External request to leave — bypass TopFinish, drop straight to None.
  SetLadderState(EPlatformerLadderState::None);
}

void APlatformerCharacterBase::FinishLadderClimbAtTop(
    APlatformerLadder *Ladder) {
  // Public entry point for "character reached the top of the ladder".
  // All triggers (TopEntryVolume overlap, position check, tick overshoot)
  // funnel through here. The actual work is done in SetLadderState(TopFinish):
  //  - Snap to top surface
  //  - Restore TopBlock collision (via HandleCharacterExitedAtTop)
  //  - Start TopFinish timer → AnimBP plays AS_Platformer_Ladder_End
  //  - Block input until timer expires
  //  - Restore movement mode → MOVE_Walking when the timer expires
  if (!IsValid(Ladder) || ActiveLadder != Ladder) {
    return;
  }
  SetLadderState(EPlatformerLadderState::TopFinish);
}

bool APlatformerCharacterBase::TryEnterAvailableLadder(
    float DesiredClimbInput) {
  if (LadderState == EPlatformerLadderState::TopFinish) {
    return false;
  }

  if (IsLadderReentryOnCooldown()) {
    return false;
  }

  // Defense-in-depth: if some other code path bypassed HandleLadderClimbInput
  // and called TryEnterAvailableLadder directly, still respect the release
  // gate.
  if (bRequiresLadderInputRelease) {
    return false;
  }

  if (!IsValid(AvailableLadder)) {
    AvailableLadder = nullptr;
    return false;
  }

  APlatformerLadder *LadderToEnter = AvailableLadder;
  if (!LadderToEnter->CanCharacterEnterLadder(this, DesiredClimbInput)) {
    return false;
  }

  if (!EnterLadder(LadderToEnter)) {
    return false;
  }

  LadderToEnter->HandleCharacterEnteredLadder(this, DesiredClimbInput);
  SetLadderClimbInput(DesiredClimbInput);
  return true;
}

bool APlatformerCharacterBase::HandleLadderClimbInput(float InClimbInput,
                                                      float InputDeadZone) {
  if (LadderState == EPlatformerLadderState::TopFinish) {
    // Even though input is ignored, drive the release gate so it clears
    // when the player lets go during the lock window.
    TickLadderInputReleaseGate(FMath::Abs(InClimbInput));
    return true;
  }

  const float ResolvedDeadZone = FMath::Max(InputDeadZone, 0.0f);
  const float InputMagnitude = FMath::Abs(InClimbInput);
  const float ResolvedClimbInput =
      InputMagnitude > ResolvedDeadZone ? InClimbInput : 0.0f;

  // Update the release gate every call so it clears as soon as the stick
  // returns to neutral, regardless of which branch we take below.
  TickLadderInputReleaseGate(InputMagnitude);

  if (IsOnLadder()) {
    SetLadderClimbInput(ResolvedClimbInput);
    return true;
  }

  // Gate is set after TopFinish or bottom dismount — block re-mount until
  // the player releases the stick. Still report "consumed" so the caller
  // doesn't accidentally route the input to walking/look.
  if (bRequiresLadderInputRelease) {
    return true;
  }

  if (FMath::IsNearlyZero(ResolvedClimbInput)) {
    return false;
  }

  return TryEnterAvailableLadder(ResolvedClimbInput);
}

bool APlatformerCharacterBase::HandleLadderHorizontalExitInput(
    float InHorizontalInput, float InputDeadZone) {
  if (LadderState == EPlatformerLadderState::TopFinish) {
    return true;
  }

  if (!IsOnLadder()) {
    return false;
  }

  // Per-ladder threshold takes precedence; falls back to caller dead zone.
  // This avoids accidental sideways dismounts from analog stick noise.
  float ExitThreshold = FMath::Max(InputDeadZone, 0.0f);
  if (IsValid(ActiveLadder)) {
    ExitThreshold = FMath::Max(ExitThreshold,
                               ActiveLadder->GetHorizontalExitInputThreshold());
  }

  if (FMath::Abs(InHorizontalInput) <= ExitThreshold) {
    return false;
  }

  SetLadderClimbInput(0.0f);
  // Sideways exit always falls — feet are mid-ladder, no top surface to land
  // on.
  SetLadderState(EPlatformerLadderState::None);
  return true;
}

bool APlatformerCharacterBase::PerformLadderJump() {
  if (LadderState == EPlatformerLadderState::TopFinish) {
    return true;
  }

  if (LadderState != EPlatformerLadderState::Climbing &&
      LadderState != EPlatformerLadderState::CrouchBoost) {
    return false;
  }

  if (!IsValid(ActiveLadder) || ActiveLadder->GetJumpClimbDuration() <= 0.0f ||
      ActiveLadder->GetJumpClimbSpeed() <= 0.0f) {
    return false;
  }

  SetLadderState(EPlatformerLadderState::JumpBoost);
  return true;
}

bool APlatformerCharacterBase::PerformLadderCrouch() {
  if (LadderState == EPlatformerLadderState::TopFinish) {
    return true;
  }

  if (LadderState != EPlatformerLadderState::Climbing &&
      LadderState != EPlatformerLadderState::JumpBoost) {
    return false;
  }

  if (!IsValid(ActiveLadder) ||
      ActiveLadder->GetCrouchClimbDuration() <= 0.0f ||
      ActiveLadder->GetCrouchClimbSpeed() <= 0.0f) {
    return false;
  }

  SetLadderState(EPlatformerLadderState::CrouchBoost);
  return true;
}

void APlatformerCharacterBase::SetLadderState(EPlatformerLadderState NewState) {
  const EPlatformerLadderState OldState = LadderState;
  if (OldState == NewState) {
    return;
  }

  // ---- Enter actions ----
  switch (NewState) {
  case EPlatformerLadderState::None: {
    // Only restore movement if we actually held a ladder mount before this.
    // TopFinish has already restored — see its enter case.
    const bool bWasMounted = OldState == EPlatformerLadderState::Mounting ||
                             OldState == EPlatformerLadderState::Climbing ||
                             OldState == EPlatformerLadderState::JumpBoost ||
                             OldState == EPlatformerLadderState::CrouchBoost;

    APlatformerLadder *ExitedLadder =
        IsValid(ActiveLadder) ? ActiveLadder.Get() : nullptr;

    if (bWasMounted) {
      // Notify the ladder before restoring movement so subclasses (e.g.
      // LadderTop) can remove their blocking-collision bypass while the
      // character is still in the ladder movement configuration.
      if (ExitedLadder) {
        ExitedLadder->OnCharacterDismountedBottom(this);
      }

      RestoreCharacterMovementFromLadder();

      // Sideways / bottom dismount → switch to falling so the character drops
      // naturally; jumping off via "Jump" key after Climbing also lands here.
      if (UCharacterMovementComponent *MovementComponent =
              GetCharacterMovement()) {
        MovementComponent->SetMovementMode(MOVE_Falling);
      }

      if (AbilitySystemComponent) {
        AbilitySystemComponent->RemoveLooseGameplayTag(
            PlatformerGameplayTags::State_Movement_Ladder);
      }
    }

    LadderState = EPlatformerLadderState::None;
    LadderStateTimeRemaining = 0.0f;
    LadderClimbInput = 0.0f;
    ActiveLadder = nullptr;

    if (bWasMounted) {
      // TopFinish path already armed the cooldown and fired OnExitedLadder
      // during its own enter; only the bottom/sideways path needs them here.
      ArmLadderReentryCooldown();
      OnExitedLadder(ExitedLadder);
    }
    return;
  }

  case EPlatformerLadderState::Mounting: {
    if (!IsValid(ActiveLadder) || !GetCharacterMovement()) {
      // Caller (EnterLadder) checks state to detect Mount setup failure.
      return;
    }

    const bool bEnteredLadderFromAir = GetCharacterMovement()->IsFalling();

    SaveCharacterMovementForLadder();

    if (USideViewMovementComponent *SideViewMovementComponent =
            Cast<USideViewMovementComponent>(GetCharacterMovement())) {
      SideViewMovementComponent->SetExternalGravityScaleOverride(
          ActiveLadder->GetClimbGravityScale());
    } else {
      GetCharacterMovement()->GravityScale =
          ActiveLadder->GetClimbGravityScale();
    }

    if (ActiveLadder->UsesFlyingMovementMode()) {
      GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    }

    // Decoupled MaxFlySpeed clamp: keeps ladder velocity from leaking
    // into / from the character's general flying speed configuration.
    GetCharacterMovement()->MaxFlySpeed =
        FMath::Max(ActiveLadder->GetLadderMaxFlySpeed(), 1.0f);

    // Freeze velocity at mount — start animation owns the entry frames.
    GetCharacterMovement()->Velocity = FVector::ZeroVector;

    if (AbilitySystemComponent) {
      AbilitySystemComponent->AddLooseGameplayTag(
          PlatformerGameplayTags::State_Movement_Ladder);
    }

    SnapCharacterToActiveLadder();

    // Notify the ladder after the snap so subclasses (e.g. LadderTop) can
    // disable their blocking geometry before the first climb tick.
    ActiveLadder->OnCharacterMounted(this);

    LadderState = EPlatformerLadderState::Mounting;
    LadderStateTimeRemaining =
        bEnteredLadderFromAir
            ? 0.0f
            : FMath::Max(ActiveLadder->GetMountDuration(), 0.0f);
    LadderClimbInput = 0.0f;

    // Mount duration of 0 → skip directly to Climbing. This is the default
    // for air grabs so a falling/jumping character catches the ladder without
    // waiting through a grounded start animation.
    if (LadderStateTimeRemaining <= 0.0f) {
      SetLadderState(EPlatformerLadderState::Climbing);
    }
    return;
  }

  case EPlatformerLadderState::Climbing:
    LadderState = EPlatformerLadderState::Climbing;
    LadderStateTimeRemaining = 0.0f;
    return;

  case EPlatformerLadderState::JumpBoost:
    LadderState = EPlatformerLadderState::JumpBoost;
    LadderStateTimeRemaining =
        IsValid(ActiveLadder)
            ? FMath::Max(ActiveLadder->GetJumpClimbDuration(), 0.0f)
            : 0.0f;
    return;

  case EPlatformerLadderState::CrouchBoost:
    LadderState = EPlatformerLadderState::CrouchBoost;
    LadderStateTimeRemaining =
        IsValid(ActiveLadder)
            ? FMath::Max(ActiveLadder->GetCrouchClimbDuration(), 0.0f)
            : 0.0f;
    return;

  case EPlatformerLadderState::TopFinish: {
    APlatformerLadder *FinishedLadder =
        IsValid(ActiveLadder) ? ActiveLadder.Get() : nullptr;
    const float FinishDuration =
        FinishedLadder
            ? FMath::Max(FinishedLadder->GetTopFinishDuration(), 0.0f)
            : 0.0f;

    SnapCharacterToLadderTopSurface(FinishedLadder);

    // Restore the TopBlock collision now so the surface is solid beneath the
    // character before the animation plays.
    if (FinishedLadder) {
      FinishedLadder->HandleCharacterExitedAtTop(this);
    }

    // Keep MOVE_Flying with gravity=0 for the full TopFinish duration.
    // Switching to MOVE_Walking here would cause the AnimBP to leave the ladder
    // context immediately and skip the LedgeGrab_End animation.
    // RestoreCharacterMovementFromLadder is deferred to TickLadderState when
    // the timer expires (see TopFinish tick case below).
    if (UCharacterMovementComponent *MovementComponent =
            GetCharacterMovement()) {
      MovementComponent->Velocity = FVector::ZeroVector;
    }

    if (AbilitySystemComponent) {
      AbilitySystemComponent->RemoveLooseGameplayTag(
          PlatformerGameplayTags::State_Movement_Ladder);
    }

    LadderState = EPlatformerLadderState::TopFinish;
    LadderStateTimeRemaining = FinishDuration;
    LadderClimbInput = 0.0f;

    OnExitedLadder(FinishedLadder);

    // Fire the cooldown now so the entire TopFinish + cooldown window blocks
    // re-entry — without this, releasing TopFinish over the same volume
    // would re-mount immediately.
    ArmLadderReentryCooldown();

    // Player is almost certainly still holding "up" when they reach the top.
    // Force them to release before any further climb input is honored —
    // otherwise the held input would re-mount the ladder right after
    // TopFinish ends and fight the climb-end animation.
    ArmLadderInputReleaseGate();

    if (FinishDuration <= 0.0f) {
      // Zero duration: no animation to wait for, restore movement immediately.
      RestoreCharacterMovementFromLadder();
      if (UCharacterMovementComponent *MovementComponent =
              GetCharacterMovement()) {
        MovementComponent->SetMovementMode(MOVE_Walking);
        MovementComponent->Velocity = FVector::ZeroVector;
      }
      SetLadderState(EPlatformerLadderState::None);
    }
    return;
  }
  }
}

void APlatformerCharacterBase::TickLadderState(float DeltaTime) {
  if (LadderState == EPlatformerLadderState::None) {
    return;
  }

  // Active ladder vanished while mounted (level streaming, destruction) — bail
  // out.
  if (LadderState != EPlatformerLadderState::TopFinish &&
      !IsValid(ActiveLadder)) {
    SetLadderState(EPlatformerLadderState::None);
    return;
  }

  const float ResolvedDeltaTime = FMath::Max(DeltaTime, 0.0f);
  if (LadderStateTimeRemaining > 0.0f) {
    LadderStateTimeRemaining =
        FMath::Max(LadderStateTimeRemaining - ResolvedDeltaTime, 0.0f);
  }

  switch (LadderState) {
  case EPlatformerLadderState::Mounting:
    // Hold velocity at zero while the start animation plays.
    if (UCharacterMovementComponent *MovementComponent =
            GetCharacterMovement()) {
      MovementComponent->Velocity = FVector::ZeroVector;
    }
    if (LadderStateTimeRemaining <= 0.0f) {
      SetLadderState(EPlatformerLadderState::Climbing);
    }
    return;

  case EPlatformerLadderState::Climbing: {
    // Top exit triggers TopFinish; bottom exit drops to falling. Bottom check
    // also includes "feet touching walkable floor" so a ladder embedded below
    // ground level still releases when the character can't physically descend.
    const bool bDrivingUp = LadderClimbInput > 0.0f;
    const bool bDrivingDown = LadderClimbInput < 0.0f;
    if (bDrivingUp && IsCharacterAtOrAboveLadderTop(ActiveLadder)) {
      // Position-based safety net (primary trigger is TopEntryVolume overlap
      // in APlatformerLadderTop::OnTopEntryVolumeBeginOverlap).
      if (APlatformerLadder *ReachedLadder = ActiveLadder.Get()) {
        FinishLadderClimbAtTop(ReachedLadder);
      }
      return;
    }
    if (bDrivingDown && (IsCharacterAtOrBelowLadderBottom(ActiveLadder) ||
                         HasWalkableFloorClose(LadderBottomExitTolerance))) {
      // Held-down auto-remount guard: even though we just dismount-fall, the
      // player likely still holds down. Block re-entry until release.
      ArmLadderInputReleaseGate();
      SetLadderState(EPlatformerLadderState::None);
      return;
    }
    ApplyLadderVelocity();
    return;
  }

  case EPlatformerLadderState::JumpBoost:
    if (IsCharacterAtOrAboveLadderTop(ActiveLadder)) {
      if (APlatformerLadder *ReachedLadder = ActiveLadder.Get()) {
        FinishLadderClimbAtTop(ReachedLadder);
      }
      return;
    }
    ApplyLadderVelocity();
    if (LadderStateTimeRemaining <= 0.0f) {
      SetLadderState(EPlatformerLadderState::Climbing);
    }
    return;

  case EPlatformerLadderState::CrouchBoost:
    if (IsCharacterAtOrBelowLadderBottom(ActiveLadder) ||
        HasWalkableFloorClose(LadderBottomExitTolerance)) {
      ArmLadderInputReleaseGate();
      SetLadderState(EPlatformerLadderState::None);
      return;
    }
    ApplyLadderVelocity();
    if (LadderStateTimeRemaining <= 0.0f) {
      SetLadderState(EPlatformerLadderState::Climbing);
    }
    return;

  case EPlatformerLadderState::TopFinish:
    if (UCharacterMovementComponent *MovementComponent =
            GetCharacterMovement()) {
      // Pin the character at the top surface while the end animation plays.
      MovementComponent->Velocity = FVector::ZeroVector;
    }
    if (LadderStateTimeRemaining <= 0.0f) {
      // Animation has finished. Now switch to MOVE_Walking so the character
      // lands on the TopBlock surface. The TopBlock collision was already
      // restored in HandleCharacterExitedAtTop (TopFinish entry), so
      // FindFloor will detect the floor immediately.
      RestoreCharacterMovementFromLadder();
      if (UCharacterMovementComponent *MovementComponent =
              GetCharacterMovement()) {
        MovementComponent->SetMovementMode(MOVE_Walking);
        MovementComponent->Velocity = FVector::ZeroVector;
      }
      SetLadderState(EPlatformerLadderState::None);
    }
    return;

  default:
    return;
  }
}

void APlatformerCharacterBase::ApplyLadderVelocity() {
  if (!IsValid(ActiveLadder)) {
    return;
  }
  UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  if (!MovementComponent) {
    return;
  }

  float TargetZ = 0.0f;
  switch (LadderState) {
  case EPlatformerLadderState::Climbing:
    TargetZ = LadderClimbInput * ActiveLadder->GetClimbSpeed();
    break;
  case EPlatformerLadderState::JumpBoost:
    TargetZ = ActiveLadder->GetJumpClimbSpeed();
    break;
  case EPlatformerLadderState::CrouchBoost:
    TargetZ = -ActiveLadder->GetCrouchClimbSpeed();
    break;
  default:
    TargetZ = 0.0f;
    break;
  }

  // Lock X/Y; only Z is driven. The character's plane constraint keeps Y
  // locked, and zeroed X velocity keeps the body aligned with the snapped
  // ladder X.
  MovementComponent->Velocity = FVector(0.0f, 0.0f, TargetZ);
}

void APlatformerCharacterBase::SaveCharacterMovementForLadder() {
  UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  if (!MovementComponent) {
    return;
  }

  SavedPreLadderMovementMode = MovementComponent->MovementMode;
  SavedPreLadderGravityScale = MovementComponent->GravityScale;
  SavedPreLadderMaxFlySpeed = MovementComponent->MaxFlySpeed;

  if (USideViewMovementComponent *SideViewMovementComponent =
          Cast<USideViewMovementComponent>(MovementComponent)) {
    bHadPreLadderGravityOverride =
        SideViewMovementComponent->HasExternalGravityScaleOverride();
    PreLadderGravityOverride =
        SideViewMovementComponent->GetExternalGravityScaleOverride();
  } else {
    bHadPreLadderGravityOverride = false;
    PreLadderGravityOverride = SavedPreLadderGravityScale;
  }
}

void APlatformerCharacterBase::RestoreCharacterMovementFromLadder() {
  UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  if (!MovementComponent) {
    return;
  }

  if (USideViewMovementComponent *SideViewMovementComponent =
          Cast<USideViewMovementComponent>(MovementComponent)) {
    if (bHadPreLadderGravityOverride) {
      SideViewMovementComponent->SetExternalGravityScaleOverride(
          PreLadderGravityOverride);
    } else {
      SideViewMovementComponent->ClearExternalGravityScaleOverride();
    }
  } else {
    MovementComponent->GravityScale = SavedPreLadderGravityScale;
  }

  MovementComponent->MaxFlySpeed = SavedPreLadderMaxFlySpeed;
  MovementComponent->SetMovementMode(SavedPreLadderMovementMode);

  bHadPreLadderGravityOverride = false;
  PreLadderGravityOverride = SavedPreLadderGravityScale;
}

bool APlatformerCharacterBase::IsLadderReentryOnCooldown() const {
  return LadderReentryCooldownEndTime > GetWorldTimeSafe();
}

void APlatformerCharacterBase::ArmLadderInputReleaseGate() {
  bRequiresLadderInputRelease = true;
}

void APlatformerCharacterBase::TickLadderInputReleaseGate(
    float CurrentInputMagnitude) {
  if (!bRequiresLadderInputRelease) {
    return;
  }

  // Player must let the stick / key fall back through neutral before climb
  // input is honored again. Uses a tiny constant rather than the per-call
  // dead-zone to avoid races where dead-zone tuning lets the stick "stay
  // pressed" forever.
  constexpr float NeutralReleaseThreshold = 0.05f;
  if (CurrentInputMagnitude <= NeutralReleaseThreshold) {
    bRequiresLadderInputRelease = false;
  }
}

void APlatformerCharacterBase::ArmLadderReentryCooldown() {
  // ActiveLadder may already be cleared in the None-enter path; use the
  // current AvailableLadder as a fallback so we still get a sensible value.
  const APlatformerLadder *CooldownLadder =
      IsValid(ActiveLadder) ? ActiveLadder.Get() : AvailableLadder.Get();
  const float CooldownDuration =
      CooldownLadder
          ? FMath::Max(CooldownLadder->GetReentryCooldownDuration(), 0.0f)
          : 0.0f;
  if (CooldownDuration > 0.0f) {
    LadderReentryCooldownEndTime = GetWorldTimeSafe() + CooldownDuration;
  } else {
    LadderReentryCooldownEndTime = -1.0f;
  }
}

void APlatformerCharacterBase::SnapCharacterToLadderTopSurface(
    const APlatformerLadder *Ladder) {
  if (!IsValid(Ladder)) {
    return;
  }

  const float TargetFeetZ = Ladder->GetClimbTopWorldZ();
  const float CurrentFeetZ = GetCharacterFeetWorldZ();
  if (CurrentFeetZ < TargetFeetZ) {
    FVector CharacterLocation = GetActorLocation();
    CharacterLocation.Z += TargetFeetZ - CurrentFeetZ;
    SetActorLocation(CharacterLocation, false, nullptr,
                     ETeleportType::TeleportPhysics);
  }
}

void APlatformerCharacterBase::GetAvailableLedgeGrabs(
    TArray<APlatformerLedgeGrab *> &OutLedgeGrabs) const {
  OutLedgeGrabs.Reset();

  for (const TWeakObjectPtr<APlatformerLedgeGrab> &AvailableLedgeGrab :
       AvailableLedgeGrabs) {
    if (APlatformerLedgeGrab *ResolvedLedgeGrab = AvailableLedgeGrab.Get()) {
      OutLedgeGrabs.Add(ResolvedLedgeGrab);
    }
  }
}

void APlatformerCharacterBase::BeginPlay() {
  Super::BeginPlay();

  if (AbilitySystemComponent) {
    InitializeAbilities(DefaultAbilitySet);
  }

  // Link project-specific anim layer implementation if configured
  if (LinkedAnimLayerClass) {
    if (USkeletalMeshComponent *CharacterMesh = GetMesh()) {
      if (UAnimInstance *AnimInstance = CharacterMesh->GetAnimInstance()) {
        AnimInstance->LinkAnimClassLayers(LinkedAnimLayerClass);
      }
    }
  }

  LoadAndApplyDeveloperSettings();
  EnsureDeveloperJumpTrajectoryActor();
  RefreshJumpTrajectoryPreview();
}

void APlatformerCharacterBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Drop stale weak refs before the FSM consults them.
  if (AvailableLadder && !IsValid(AvailableLadder)) {
    AvailableLadder = nullptr;
  }

  // Single per-tick driver for the ladder state machine. Animation booleans
  // are derived from LadderState directly (see ShouldPlay*Animation getters),
  // so there is no separate animation tick.
  TickLadderState(DeltaTime);

  AvailableLedgeGrabs.RemoveAll(
      [](const TWeakObjectPtr<APlatformerLedgeGrab> &AvailableLedgeGrab) {
        return !AvailableLedgeGrab.IsValid();
      });

  if (bPrintDeveloperCharacterStateEveryTick) {
    PrintDeveloperCharacterState();
  }
}

void APlatformerCharacterBase::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  GetWorldTimerManager().ClearTimer(DeathSequenceTimerHandle);

  if (LadderState != EPlatformerLadderState::None) {
    SetLadderState(EPlatformerLadderState::None);
  }
  DestroyDeveloperJumpTrajectoryActor();
  Super::EndPlay(EndPlayReason);
}

void APlatformerCharacterBase::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Input binding lives in higher-level player/controller shells.
}

void APlatformerCharacterBase::FellOutOfWorld(const UDamageType &DamageType) {
  if (AttributeSet && AttributeSet->GetHealth() > 0.0f) {
    AttributeSet->SetHealth(0.0f);
    SyncCombatLifeStateFromAttributes();
  }
}

void APlatformerCharacterBase::OnCombatDeath(AActor *DamageInstigatorActor) {
  DisablePlayerInputForDeath();

  Super::OnCombatDeath(DamageInstigatorActor);

  if (LadderState != EPlatformerLadderState::None) {
    SetLadderState(EPlatformerLadderState::None);
  }

  if (USkeletalMeshComponent *CharacterMesh = GetMesh()) {
    CharacterMesh->SetHiddenInGame(false, true);
    CharacterMesh->SetVisibility(true, true);
  }

  if (UCapsuleComponent *CharacterCapsule = GetCapsuleComponent()) {
    CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }

  if (HealthWidgetComponent) {
    HealthWidgetComponent->SetVisibility(false);
  }

  StartCombatDeathSequence(DamageInstigatorActor);
}

void APlatformerCharacterBase::OnCombatRevived() {
  GetWorldTimerManager().ClearTimer(DeathSequenceTimerHandle);
  PendingDeathInstigatorActor.Reset();

  Super::OnCombatRevived();

  if (APlayerController *PlayerController = Cast<APlayerController>(GetController())) {
    EnableInput(PlayerController);
    PlayerController->SetIgnoreMoveInput(false);
    PlayerController->SetIgnoreLookInput(false);
  }

  if (USkeletalMeshComponent *CharacterMesh = GetMesh()) {
    CharacterMesh->SetHiddenInGame(false, true);
    CharacterMesh->SetVisibility(true, true);
  }

  if (UCapsuleComponent *CharacterCapsule = GetCapsuleComponent()) {
    CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  }

  if (HealthWidgetComponent) {
    RefreshHealthWidget();
  }
}

void APlatformerCharacterBase::HandleCombatDeathAftermath(AActor *DamageInstigatorActor) {
  PendingDeathInstigatorActor = DamageInstigatorActor;
}

void APlatformerCharacterBase::DisablePlayerInputForDeath() {
  if (APlayerController *PlayerController = Cast<APlayerController>(GetController())) {
    DisableInput(PlayerController);
    PlayerController->SetIgnoreMoveInput(true);
    PlayerController->SetIgnoreLookInput(true);
  }
}

float APlatformerCharacterBase::PlayCombatDeathAnimation() {
  constexpr float DefaultPlayRate = 1.0f;
  const FGameplayTag DeathAnimTag = PlatformerAnimGameplayTags::Anim_Combat_Death;

  if (USkeletalMeshComponent *CharacterMesh = GetMesh()) {
    if (UPlatformerAnimInstance *PlatformerAnimInstance =
            Cast<UPlatformerAnimInstance>(CharacterMesh->GetAnimInstance())) {
      const float Duration = PlatformerAnimInstance->PlayAbilityMontage(DeathAnimTag, DefaultPlayRate);
      if (Duration > 0.0f) {
        UE_LOG(LogTemp, Log,
               TEXT("PlayCombatDeathAnimation[%s]: tag '%s' played through PlatformerAnimInstance (duration=%.2fs)."),
               *GetName(), *DeathAnimTag.ToString(), Duration);
        return Duration;
      }
    }
  }

  if (AnimDataAsset) {
    if (const FPlatformerAbilityAnimEntry *DeathAnimEntry = AnimDataAsset->FindAnimEntry(DeathAnimTag)) {
      if (UAnimMontage *DeathMontage = DeathAnimEntry->Montage.Get()) {
        const float PlayRate = FMath::Max(DeathAnimEntry->DefaultPlayRate, KINDA_SMALL_NUMBER);
        const float Duration = PlayAnimMontage(DeathMontage, PlayRate);
        UE_LOG(LogTemp, Log,
               TEXT("PlayCombatDeathAnimation[%s]: tag '%s' -> montage '%s' (rate=%.2f, duration=%.2fs)."),
               *GetName(), *DeathAnimTag.ToString(), *DeathMontage->GetName(), PlayRate, Duration);
        return Duration;
      }
    }
  }

  UE_LOG(LogTemp, Warning,
         TEXT("PlayCombatDeathAnimation[%s]: no montage resolved for tag '%s'. Check character AnimDataAsset or PlatformerAnimInstance.AnimData."),
         *GetName(), *DeathAnimTag.ToString());
  return 0.0f;
}

void APlatformerCharacterBase::StartCombatDeathSequence(AActor *DamageInstigatorActor) {
  PendingDeathInstigatorActor = DamageInstigatorActor;
  GetWorldTimerManager().ClearTimer(DeathSequenceTimerHandle);

  const float AnimationDuration = PlayCombatDeathAnimation();
  const float TotalDelay = FMath::Max(AnimationDuration, 0.0f)
      + FMath::Max(DeathPostAnimationDelay, 0.0f);

  if (TotalDelay <= 0.0f) {
    FinishCombatDeathSequence();
    return;
  }

  GetWorldTimerManager().SetTimer(
      DeathSequenceTimerHandle,
      this,
      &APlatformerCharacterBase::FinishCombatDeathSequence,
      TotalDelay,
      false);
}

void APlatformerCharacterBase::FinishCombatDeathSequence() {
  GetWorldTimerManager().ClearTimer(DeathSequenceTimerHandle);

  BP_OnCombatDeath(PendingDeathInstigatorActor.Get());

  if (APlatformerPlayerControllerBase *PlatformerPlayerController =
          Cast<APlatformerPlayerControllerBase>(GetController())) {
    PlatformerPlayerController->HandleControlledCharacterDeath();
  }
}

void APlatformerCharacterBase::OnEnteredLadder(APlatformerLadder *Ladder) {}

void APlatformerCharacterBase::OnExitedLadder(APlatformerLadder *Ladder) {}

void APlatformerCharacterBase::SnapCharacterToActiveLadder() {
  // Called from SetLadderState(Mounting) only — at this point ActiveLadder is
  // set but LadderState may still be the previous value, so guard on the
  // pointer alone.
  if (!IsValid(ActiveLadder)) {
    return;
  }

  FVector CharacterLocation = GetActorLocation();
  if (ActiveLadder->ShouldSnapCharacterHorizontalToLadder()) {
    CharacterLocation.X = ActiveLadder->GetActorLocation().X;
  }

  if (ActiveLadder->ShouldSnapCharacterDepthToLadder()) {
    CharacterLocation.Y = ActiveLadder->GetActorLocation().Y;
  }

  if (ActiveLadder->ShouldSnapCharacterRotationToLadder()) {
    const FQuat CharacterLadderRotation =
        ActiveLadder->GetActorQuat() *
        ActiveLadder->GetCharacterLadderRotation().Quaternion();
    SetActorLocationAndRotation(CharacterLocation, CharacterLadderRotation,
                                false, nullptr, ETeleportType::TeleportPhysics);
    return;
  }

  SetActorLocation(CharacterLocation, false, nullptr,
                   ETeleportType::TeleportPhysics);
}

bool APlatformerCharacterBase::IsCharacterAtOrBelowLadderBottom(
    const APlatformerLadder *Ladder) const {
  if (!IsValid(Ladder)) {
    return false;
  }

  return GetCharacterFeetWorldZ() <=
         Ladder->GetClimbBottomWorldZ() + LadderBottomExitTolerance;
}

bool APlatformerCharacterBase::HasWalkableFloorClose(float MaxDistance) const {
  UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  if (!MovementComponent) {
    return false;
  }

  // FindFloor sweeps a small distance below the capsule. Even in MOVE_Flying
  // (used during climb) it returns a valid floor — we do not need to switch
  // movement mode just to query.
  FFindFloorResult FloorResult;
  MovementComponent->FindFloor(GetActorLocation(), FloorResult, false);

  if (!FloorResult.bWalkableFloor ||
      FloorResult.FloorDist > FMath::Max(MaxDistance, 0.0f)) {
    return false;
  }

  // Do not treat the active ladder actor itself as a "ground floor" exit
  // trigger. When descending through a LadderTop actor the top block sits
  // right at the character's feet (FloorDist ≈ 0), which would fire an
  // immediate bottom dismount before the character has descended at all.
  // FindFloor builds its own FCollisionQueryParams and ignores
  // MoveIgnoreActors, so we must filter the result here instead.
  if (IsValid(ActiveLadder) &&
      FloorResult.HitResult.GetActor() == ActiveLadder.Get()) {
    return false;
  }

  return true;
}

bool APlatformerCharacterBase::IsCharacterAtOrAboveLadderTop(
    const APlatformerLadder *Ladder) const {
  if (!IsValid(Ladder)) {
    return false;
  }

  return GetCharacterFeetWorldZ() >=
         Ladder->GetClimbTopWorldZ() - LadderBottomExitTolerance;
}

float APlatformerCharacterBase::GetCharacterFeetWorldZ() const {
  if (const UCapsuleComponent *CharacterCapsule = GetCapsuleComponent()) {
    return GetActorLocation().Z -
           CharacterCapsule->GetScaledCapsuleHalfHeight();
  }

  return GetActorLocation().Z;
}

float APlatformerCharacterBase::GetWorldTimeSafe() const {
  return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void APlatformerCharacterBase::ApplyDeveloperCameraSettings(
    const FDeveloperPlatformerCameraSettings &DeveloperCameraSettings) {
  if (CameraBoom) {
    CameraBoom->TargetArmLength =
        DeveloperCameraSettings.DeveloperSpringArmArmLength;
    CameraBoom->SetRelativeLocation(
        DeveloperCameraSettings.DeveloperSpringArmLocation);
    CameraBoom->SetRelativeRotation(
        FRotator(DeveloperCameraSettings.DeveloperSpringArmRotation.X,
                 DeveloperCameraSettings.DeveloperSpringArmRotation.Y,
                 DeveloperCameraSettings.DeveloperSpringArmRotation.Z));
  }

  if (FollowCamera) {
    FollowCamera->FieldOfView = DeveloperCameraSettings.DeveloperCameraFOV;
    FollowCamera->SetRelativeLocation(
        FVector(DeveloperCameraSettings.DeveloperCameraLocation.Y,
                DeveloperCameraSettings.DeveloperCameraLocation.X,
                DeveloperCameraSettings.DeveloperCameraLocation.Z));
    FollowCamera->SetRelativeRotation(
        FRotator(DeveloperCameraSettings.DeveloperCameraRotation.X,
                 DeveloperCameraSettings.DeveloperCameraRotation.Y,
                 DeveloperCameraSettings.DeveloperCameraRotation.Z));
  }
}

FDeveloperPlatformerCameraSettings
APlatformerCharacterBase::CaptureDeveloperCameraSettings() const {
  FDeveloperPlatformerCameraSettings DeveloperCameraSettings;

  if (CameraBoom) {
    DeveloperCameraSettings.DeveloperSpringArmArmLength =
        CameraBoom->TargetArmLength;
    DeveloperCameraSettings.DeveloperSpringArmLocation =
        CameraBoom->GetRelativeLocation();

    const FRotator SpringArmRotation = CameraBoom->GetRelativeRotation();
    DeveloperCameraSettings.DeveloperSpringArmRotation = FVector(
        SpringArmRotation.Pitch, SpringArmRotation.Yaw, SpringArmRotation.Roll);
  }

  if (FollowCamera) {
    DeveloperCameraSettings.DeveloperCameraFOV = FollowCamera->FieldOfView;
    const FVector CameraLocation = FollowCamera->GetRelativeLocation();
    DeveloperCameraSettings.DeveloperCameraLocation =
        FVector(CameraLocation.Y, CameraLocation.X, CameraLocation.Z);

    const FRotator CameraRotation = FollowCamera->GetRelativeRotation();
    DeveloperCameraSettings.DeveloperCameraRotation =
        FVector(CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);
  }

  return DeveloperCameraSettings;
}

void APlatformerCharacterBase::ApplyDeveloperCharacterMovementSettings(
    const FDeveloperPlatformerCharacterMovementSettings
        &DeveloperCharacterMovementSettings) {
  if (UCharacterMovementComponent *MovementComponent = GetCharacterMovement()) {
    MovementComponent->MaxWalkSpeed = FMath::Max(
        0.0f, DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed);
    MovementComponent->MaxFlySpeed = FMath::Max(
        0.0f, DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed);
    MovementComponent->MaxAcceleration = FMath::Max(
        0.0f,
        DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration);
    MovementComponent->BrakingDecelerationWalking =
        FMath::Max(0.0f, DeveloperCharacterMovementSettings
                             .DeveloperMovementBrakingDecelerationWalking);
    MovementComponent->JumpZVelocity = FMath::Max(
        0.0f,
        DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity);
    MovementComponent->Mass =
        FMath::Max(KINDA_SMALL_NUMBER,
                   DeveloperCharacterMovementSettings.DeveloperMovementMass);
    MovementComponent->BrakingFrictionFactor =
        FMath::Max(0.0f, DeveloperCharacterMovementSettings
                             .DeveloperMovementBrakingFrictionFactor);
    MovementComponent->GroundFriction = FMath::Max(
        0.0f,
        DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction);
    MovementComponent->AirControl = FMath::Max(
        0.0f, DeveloperCharacterMovementSettings.DeveloperMovementAirControl);

    if (USideViewMovementComponent *SideViewMovementComponent =
            Cast<USideViewMovementComponent>(MovementComponent)) {
      SideViewMovementComponent->JumpApexGravityMultiplier =
          FMath::Max(0.0f, DeveloperCharacterMovementSettings
                               .DeveloperMovementJumpApexGravityMultiplier);
      SideViewMovementComponent->SetChangeDirectionSpeed(
          FMath::Max(0.0f, DeveloperCharacterMovementSettings
                               .DeveloperMovementChangeDirectionSpeed));
      SideViewMovementComponent->SetBaseGravityScale(
          DeveloperCharacterMovementSettings.DeveloperMovementGravityScale);
    } else {
      MovementComponent->GravityScale = FMath::Max(
          0.0f,
          DeveloperCharacterMovementSettings.DeveloperMovementGravityScale);
    }
  }

  SetDeveloperCrouchCapsuleScaleOverride(
      DeveloperCharacterMovementSettings.DeveloperMovementCrouchCapsuleScale);
  SetDeveloperJumpHorizontalSpeedOverride(
      DeveloperCharacterMovementSettings.DeveloperMovementJumpHorizontalSpeed);
  SetShowJumpTrajectoryPreview(
      DeveloperCharacterMovementSettings.DeveloperMovementShowJumpTrajectory);
  RefreshJumpTrajectoryPreview();
}

FDeveloperPlatformerCharacterMovementSettings
APlatformerCharacterBase::CaptureDeveloperCharacterMovementSettings() const {
  FDeveloperPlatformerCharacterMovementSettings
      DeveloperCharacterMovementSettings;

  if (const UCharacterMovementComponent *MovementComponent =
          GetCharacterMovement()) {
    DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed =
        MovementComponent->MaxWalkSpeed;
    DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed =
        MovementComponent->MaxFlySpeed;
    DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration =
        MovementComponent->MaxAcceleration;
    DeveloperCharacterMovementSettings
        .DeveloperMovementBrakingDecelerationWalking =
        MovementComponent->BrakingDecelerationWalking;
    DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity =
        MovementComponent->JumpZVelocity;
    DeveloperCharacterMovementSettings.DeveloperMovementMass =
        MovementComponent->Mass;
    DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor =
        MovementComponent->BrakingFrictionFactor;
    DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction =
        MovementComponent->GroundFriction;
    DeveloperCharacterMovementSettings.DeveloperMovementAirControl =
        MovementComponent->AirControl;

    if (const USideViewMovementComponent *SideViewMovementComponent =
            Cast<USideViewMovementComponent>(MovementComponent)) {
      DeveloperCharacterMovementSettings
          .DeveloperMovementJumpApexGravityMultiplier =
          SideViewMovementComponent->JumpApexGravityMultiplier;
      DeveloperCharacterMovementSettings.DeveloperMovementChangeDirectionSpeed =
          SideViewMovementComponent->GetChangeDirectionSpeed();
      DeveloperCharacterMovementSettings.DeveloperMovementGravityScale =
          SideViewMovementComponent->GetBaseGravityScale();
    } else {
      DeveloperCharacterMovementSettings.DeveloperMovementGravityScale =
          MovementComponent->GravityScale;
    }
  }

  DeveloperCharacterMovementSettings.DeveloperMovementCrouchCapsuleScale =
      CaptureDeveloperCrouchCapsuleScale();
  DeveloperCharacterMovementSettings.DeveloperMovementJumpHorizontalSpeed =
      CaptureDeveloperJumpHorizontalSpeed();
  DeveloperCharacterMovementSettings.DeveloperMovementShowJumpTrajectory =
      bShowJumpTrajectoryPreview;

  return DeveloperCharacterMovementSettings;
}

void APlatformerCharacterBase::SetDeveloperCrouchCapsuleScaleOverride(
    float InCrouchCapsuleScale) {
  bHasDeveloperCrouchCapsuleScaleOverride = true;
  DeveloperCrouchCapsuleScaleOverride = FMath::Max(InCrouchCapsuleScale, 0.0f);
  ApplyResolvedCrouchCapsuleScale();
}

void APlatformerCharacterBase::ClearDeveloperCrouchCapsuleScaleOverride() {
  bHasDeveloperCrouchCapsuleScaleOverride = false;
  DeveloperCrouchCapsuleScaleOverride = 0.0f;
  ApplyResolvedCrouchCapsuleScale();
}

void APlatformerCharacterBase::SetDeveloperJumpHorizontalSpeedOverride(
    float InJumpHorizontalSpeed) {
  bHasDeveloperJumpHorizontalSpeedOverride = true;
  DeveloperJumpHorizontalSpeedOverride =
      FMath::Max(InJumpHorizontalSpeed, 0.0f);
}

void APlatformerCharacterBase::ClearDeveloperJumpHorizontalSpeedOverride() {
  bHasDeveloperJumpHorizontalSpeedOverride = false;
  DeveloperJumpHorizontalSpeedOverride = 0.0f;
}

float APlatformerCharacterBase::CaptureDeveloperCrouchCapsuleScale() const {
  if (bHasDeveloperCrouchCapsuleScaleOverride) {
    return DeveloperCrouchCapsuleScaleOverride;
  }

  return ResolveDefaultCrouchCapsuleScale();
}

void APlatformerCharacterBase::SetShowJumpTrajectoryPreview(
    bool bInShowJumpTrajectoryPreview) {
  bShowJumpTrajectoryPreview = bInShowJumpTrajectoryPreview;

  if (ADeveloperJumpTrajectory *JumpTrajectoryActor =
          EnsureDeveloperJumpTrajectoryActor()) {
    JumpTrajectoryActor->SetShowJumpTrajectoryPreview(
        bShowJumpTrajectoryPreview);
  }
}

float APlatformerCharacterBase::ResolveDefaultCrouchCapsuleScale() const {
  const ACharacter *DefaultCharacter =
      GetClass()->GetDefaultObject<ACharacter>();
  const UCharacterMovementComponent *DefaultMovementComponent =
      DefaultCharacter ? DefaultCharacter->GetCharacterMovement()
                       : GetCharacterMovement();
  const float StandingCapsuleHalfHeight = ResolveStandingCapsuleHalfHeight();
  if (!DefaultMovementComponent ||
      StandingCapsuleHalfHeight <= UE_KINDA_SMALL_NUMBER) {
    return 1.0f;
  }

  return FMath::Max(DefaultMovementComponent->GetCrouchedHalfHeight() /
                        StandingCapsuleHalfHeight,
                    0.0f);
}

float APlatformerCharacterBase::ResolveStandingCapsuleHalfHeight() const {
  const ACharacter *DefaultCharacter =
      GetClass()->GetDefaultObject<ACharacter>();
  const UCapsuleComponent *DefaultCapsuleComponent =
      DefaultCharacter ? DefaultCharacter->GetCapsuleComponent()
                       : GetCapsuleComponent();
  return DefaultCapsuleComponent
             ? FMath::Max(
                   DefaultCapsuleComponent->GetUnscaledCapsuleHalfHeight(),
                   0.0f)
             : 0.0f;
}

void APlatformerCharacterBase::ApplyResolvedCrouchCapsuleScale() {
  UCharacterMovementComponent *MovementComponent = GetCharacterMovement();
  UCapsuleComponent *CharacterCapsule = GetCapsuleComponent();
  if (!MovementComponent || !CharacterCapsule) {
    return;
  }

  const float StandingCapsuleHalfHeight = ResolveStandingCapsuleHalfHeight();
  if (StandingCapsuleHalfHeight <= UE_KINDA_SMALL_NUMBER) {
    return;
  }

  const float ResolvedCrouchCapsuleScale =
      ResolveDeveloperCrouchCapsuleScale(ResolveDefaultCrouchCapsuleScale());
  const float ResolvedCrouchedHalfHeight =
      FMath::Max(CharacterCapsule->GetUnscaledCapsuleRadius(),
                 StandingCapsuleHalfHeight * ResolvedCrouchCapsuleScale);
  MovementComponent->SetCrouchedHalfHeight(ResolvedCrouchedHalfHeight);

  if (bIsCrouched) {
    MovementComponent->Crouch(false);
  }
}

float APlatformerCharacterBase::CaptureDeveloperJumpHorizontalSpeed() const {
  if (bHasDeveloperJumpHorizontalSpeedOverride) {
    return DeveloperJumpHorizontalSpeedOverride;
  }

  if (const UGA_PlatformerJump *JumpAbility = FindGrantedJumpAbility()) {
    return JumpAbility->GetJumpHorizontalSpeed();
  }

  return 0.0f;
}

const UGA_PlatformerJump *
APlatformerCharacterBase::FindGrantedJumpAbility() const {
  if (!AbilitySystemComponent) {
    return nullptr;
  }

  const TArray<FGameplayAbilitySpec> &ActivatableAbilities =
      AbilitySystemComponent->GetActivatableAbilities();
  for (const FGameplayAbilitySpec &AbilitySpec : ActivatableAbilities) {
    if (const UGA_PlatformerJump *JumpAbility =
            Cast<UGA_PlatformerJump>(AbilitySpec.Ability)) {
      return JumpAbility;
    }
  }

  return nullptr;
}

void APlatformerCharacterBase::RefreshJumpTrajectoryPreview() {
  if (ADeveloperJumpTrajectory *JumpTrajectoryActor =
          EnsureDeveloperJumpTrajectoryActor()) {
    JumpTrajectoryActor->SetShowJumpTrajectoryPreview(
        bShowJumpTrajectoryPreview);
    JumpTrajectoryActor->RefreshTrajectoryPreview();
  }
}

ADeveloperJumpTrajectory *
APlatformerCharacterBase::SpawnJumpTrajectorySnapshotActor() {
  ADeveloperJumpTrajectory *JumpTrajectoryActor =
      EnsureDeveloperJumpTrajectoryActor();
  if (!JumpTrajectoryActor) {
    return nullptr;
  }

  JumpTrajectoryActor->SetShowJumpTrajectoryPreview(bShowJumpTrajectoryPreview);
  JumpTrajectoryActor->RefreshTrajectoryPreview();
  return JumpTrajectoryActor->SpawnSnapshotCopy();
}

ADeveloperJumpTrajectory *
APlatformerCharacterBase::EnsureDeveloperJumpTrajectoryActor() {
  if (IsValid(DeveloperJumpTrajectoryActor)) {
    return DeveloperJumpTrajectoryActor;
  }

  UWorld *World = GetWorld();
  if (!World || !World->IsGameWorld()) {
    return nullptr;
  }

  if (!GetCapsuleComponent()) {
    return nullptr;
  }

  UClass *JumpTrajectoryClass = DeveloperJumpTrajectoryClass.Get();
  if (!JumpTrajectoryClass) {
    JumpTrajectoryClass = ADeveloperJumpTrajectory::StaticClass();
  }

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.Owner = this;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  DeveloperJumpTrajectoryActor = World->SpawnActor<ADeveloperJumpTrajectory>(
      JumpTrajectoryClass, GetCapsuleComponent()->GetComponentTransform(),
      SpawnParameters);
  if (!DeveloperJumpTrajectoryActor) {
    return nullptr;
  }

  DeveloperJumpTrajectoryActor->AttachToComponent(
      GetCapsuleComponent(),
      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  DeveloperJumpTrajectoryActor->SetActorRelativeLocation(FVector::ZeroVector);
  DeveloperJumpTrajectoryActor->SetActorRelativeRotation(FRotator::ZeroRotator);
  DeveloperJumpTrajectoryActor->InitializeAttachedPreview(this);
  DeveloperJumpTrajectoryActor->SetShowJumpTrajectoryPreview(
      bShowJumpTrajectoryPreview);

  return DeveloperJumpTrajectoryActor;
}

void APlatformerCharacterBase::DestroyDeveloperJumpTrajectoryActor() {
  if (!IsValid(DeveloperJumpTrajectoryActor)) {
    return;
  }

  DeveloperJumpTrajectoryActor->Destroy();
  DeveloperJumpTrajectoryActor = nullptr;
}

void APlatformerCharacterBase::ApplyDeveloperCombatSettings(
    const FDeveloperPlatformerCombatSettings &DeveloperCombatSettings) {
  ActiveDeveloperCombatSettings = DeveloperCombatSettings;
  SetHasActiveDeveloperCombatSettings(true);

  if (!AbilitySystemComponent || !AttributeSet) {
    return;
  }

  const FGameplayAttribute DeveloperMaxHealthAttribute =
      UPlatformerCharacterAttributeSet::GetMaxHealthAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMaxHealthAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperMaxHealthAttribute,
        FMath::Max(DeveloperCombatSettings.DeveloperCombatMaxHealth, 1.0f));
  }

  const FGameplayAttribute DeveloperHealthAttribute =
      UPlatformerCharacterAttributeSet::GetHealthAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperHealthAttribute)) {
    const float ResolvedDeveloperMaxHealth =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperMaxHealthAttribute);
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperHealthAttribute,
        FMath::Clamp(DeveloperCombatSettings.DeveloperCombatCurrentHealth, 0.0f,
                     ResolvedDeveloperMaxHealth));
  }

  const FGameplayAttribute DeveloperMeleeAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetMeleeAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMeleeAttackDamageAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperMeleeAttackDamageAttribute,
        FMath::Max(0.0f,
                   DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage));
  }

  const FGameplayAttribute DeveloperMeleeAttackDelayAttribute =
      UPlatformerCharacterAttributeSet::GetMeleeAttackDelayAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMeleeAttackDelayAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperMeleeAttackDelayAttribute,
        FMath::Max(0.0f,
                   DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay));
  }

  const FGameplayAttribute DeveloperRangeBaseAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetRangeBaseAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeBaseAttackDamageAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperRangeBaseAttackDamageAttribute,
        FMath::Max(
            0.0f,
            DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage));
  }

  const FGameplayAttribute DeveloperRangeChargedAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetRangeChargedAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeChargedAttackDamageAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperRangeChargedAttackDamageAttribute,
        FMath::Max(
            0.0f,
            DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage));
  }

  const FGameplayAttribute DeveloperRangeAttackDelayAttribute =
      UPlatformerCharacterAttributeSet::GetRangeAttackDelayAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeAttackDelayAttribute)) {
    AbilitySystemComponent->SetNumericAttributeBase(
        DeveloperRangeAttackDelayAttribute,
        FMath::Max(0.0f,
                   DeveloperCombatSettings.DeveloperCombatRangeAttackDelay));
  }

  RefreshHealthWidget();
  SyncCombatLifeStateFromAttributes();
}

FDeveloperPlatformerCombatSettings
APlatformerCharacterBase::CaptureDeveloperCombatSettings() const {
  FDeveloperPlatformerCombatSettings DeveloperCombatSettings =
      ActiveDeveloperCombatSettings;

  if (!AbilitySystemComponent || !AttributeSet) {
    return DeveloperCombatSettings;
  }

  const FGameplayAttribute DeveloperMaxHealthAttribute =
      UPlatformerCharacterAttributeSet::GetMaxHealthAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMaxHealthAttribute)) {
    DeveloperCombatSettings.DeveloperCombatMaxHealth =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperMaxHealthAttribute);
  } else {
    DeveloperCombatSettings.DeveloperCombatMaxHealth =
        AttributeSet->GetMaxHealth();
  }

  const FGameplayAttribute DeveloperHealthAttribute =
      UPlatformerCharacterAttributeSet::GetHealthAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperHealthAttribute)) {
    DeveloperCombatSettings.DeveloperCombatCurrentHealth =
        AbilitySystemComponent->GetNumericAttribute(DeveloperHealthAttribute);
  } else {
    DeveloperCombatSettings.DeveloperCombatCurrentHealth =
        AttributeSet->GetHealth();
  }

  const FGameplayAttribute DeveloperMeleeAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetMeleeAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMeleeAttackDamageAttribute)) {
    DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperMeleeAttackDamageAttribute);
  }

  const FGameplayAttribute DeveloperMeleeAttackDelayAttribute =
      UPlatformerCharacterAttributeSet::GetMeleeAttackDelayAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperMeleeAttackDelayAttribute)) {
    DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperMeleeAttackDelayAttribute);
  }

  const FGameplayAttribute DeveloperRangeBaseAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetRangeBaseAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeBaseAttackDamageAttribute)) {
    DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperRangeBaseAttackDamageAttribute);
  }

  const FGameplayAttribute DeveloperRangeChargedAttackDamageAttribute =
      UPlatformerCharacterAttributeSet::GetRangeChargedAttackDamageAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeChargedAttackDamageAttribute)) {
    DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperRangeChargedAttackDamageAttribute);
  }

  const FGameplayAttribute DeveloperRangeAttackDelayAttribute =
      UPlatformerCharacterAttributeSet::GetRangeAttackDelayAttribute();
  if (AbilitySystemComponent->HasAttributeSetForAttribute(
          DeveloperRangeAttackDelayAttribute)) {
    DeveloperCombatSettings.DeveloperCombatRangeAttackDelay =
        AbilitySystemComponent->GetNumericAttribute(
            DeveloperRangeAttackDelayAttribute);
  }

  return DeveloperCombatSettings;
}

FDeveloperPlatformerCombatSettings
APlatformerCharacterBase::ResolveDeveloperCombatSettingsForApplication(
    const FDeveloperPlatformerCombatSettings &DeveloperCombatSettings) const {
  FDeveloperPlatformerCombatSettings ResolvedDeveloperCombatSettings =
      DeveloperCombatSettings;
  const bool bUsesLegacyCombatDefaults =
      FMath::IsNearlyEqual(
          ResolvedDeveloperCombatSettings.DeveloperCombatMaxHealth, 10.0f) &&
      FMath::IsNearlyEqual(
          ResolvedDeveloperCombatSettings.DeveloperCombatCurrentHealth,
          10.0f) &&
      FMath::IsNearlyEqual(
          ResolvedDeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage,
          1.0f) &&
      FMath::IsNearlyEqual(ResolvedDeveloperCombatSettings
                               .DeveloperCombatRangeChargedAttackDamage,
                           1.0f);

  if (bUsesLegacyCombatDefaults) {
    ResolvedDeveloperCombatSettings.DeveloperCombatMaxHealth = 100.0f;
    ResolvedDeveloperCombatSettings.DeveloperCombatCurrentHealth = 100.0f;
    ResolvedDeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage =
        25.0f;
    ResolvedDeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage =
        75.0f;
  }

  return ResolvedDeveloperCombatSettings;
}

void APlatformerCharacterBase::LoadAndApplyDeveloperSettings() {
  if (UGameInstance *GameInstance = GetGameInstance()) {
    if (UPlatformerDeveloperSettingsSubsystem *DeveloperSettingsSubsystem =
            GameInstance
                ->GetSubsystem<UPlatformerDeveloperSettingsSubsystem>()) {
      FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
      if (DeveloperSettingsSubsystem->TryLoadCurrentSnapshot(
              DeveloperSettingsSnapshot)) {
        ApplyDeveloperSettingsSnapshot(DeveloperSettingsSnapshot);
        return;
      }
    }
  }

  SetHasActiveDeveloperCombatSettings(false);
  ActiveDeveloperCombatSettings = FDeveloperPlatformerCombatSettings();
}

void APlatformerCharacterBase::SetHasActiveDeveloperCombatSettings(
    bool bInHasActiveDeveloperCombatSettings) {
  bHasActiveDeveloperCombatSettings = bInHasActiveDeveloperCombatSettings;
}

bool APlatformerCharacterBase::HasActiveDeveloperCombatSettings() const {
  return bHasActiveDeveloperCombatSettings;
}

const FDeveloperPlatformerCombatSettings &
APlatformerCharacterBase::GetActiveDeveloperCombatSettings() const {
  return ActiveDeveloperCombatSettings;
}

float APlatformerCharacterBase::GetHealthWidgetVerticalPadding() const {
  return PlatformerHealthWidgetVerticalPadding;
}
