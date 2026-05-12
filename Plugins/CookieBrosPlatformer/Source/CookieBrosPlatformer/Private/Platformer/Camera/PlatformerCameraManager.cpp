// Copyright Epic Games, Inc. All Rights Reserved.
//
#include "Platformer/Camera/PlatformerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Character/PlatformerCharacterBase.h"
#include "Core/PlatformerDeveloperSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"

void APlatformerCameraManager::BeginPlay() {
  Super::BeginPlay();
  LoadAndApplyDeveloperCameraManagerSettings();
}

void APlatformerCameraManager::ResolveBaseCameraPose(const APawn &TargetPawn,
                                                     FVector &OutLocation,
                                                     FRotator &OutRotation,
                                                     float &OutFOV) const {
  OutLocation =
      TargetPawn.GetActorLocation() + FVector(0.0f, CurrentZoom, CameraZOffset);
  OutRotation = FallbackRotation;
  OutFOV = 65.0f;

  if (const UCameraComponent *CameraComp =
          TargetPawn.FindComponentByClass<UCameraComponent>()) {
    OutLocation = CameraComp->GetComponentLocation();
    OutRotation = CameraComp->GetComponentRotation();
    OutFOV = CameraComp->FieldOfView;
    return;
  }

  if (const USpringArmComponent *SpringArm =
          TargetPawn.FindComponentByClass<USpringArmComponent>()) {
    OutLocation = SpringArm->GetSocketLocation(USpringArmComponent::SocketName);
    OutRotation = SpringArm->GetSocketRotation(USpringArmComponent::SocketName);
  }
}

FVector APlatformerCameraManager::ResolveCameraFocusLocation(
    const APawn &TargetPawn) const {
  if (const APlatformerCharacterBase *PlatformerCharacter =
          Cast<APlatformerCharacterBase>(&TargetPawn)) {
    return PlatformerCharacter->GetPlatformerCameraFocusLocation();
  }

  return TargetPawn.GetActorLocation();
}

bool APlatformerCameraManager::IsOrthographicProjectionEnabled() const {
  return bIsOrthographic ||
         CameraProjectionMode == EPlatformerCameraProjectionMode::Orthographic;
}

void APlatformerCameraManager::SyncProjectionSettings(APawn &TargetPawn) {
  const bool bUseOrthographicProjection = IsOrthographicProjectionEnabled();
  bIsOrthographic = bUseOrthographicProjection;
  CameraProjectionMode = bUseOrthographicProjection
                             ? EPlatformerCameraProjectionMode::Orthographic
                             : EPlatformerCameraProjectionMode::Perspective;
  DefaultOrthoWidth = OrthographicWidth;

  if (UCameraComponent *CameraComponent =
          TargetPawn.FindComponentByClass<UCameraComponent>()) {
    CameraComponent->SetProjectionMode(
        bUseOrthographicProjection ? ECameraProjectionMode::Orthographic
                                   : ECameraProjectionMode::Perspective);
    CameraComponent->SetOrthoWidth(OrthographicWidth);
  }
}

FVector2D APlatformerCameraManager::ResolveMovementDirection2D(
    const FVector &Velocity) const {
  const FVector2D MovementInPlane(Velocity.X, Velocity.Z);
  return MovementInPlane.GetSafeNormal();
}

void APlatformerCameraManager::ApplyDeveloperCameraManagerSettings(
    const FDeveloperPlatformerCameraManagerSettings
        &DeveloperCameraManagerSettings) {
  CameraProjectionMode =
      DeveloperCameraManagerSettings.DeveloperCameraProjectionMode;
  bIsOrthographic =
      CameraProjectionMode == EPlatformerCameraProjectionMode::Orthographic;
  OrthographicWidth = FMath::Max(
      1.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerOrthographicWidth);
  DefaultOrthoWidth = OrthographicWidth;
  IdleSpeedThreshold = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold *
          100.0f);
  HorizontalOffset = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset);
  HorizontalOffsetInterpSpeedStart = FMath::Max(
      0.0f, DeveloperCameraManagerSettings
                .DeveloperCameraManagerHorizontalOffsetInterpSpeedStart);
  HorizontalOffsetInterpSpeedEnd = FMath::Max(
      0.0f, DeveloperCameraManagerSettings
                .DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd);
  VerticalOffset = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset);
  VerticalOffsetInterpSpeed =
      FMath::Max(0.0f, DeveloperCameraManagerSettings
                           .DeveloperCameraManagerVerticalOffsetInterpSpeed);
  CameraDeadZoneWidth = FMath::Max(
      0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerDeadZoneWidth);
  CameraDeadZoneHeight = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerDeadZoneHeight);
  CameraBoundBoxWidth = FMath::Max(
      0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerBoundBoxWidth);
  CameraBoundBoxHeight = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerBoundBoxHeight);
  CrouchInterpSpeed = FMath::Max(
      0.0f,
      DeveloperCameraManagerSettings.DeveloperCameraManagerCrouchInterpSpeed);

  CurrentHorizontalOffset = FMath::Clamp(CurrentHorizontalOffset,
                                         -HorizontalOffset, HorizontalOffset);
  CurrentVerticalOffset =
      FMath::Clamp(CurrentVerticalOffset, -VerticalOffset, VerticalOffset);
}

FDeveloperPlatformerCameraManagerSettings
APlatformerCameraManager::CaptureDeveloperCameraManagerSettings() const {
  FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings;
  DeveloperCameraManagerSettings.DeveloperCameraProjectionMode =
      IsOrthographicProjectionEnabled()
          ? EPlatformerCameraProjectionMode::Orthographic
          : EPlatformerCameraProjectionMode::Perspective;
  DeveloperCameraManagerSettings.DeveloperCameraManagerOrthographicWidth =
      OrthographicWidth;
  DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold =
      IdleSpeedThreshold / 100.0f;
  DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset =
      HorizontalOffset;
  DeveloperCameraManagerSettings
      .DeveloperCameraManagerHorizontalOffsetInterpSpeedStart =
      HorizontalOffsetInterpSpeedStart;
  DeveloperCameraManagerSettings
      .DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd =
      HorizontalOffsetInterpSpeedEnd;
  DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset =
      VerticalOffset;
  DeveloperCameraManagerSettings
      .DeveloperCameraManagerVerticalOffsetInterpSpeed =
      VerticalOffsetInterpSpeed;
  DeveloperCameraManagerSettings.DeveloperCameraManagerDeadZoneWidth =
      CameraDeadZoneWidth;
  DeveloperCameraManagerSettings.DeveloperCameraManagerDeadZoneHeight =
      CameraDeadZoneHeight;
  DeveloperCameraManagerSettings.DeveloperCameraManagerBoundBoxWidth =
      CameraBoundBoxWidth;
  DeveloperCameraManagerSettings.DeveloperCameraManagerBoundBoxHeight =
      CameraBoundBoxHeight;
  DeveloperCameraManagerSettings.DeveloperCameraManagerCrouchInterpSpeed =
      CrouchInterpSpeed;
  return DeveloperCameraManagerSettings;
}

void APlatformerCameraManager::LoadAndApplyDeveloperCameraManagerSettings() {
  if (UGameInstance *GameInstance = GetGameInstance()) {
    if (UPlatformerDeveloperSettingsSubsystem *DeveloperSettingsSubsystem =
            GameInstance
                ->GetSubsystem<UPlatformerDeveloperSettingsSubsystem>()) {
      FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
      if (DeveloperSettingsSubsystem->TryLoadCurrentSnapshot(
              DeveloperSettingsSnapshot)) {
        if (DeveloperSettingsSnapshot.bHasSavedCameraManagerSettings) {
          ApplyDeveloperCameraManagerSettings(
              DeveloperSettingsSnapshot.CameraManagerSettings);
        }
      }
    }
  }
}

void APlatformerCameraManager::UpdateViewTarget(FTViewTarget &OutVT,
                                                float DeltaTime) {
  APawn *TargetPawn = Cast<APawn>(OutVT.Target);
  if (!IsValid(TargetPawn)) {
    Super::UpdateViewTarget(OutVT, DeltaTime);
    return;
  }

  SyncProjectionSettings(*TargetPawn);
  Super::UpdateViewTarget(OutVT, DeltaTime);

  const FVector RawCameraFocusLocation =
      ResolveCameraFocusLocation(*TargetPawn);
  FVector CameraFocusLocation = RawCameraFocusLocation;

  // Compute TargetCameraRigOffset BEFORE artificial Z-smoothing to avoid
  // phantom spikes! (Since OutVT.POV.Location instantly changes when the pawn
  // capsule height changes)
  const FVector TargetCameraRigOffset =
      OutVT.POV.Location - RawCameraFocusLocation;

  // --- Detect pawn change and reinitialize ---
  const bool bIsNewTargetPawn =
      !bIsDeadZoneInitialized || DeadZoneTrackedPawn.Get() != TargetPawn;
  if (bIsNewTargetPawn) {
    DeadZoneTrackedPawn = TargetPawn;
    DeadZoneCenter = CameraFocusLocation;
    CurrentDeadZoneOffset = FVector::ZeroVector;
    SmoothedFocusLocationZ = CameraFocusLocation.Z;
    CameraFollowState = EPlatformerCameraFollowState::DeadZone;
    bIsDeadZoneInitialized = true;
  } else {
    // Smooth vertical focus to prevent Z-axis snap on crouch/stand.
    SmoothedFocusLocationZ =
        FMath::FInterpTo(SmoothedFocusLocationZ, CameraFocusLocation.Z,
                         DeltaTime, CrouchInterpSpeed);
  }
  CameraFocusLocation.Z = SmoothedFocusLocationZ;

  // --- Camera rig offset (SpringArm / Camera component pose) ---
  if (!bHasCameraRigOffset || bIsNewTargetPawn) {
    CurrentCameraRigOffset = TargetCameraRigOffset;
    bHasCameraRigOffset = true;
  } else {
    const float HorizontalRigInterpSpeed =
        FMath::Max(0.0f, HorizontalOffsetInterpSpeedStart * 0.7f);
    CurrentCameraRigOffset.X =
        FMath::FInterpTo(CurrentCameraRigOffset.X, TargetCameraRigOffset.X,
                         DeltaTime, HorizontalRigInterpSpeed);
    CurrentCameraRigOffset.Y =
        FMath::FInterpTo(CurrentCameraRigOffset.Y, TargetCameraRigOffset.Y,
                         DeltaTime, HorizontalRigInterpSpeed);
    CurrentCameraRigOffset.Z =
        FMath::FInterpTo(CurrentCameraRigOffset.Z, TargetCameraRigOffset.Z,
                         DeltaTime, VerticalOffsetInterpSpeed);
  }

  const FVector Velocity = TargetPawn->GetVelocity();
  const bool bIsIdle = Velocity.IsNearlyZero(IdleSpeedThreshold);
  const bool bHasDeadZone = CameraDeadZoneWidth > KINDA_SMALL_NUMBER ||
                            CameraDeadZoneHeight > KINDA_SMALL_NUMBER;

  // --- State machine (active only when dead zone is configured) ---
  if (bHasDeadZone) {
    const float HalfWidth = CameraDeadZoneWidth * 0.5f;
    const float HalfHeight = CameraDeadZoneHeight * 0.5f;

    if (CameraFollowState == EPlatformerCameraFollowState::DeadZone) {
      const FVector Delta = CameraFocusLocation - DeadZoneCenter;
      const bool bPastHorizontal =
          CameraDeadZoneWidth > KINDA_SMALL_NUMBER &&
          FMath::Abs(Delta.X) > HalfWidth - KINDA_SMALL_NUMBER;
      const bool bPastVertical =
          CameraDeadZoneHeight > KINDA_SMALL_NUMBER &&
          FMath::Abs(Delta.Z) > HalfHeight - KINDA_SMALL_NUMBER;

      if (bPastHorizontal || bPastVertical) {
        CameraFollowState = EPlatformerCameraFollowState::Following;
      }
    } else // Following
    {
      if (bIsIdle) {
        CameraFollowState = EPlatformerCameraFollowState::DeadZone;
        // Do NOT snap DeadZoneCenter here to preserve the current offset
        // preventing jerks
      }
    }
  }

  // --- Dead zone logic ---
  if (bHasDeadZone) {
    if (CameraFollowState == EPlatformerCameraFollowState::DeadZone) {
      const float HalfWidth = CameraDeadZoneWidth * 0.5f;
      const float HalfHeight = CameraDeadZoneHeight * 0.5f;

      // 1. Rigid Bounding Box: Guarantee the character can never escape the
      // dead zone rectangle!
      if (CameraDeadZoneWidth > KINDA_SMALL_NUMBER) {
        if (CameraFocusLocation.X > DeadZoneCenter.X + HalfWidth)
          DeadZoneCenter.X = CameraFocusLocation.X - HalfWidth;
        if (CameraFocusLocation.X < DeadZoneCenter.X - HalfWidth)
          DeadZoneCenter.X = CameraFocusLocation.X + HalfWidth;
      }
      if (CameraDeadZoneHeight > KINDA_SMALL_NUMBER) {
        if (CameraFocusLocation.Z > DeadZoneCenter.Z + HalfHeight)
          DeadZoneCenter.Z = CameraFocusLocation.Z - HalfHeight;
        if (CameraFocusLocation.Z < DeadZoneCenter.Z - HalfHeight)
          DeadZoneCenter.Z = CameraFocusLocation.Z + HalfHeight;
      }

      // 2. Derive offset from the bounded DeadZoneCenter
      CurrentDeadZoneOffset.X = (CameraDeadZoneWidth > KINDA_SMALL_NUMBER)
                                    ? (CameraFocusLocation.X - DeadZoneCenter.X)
                                    : 0.0f;
      CurrentDeadZoneOffset.Z = (CameraDeadZoneHeight > KINDA_SMALL_NUMBER)
                                    ? (CameraFocusLocation.Z - DeadZoneCenter.Z)
                                    : 0.0f;
    } else // Following
    {
      // 1. Horizontal: Smoothly catch up to the character (collapse offset to
      // 0)
      CurrentDeadZoneOffset.X =
          FMath::FInterpTo(CurrentDeadZoneOffset.X, 0.0f, DeltaTime,
                           HorizontalOffsetInterpSpeedStart);
      DeadZoneCenter.X = CameraFocusLocation.X - CurrentDeadZoneOffset.X;

      // 2. Vertical: Do NOT collapse to 0 just because we are running
      // horizontally. Keep standard rigid dead zone behavior for Z, so
      // crouching/dashing doesn't forcibly dip the camera.
      CurrentDeadZoneOffset.Z = (CameraDeadZoneHeight > KINDA_SMALL_NUMBER)
                                    ? (CameraFocusLocation.Z - DeadZoneCenter.Z)
                                    : 0.0f;
    }
  } else {
    CurrentDeadZoneOffset = FVector::ZeroVector;
  }
  CurrentDeadZoneOffset.Y = 0.0f;

  // --- Look-ahead offset (only in Following state or when no dead zone) ---
  float TargetHorizontalLookAhead = 0.0f;
  float TargetVerticalLookAhead = 0.0f;

  const bool bApplyLookAhead =
      !bIsIdle &&
      (!bHasDeadZone ||
       CameraFollowState == EPlatformerCameraFollowState::Following);
  if (bApplyLookAhead) {
    const FVector2D MovementDirection = ResolveMovementDirection2D(Velocity);
    TargetHorizontalLookAhead = MovementDirection.X * HorizontalOffset;
    TargetVerticalLookAhead = MovementDirection.Y * VerticalOffset;
  }

  const float HorizontalInterpSpeed =
      FMath::IsNearlyZero(TargetHorizontalLookAhead, KINDA_SMALL_NUMBER)
          ? HorizontalOffsetInterpSpeedEnd
          : HorizontalOffsetInterpSpeedStart;
  CurrentHorizontalOffset =
      FMath::FInterpTo(CurrentHorizontalOffset, TargetHorizontalLookAhead,
                       DeltaTime, HorizontalInterpSpeed);
  CurrentVerticalOffset =
      FMath::FInterpTo(CurrentVerticalOffset, TargetVerticalLookAhead,
                       DeltaTime, VerticalOffsetInterpSpeed);

  // --- Bound Box Hard Limits ---
  // Ensures the character's visual offset (DeadZoneOffset - LookAhead) never
  // exceeds the BoundBox. This acts as a rigid failsafe for extremely high
  // speeds, preventing the character from leaving the screen.
  if (CameraBoundBoxWidth > KINDA_SMALL_NUMBER) {
    const float HalfBoundWidth = CameraBoundBoxWidth * 0.5f;
    const float CharScreenX = CurrentDeadZoneOffset.X - CurrentHorizontalOffset;
    if (CharScreenX > HalfBoundWidth) {
      const float Diff = CharScreenX - HalfBoundWidth;
      CurrentDeadZoneOffset.X -= Diff;
      DeadZoneCenter.X += Diff;
    } else if (CharScreenX < -HalfBoundWidth) {
      const float Diff = CharScreenX - (-HalfBoundWidth);
      CurrentDeadZoneOffset.X -= Diff;
      DeadZoneCenter.X += Diff;
    }
  }

  if (CameraBoundBoxHeight > KINDA_SMALL_NUMBER) {
    const float HalfBoundHeight = CameraBoundBoxHeight * 0.5f;
    // Include the FInterpTo steady-state lag of the Z-focus into the real
    // visual offset!
    const float TrueScreenZ =
        (RawCameraFocusLocation.Z - CameraFocusLocation.Z) +
        CurrentDeadZoneOffset.Z - CurrentVerticalOffset;
    if (TrueScreenZ > HalfBoundHeight) {
      const float Diff = TrueScreenZ - HalfBoundHeight;
      SmoothedFocusLocationZ += Diff;
      CameraFocusLocation.Z += Diff;
      DeadZoneCenter.Z += Diff;
    } else if (TrueScreenZ < -HalfBoundHeight) {
      const float Diff = TrueScreenZ - (-HalfBoundHeight);
      SmoothedFocusLocationZ += Diff;
      CameraFocusLocation.Z += Diff;
      DeadZoneCenter.Z += Diff;
    }
  }

  OutVT.POV.Location =
      CameraFocusLocation - CurrentDeadZoneOffset + CurrentCameraRigOffset +
      FVector(CurrentHorizontalOffset, 0.0f, CurrentVerticalOffset);
  OutVT.POV.ProjectionMode = bIsOrthographic
                                 ? ECameraProjectionMode::Orthographic
                                 : ECameraProjectionMode::Perspective;
  OutVT.POV.OrthoWidth = OrthographicWidth;
}
