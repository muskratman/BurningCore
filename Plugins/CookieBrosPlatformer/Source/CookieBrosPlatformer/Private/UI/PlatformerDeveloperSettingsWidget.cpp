#include "UI/PlatformerDeveloperSettingsWidget.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Core/PlatformerDeveloperSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Platformer/Camera/PlatformerCameraManager.h"
#include "UI/DeveloperCheckboxWidget.h"
#include "UI/DeveloperParameterWidget.h"
#include "UI/DeveloperVectorWidget.h"

namespace
{
float ResolveDeveloperPartialAttackDamage(
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings,
	const FPlatformerChargeShotTuning& DeveloperChargeShotSettings)
{
	const float BaseAttackDamage = FMath::Max(DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage, 0.0f);
	const float FullChargeDamage = FMath::Max(DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage, 0.0f);
	const float RawPartialDamage = BaseAttackDamage * FMath::Max(DeveloperChargeShotSettings.PartialDamageMultiplier, 1.0f);
	if (FullChargeDamage > 0.0f)
	{
		return FMath::Clamp(
			RawPartialDamage,
			BaseAttackDamage,
			FMath::Max(BaseAttackDamage, FullChargeDamage - KINDA_SMALL_NUMBER));
	}

	return RawPartialDamage;
}

float ResolveDeveloperPartialAttackSpeed(
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings,
	const FPlatformerChargeShotTuning& DeveloperChargeShotSettings)
{
	const float ChargedAttackSpeed = FMath::Max(DeveloperCombatSettings.DeveloperCombatRangeChargedAttackSpeed, 0.0f);
	return ChargedAttackSpeed * FMath::Max(DeveloperChargeShotSettings.PartialProjectileSpeedMultiplier, 0.0f);
}

float ResolveDeveloperPartialDamageMultiplierFromDisplay(
	float PartialAttackDamage,
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings,
	const FPlatformerChargeShotTuning& BaseDeveloperChargeShotSettings)
{
	const float BaseAttackDamage = FMath::Max(DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage, 0.0f);
	if (BaseAttackDamage <= KINDA_SMALL_NUMBER)
	{
		return BaseDeveloperChargeShotSettings.PartialDamageMultiplier;
	}

	return FMath::Max(PartialAttackDamage / BaseAttackDamage, 1.0f);
}

float ResolveDeveloperPartialSpeedMultiplierFromDisplay(
	float PartialAttackSpeed,
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings,
	const FPlatformerChargeShotTuning& BaseDeveloperChargeShotSettings)
{
	const float ChargedAttackSpeed = FMath::Max(DeveloperCombatSettings.DeveloperCombatRangeChargedAttackSpeed, 0.0f);
	if (ChargedAttackSpeed <= KINDA_SMALL_NUMBER)
	{
		return BaseDeveloperChargeShotSettings.PartialProjectileSpeedMultiplier;
	}

	return FMath::Max(PartialAttackSpeed / ChargedAttackSpeed, 0.0f);
}
}

void UPlatformerDeveloperSettingsWidget::RefreshDeveloperSettingsWidget()
{
	if (UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem())
	{
		FPlatformerDeveloperSettingsSnapshot SavedSnapshot;
		if (DeveloperSettingsSubsystem->TryLoadCurrentSnapshot(SavedSnapshot))
		{
			WorkingCopy = SavedSnapshot;
		}
		else
		{
			WorkingCopy = CaptureDeveloperSettingsSnapshotFromRuntime();
		}
	}
	else
	{
		WorkingCopy = CaptureDeveloperSettingsSnapshotFromRuntime();
	}

	LoadDeveloperSettingsSnapshotIntoWidgets(WorkingCopy);
	RefreshDeveloperSlotWidgets();
}

void UPlatformerDeveloperSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Common_AutoRestartLevel)
	{
		Common_AutoRestartLevel->SetParameterName(INVTEXT("Auto Restart Level"));
	}

	RefreshDeveloperSettingsWidget();
}

void UPlatformerDeveloperSettingsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Butt_Save)
	{
		Butt_Save->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveClicked);
	}

	if (Butt_SaveAs)
	{
		Butt_SaveAs->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveAsClicked);
	}

	if (Butt_Load)
	{
		Butt_Load->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperLoadClicked);
	}

	if (Butt_Delete)
	{
		Butt_Delete->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperDeleteClicked);
	}

	if (Butt_Close)
	{
		Butt_Close->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperCloseClicked);
	}

	if (Combo_Slots)
	{
		Combo_Slots->OnSelectionChanged.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperSlotSelectionChanged);
	}

	if (Editable_SlotName)
	{
		Editable_SlotName->OnTextChanged.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperSlotNameChanged);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperSettingsSnapshotIntoWidgets(
	const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot)
{
	if (Common_AutoRestartLevel)
	{
		Common_AutoRestartLevel->SetCheckboxValue(DeveloperSettingsSnapshot.bAutoRestartLevel);
	}

	FDeveloperPlatformerCharacterSettings ResolvedCharacterSettings = DeveloperSettingsSnapshot.CharacterSettings;
	const bool bNeedsRuntimeCharacterFallback =
		!DeveloperSettingsSnapshot.bHasSavedCombatSettings
		|| !DeveloperSettingsSnapshot.bHasSavedChargeShotSettings
		|| !DeveloperSettingsSnapshot.bHasSavedTraversalSettings;

	if (bNeedsRuntimeCharacterFallback)
	{
		const FDeveloperPlatformerCharacterSettings RuntimeCharacterSettings = CaptureDeveloperSettingsSnapshotFromRuntime().CharacterSettings;

		if (!DeveloperSettingsSnapshot.bHasSavedCombatSettings)
		{
			ResolvedCharacterSettings.DeveloperCombatSettings = RuntimeCharacterSettings.DeveloperCombatSettings;
		}

		if (!DeveloperSettingsSnapshot.bHasSavedChargeShotSettings)
		{
			ResolvedCharacterSettings.DeveloperChargeShotSettings = RuntimeCharacterSettings.DeveloperChargeShotSettings;
		}

		if (!DeveloperSettingsSnapshot.bHasSavedTraversalSettings)
		{
			ResolvedCharacterSettings.DeveloperLedgeSettings = RuntimeCharacterSettings.DeveloperLedgeSettings;
			ResolvedCharacterSettings.DeveloperSlideDashSettings = RuntimeCharacterSettings.DeveloperSlideDashSettings;
			ResolvedCharacterSettings.DeveloperWallSettings = RuntimeCharacterSettings.DeveloperWallSettings;
		}
	}

	WorkingCopy.CharacterSettings = ResolvedCharacterSettings;
	LoadDeveloperCharacterSettingsIntoWidgets(ResolvedCharacterSettings);
	LoadDeveloperCameraManagerSettingsIntoWidgets(DeveloperSettingsSnapshot.CameraManagerSettings);
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperCharacterSettingsIntoWidgets(const FDeveloperPlatformerCharacterSettings& DeveloperSettings)
{
	if (SpringArm_ArmLength)
	{
		SpringArm_ArmLength->SetParameterValue(DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmArmLength);
	}

	if (SpringArm_Location)
	{
		SpringArm_Location->SetVectorValue(DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmLocation);
	}

	if (SpringArm_Rotation)
	{
		SpringArm_Rotation->SetVectorValue(DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmRotation);
	}

	if (Camera_FOV)
	{
		Camera_FOV->SetParameterValue(DeveloperSettings.DeveloperCameraSettings.DeveloperCameraFOV);
	}

	if (Camera_Location)
	{
		Camera_Location->SetVectorValue(DeveloperSettings.DeveloperCameraSettings.DeveloperCameraLocation);
	}

	if (Camera_Rotation)
	{
		Camera_Rotation->SetVectorValue(DeveloperSettings.DeveloperCameraSettings.DeveloperCameraRotation);
	}

	LoadDeveloperMovementSettingsIntoWidgets(DeveloperSettings.DeveloperCharacterMovementSettings);
	LoadDeveloperCombatSettingsIntoWidgets(DeveloperSettings.DeveloperCombatSettings);
	LoadDeveloperChargeShotSettingsIntoWidgets(
		DeveloperSettings.DeveloperChargeShotSettings,
		DeveloperSettings.DeveloperCombatSettings);
	LoadDeveloperTraversalSettingsIntoWidgets(
		DeveloperSettings.DeveloperLedgeSettings,
		DeveloperSettings.DeveloperSlideDashSettings,
		DeveloperSettings.DeveloperWallSettings);
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperCameraManagerSettingsIntoWidgets(
	const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings)
{
	if (CameraManager_IdleSpeedThreshold)
	{
		CameraManager_IdleSpeedThreshold->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold);
	}

	if (CameraManager_HOffset)
	{
		CameraManager_HOffset->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset);
	}

	if (CameraManager_HOffsetInterpSpeedStart)
	{
		CameraManager_HOffsetInterpSpeedStart->SetParameterValue(
			DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart);
	}

	if (CameraManager_HOffsetInterpSpeedEnd)
	{
		CameraManager_HOffsetInterpSpeedEnd->SetParameterValue(
			DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd);
	}

	if (CameraManager_VOffset)
	{
		CameraManager_VOffset->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset);
	}

	if (CameraManager_VOffsetInterpSpeed)
	{
		CameraManager_VOffsetInterpSpeed->SetParameterValue(
			DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperMovementSettingsIntoWidgets(
	const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings)
{
	if (Movement_MaxWalkSpeed)
	{
		Movement_MaxWalkSpeed->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed);
	}

	if (Movement_MaxFlySpeed)
	{
		Movement_MaxFlySpeed->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed);
	}

	if (Movement_MaxAcceleration)
	{
		Movement_MaxAcceleration->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration);
	}

	if (Movement_BrakingDecelerationWalking)
	{
		Movement_BrakingDecelerationWalking->SetParameterValue(
			DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking);
	}

	if (Movement_JumpZVelocity)
	{
		Movement_JumpZVelocity->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity);
	}

	if (Movement_JumpApexGravityMultiplier)
	{
		Movement_JumpApexGravityMultiplier->SetParameterValue(
			DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier);
	}

	if (Movement_GravityScale)
	{
		Movement_GravityScale->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementGravityScale);
	}

	if (Movement_Mass)
	{
		Movement_Mass->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementMass);
	}

	if (Movement_BrakingFrictionFactor)
	{
		Movement_BrakingFrictionFactor->SetParameterValue(
			DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor);
	}

	if (Movement_GroundFriction)
	{
		Movement_GroundFriction->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction);
	}

	if (Movement_AirControl)
	{
		Movement_AirControl->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementAirControl);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperCombatSettingsIntoWidgets(
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings)
{
	if (Combat_MaxHealth)
	{
		Combat_MaxHealth->SetParameterValue(DeveloperCombatSettings.DeveloperCombatMaxHealth);
	}

	if (Combat_CurrentHealth)
	{
		Combat_CurrentHealth->SetParameterValue(DeveloperCombatSettings.DeveloperCombatCurrentHealth);
	}

	if (Combat_MeleeAttackDamage)
	{
		Combat_MeleeAttackDamage->SetParameterValue(DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage);
	}

	if (Combat_MeleeAttackDelay)
	{
		Combat_MeleeAttackDelay->SetParameterValue(DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay);
	}

	if (Combat_RangeBaseAttackDamage)
	{
		Combat_RangeBaseAttackDamage->SetParameterValue(DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage);
	}

	if (Combat_RangeBaseAttackSpeed)
	{
		Combat_RangeBaseAttackSpeed->SetParameterValue(DeveloperCombatSettings.DeveloperCombatRangeBaseAttackSpeed);
	}

	if (Combat_RangeChargedAttackDamage)
	{
		Combat_RangeChargedAttackDamage->SetParameterValue(DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage);
	}

	if (Combat_RangeChargedAttackSpeed)
	{
		Combat_RangeChargedAttackSpeed->SetParameterValue(DeveloperCombatSettings.DeveloperCombatRangeChargedAttackSpeed);
	}

	if (Combat_RangeAttackDelay)
	{
		Combat_RangeAttackDelay->SetParameterValue(DeveloperCombatSettings.DeveloperCombatRangeAttackDelay);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperChargeShotSettingsIntoWidgets(
	const FPlatformerChargeShotTuning& DeveloperChargeShotSettings,
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings)
{
	if (Combat_RangePartialAttackDamage)
	{
		Combat_RangePartialAttackDamage->SetParameterValue(
			ResolveDeveloperPartialAttackDamage(DeveloperCombatSettings, DeveloperChargeShotSettings));
	}

	if (Combat_RangePartialAttackSpeed)
	{
		Combat_RangePartialAttackSpeed->SetParameterValue(
			ResolveDeveloperPartialAttackSpeed(DeveloperCombatSettings, DeveloperChargeShotSettings));
	}

	if (Combat_PartialChargeTime)
	{
		Combat_PartialChargeTime->SetParameterValue(DeveloperChargeShotSettings.PartialChargeTime);
	}

	if (Combat_FullChargeTime)
	{
		Combat_FullChargeTime->SetParameterValue(DeveloperChargeShotSettings.FullChargeTime);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperTraversalSettingsIntoWidgets(
	const FPlatformerLedgeTraversalSettings& DeveloperLedgeSettings,
	const FPlatformerSlideDashSettings& DeveloperSlideDashSettings,
	const FPlatformerWallTraversalSettings& DeveloperWallSettings)
{
	if (Ledge_DetectionDistance)
	{
		Ledge_DetectionDistance->SetParameterValue(DeveloperLedgeSettings.DetectionDistance);
	}

	if (Ledge_MaxReachHeight)
	{
		Ledge_MaxReachHeight->SetParameterValue(DeveloperLedgeSettings.MaxReachHeight);
	}

	if (Ledge_MinHangHeight)
	{
		Ledge_MinHangHeight->SetParameterValue(DeveloperLedgeSettings.MinHangHeight);
	}

	if (Ledge_ForwardProbeRadius)
	{
		Ledge_ForwardProbeRadius->SetParameterValue(DeveloperLedgeSettings.ForwardProbeRadius);
	}

	if (Ledge_TopSurfaceForwardOffset)
	{
		Ledge_TopSurfaceForwardOffset->SetParameterValue(DeveloperLedgeSettings.TopSurfaceProbeForwardOffset);
	}

	if (Ledge_HangForwardOffset)
	{
		Ledge_HangForwardOffset->SetParameterValue(DeveloperLedgeSettings.HangForwardOffset);
	}

	if (Ledge_HangVerticalOffset)
	{
		Ledge_HangVerticalOffset->SetParameterValue(DeveloperLedgeSettings.HangVerticalOffset);
	}

	if (Ledge_ClimbSpeed)
	{
		Ledge_ClimbSpeed->SetParameterValue(DeveloperLedgeSettings.ClimbSpeed);
	}

	if (Ledge_ForgivenessWindow)
	{
		Ledge_ForgivenessWindow->SetParameterValue(DeveloperLedgeSettings.ForgivenessWindow);
	}

	if (Ledge_RegrabCooldown)
	{
		Ledge_RegrabCooldown->SetParameterValue(DeveloperLedgeSettings.RegrabCooldown);
	}

	if (Dash_DashSpeed)
	{
		Dash_DashSpeed->SetParameterValue(DeveloperSlideDashSettings.DashSpeed);
	}

	if (Dash_DashDistance)
	{
		Dash_DashDistance->SetParameterValue(DeveloperSlideDashSettings.DashDistance);
	}

	if (Dash_DashDuration)
	{
		Dash_DashDuration->SetParameterValue(DeveloperSlideDashSettings.DashDuration);
	}

	if (Dash_DashRecovery)
	{
		Dash_DashRecovery->SetParameterValue(DeveloperSlideDashSettings.DashRecovery);
	}

	if (Dash_DashHitboxScale)
	{
		Dash_DashHitboxScale->SetParameterValue(DeveloperSlideDashSettings.DashHitboxScale);
	}

	if (Wall_ProbeDistance)
	{
		Wall_ProbeDistance->SetParameterValue(DeveloperWallSettings.ProbeDistance);
	}

	if (Wall_ProbeHeightOffset)
	{
		Wall_ProbeHeightOffset->SetParameterValue(DeveloperWallSettings.ProbeHeightOffset);
	}

	if (Wall_SlideSpeed)
	{
		Wall_SlideSpeed->SetParameterValue(DeveloperWallSettings.SlideSpeed);
	}

	if (Wall_WallJumpForce)
	{
		Wall_WallJumpForce->SetParameterValue(DeveloperWallSettings.WallJumpForce);
	}

	if (Wall_WallJumpAngleDegrees)
	{
		Wall_WallJumpAngleDegrees->SetParameterValue(DeveloperWallSettings.WallJumpAngleDegrees);
	}

	if (Wall_ClingTime)
	{
		Wall_ClingTime->SetParameterValue(DeveloperWallSettings.ClingTime);
	}

	if (Wall_SameWallReattachCooldown)
	{
		Wall_SameWallReattachCooldown->SetParameterValue(DeveloperWallSettings.SameWallReattachCooldown);
	}

	if (Wall_MinFallSpeedForSlide)
	{
		Wall_MinFallSpeedForSlide->SetParameterValue(DeveloperWallSettings.MinFallSpeedForSlide);
	}
}

FPlatformerDeveloperSettingsSnapshot UPlatformerDeveloperSettingsWidget::CaptureDeveloperSettingsSnapshotFromRuntime() const
{
	FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;

	if (APlatformerCharacterBase* DeveloperTargetCharacter = GetDeveloperTargetCharacter())
	{
		DeveloperSettingsSnapshot = DeveloperTargetCharacter->CaptureDeveloperSettingsSnapshot();
	}

	if (APlatformerCameraManager* DeveloperTargetCameraManager = GetDeveloperTargetCameraManager())
	{
		DeveloperSettingsSnapshot.CameraManagerSettings = DeveloperTargetCameraManager->CaptureDeveloperCameraManagerSettings();
	}

	if (Common_AutoRestartLevel)
	{
		DeveloperSettingsSnapshot.bAutoRestartLevel = Common_AutoRestartLevel->GetCheckboxValue();
	}

	return DeveloperSettingsSnapshot;
}

void UPlatformerDeveloperSettingsWidget::PatchWorkingCopyFromWidgets()
{
	WorkingCopy.CharacterSettings = BuildDeveloperCharacterSettingsFromWidgets(WorkingCopy.CharacterSettings);
	WorkingCopy.CameraManagerSettings = BuildDeveloperCameraManagerSettingsFromWidgets(WorkingCopy.CameraManagerSettings);

	if (HasDeveloperCombatWidgetBindings())
	{
		WorkingCopy.bHasSavedCombatSettings = true;
	}

	if (HasDeveloperChargeShotWidgetBindings())
	{
		WorkingCopy.bHasSavedChargeShotSettings = true;
	}

	if (HasDeveloperTraversalWidgetBindings())
	{
		WorkingCopy.bHasSavedTraversalSettings = true;
	}

	if (Common_AutoRestartLevel)
	{
		WorkingCopy.bAutoRestartLevel = Common_AutoRestartLevel->GetCheckboxValue();
	}
}

FDeveloperPlatformerCharacterSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCharacterSettingsFromWidgets(
	const FDeveloperPlatformerCharacterSettings& BaseDeveloperSettings) const
{
	FDeveloperPlatformerCharacterSettings DeveloperSettings = BaseDeveloperSettings;

	if (SpringArm_ArmLength)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmArmLength = SpringArm_ArmLength->GetEditableParameterValue();
	}

	if (SpringArm_Location)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmLocation = SpringArm_Location->GetVectorValue();
	}

	if (SpringArm_Rotation)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperSpringArmRotation = SpringArm_Rotation->GetVectorValue();
	}

	if (Camera_FOV)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperCameraFOV = Camera_FOV->GetEditableParameterValue();
	}

	if (Camera_Location)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperCameraLocation = Camera_Location->GetVectorValue();
	}

	if (Camera_Rotation)
	{
		DeveloperSettings.DeveloperCameraSettings.DeveloperCameraRotation = Camera_Rotation->GetVectorValue();
	}

	DeveloperSettings.DeveloperCharacterMovementSettings =
		BuildDeveloperMovementSettingsFromWidgets(DeveloperSettings.DeveloperCharacterMovementSettings);
	DeveloperSettings.DeveloperCombatSettings = BuildDeveloperCombatSettingsFromWidgets(DeveloperSettings.DeveloperCombatSettings);
	DeveloperSettings.DeveloperChargeShotSettings = BuildDeveloperChargeShotSettingsFromWidgets(
		DeveloperSettings.DeveloperChargeShotSettings,
		DeveloperSettings.DeveloperCombatSettings);
	DeveloperSettings.DeveloperLedgeSettings = BuildDeveloperLedgeSettingsFromWidgets(DeveloperSettings.DeveloperLedgeSettings);
	DeveloperSettings.DeveloperSlideDashSettings = BuildDeveloperSlideDashSettingsFromWidgets(
		DeveloperSettings.DeveloperSlideDashSettings);
	DeveloperSettings.DeveloperWallSettings = BuildDeveloperWallSettingsFromWidgets(DeveloperSettings.DeveloperWallSettings);

	return DeveloperSettings;
}

FDeveloperPlatformerCameraManagerSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCameraManagerSettingsFromWidgets(
	const FDeveloperPlatformerCameraManagerSettings& BaseDeveloperCameraManagerSettings) const
{
	FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings = BaseDeveloperCameraManagerSettings;

	if (CameraManager_IdleSpeedThreshold)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold =
			CameraManager_IdleSpeedThreshold->GetEditableParameterValue();
	}

	if (CameraManager_HOffset)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset =
			CameraManager_HOffset->GetEditableParameterValue();
	}

	if (CameraManager_HOffsetInterpSpeedStart)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart =
			CameraManager_HOffsetInterpSpeedStart->GetEditableParameterValue();
	}

	if (CameraManager_HOffsetInterpSpeedEnd)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd =
			CameraManager_HOffsetInterpSpeedEnd->GetEditableParameterValue();
	}

	if (CameraManager_VOffset)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset =
			CameraManager_VOffset->GetEditableParameterValue();
	}

	if (CameraManager_VOffsetInterpSpeed)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed =
			CameraManager_VOffsetInterpSpeed->GetEditableParameterValue();
	}

	return DeveloperCameraManagerSettings;
}

FDeveloperPlatformerCharacterMovementSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperMovementSettingsFromWidgets(
	const FDeveloperPlatformerCharacterMovementSettings& BaseDeveloperCharacterMovementSettings) const
{
	FDeveloperPlatformerCharacterMovementSettings DeveloperCharacterMovementSettings = BaseDeveloperCharacterMovementSettings;

	if (Movement_MaxWalkSpeed)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed = Movement_MaxWalkSpeed->GetEditableParameterValue();
	}

	if (Movement_MaxFlySpeed)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed = Movement_MaxFlySpeed->GetEditableParameterValue();
	}

	if (Movement_MaxAcceleration)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration = Movement_MaxAcceleration->GetEditableParameterValue();
	}

	if (Movement_BrakingDecelerationWalking)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking =
			Movement_BrakingDecelerationWalking->GetEditableParameterValue();
	}

	if (Movement_JumpZVelocity)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity = Movement_JumpZVelocity->GetEditableParameterValue();
	}

	if (Movement_JumpApexGravityMultiplier)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier =
			Movement_JumpApexGravityMultiplier->GetEditableParameterValue();
	}

	if (Movement_GravityScale)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementGravityScale = Movement_GravityScale->GetEditableParameterValue();
	}

	if (Movement_Mass)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementMass = Movement_Mass->GetEditableParameterValue();
	}

	if (Movement_BrakingFrictionFactor)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor =
			Movement_BrakingFrictionFactor->GetEditableParameterValue();
	}

	if (Movement_GroundFriction)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction = Movement_GroundFriction->GetEditableParameterValue();
	}

	if (Movement_AirControl)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementAirControl = Movement_AirControl->GetEditableParameterValue();
	}

	return DeveloperCharacterMovementSettings;
}

FDeveloperPlatformerCombatSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCombatSettingsFromWidgets(
	const FDeveloperPlatformerCombatSettings& BaseDeveloperCombatSettings) const
{
	FDeveloperPlatformerCombatSettings DeveloperCombatSettings = BaseDeveloperCombatSettings;

	if (Combat_MaxHealth)
	{
		DeveloperCombatSettings.DeveloperCombatMaxHealth = Combat_MaxHealth->GetEditableParameterValue();
	}

	if (Combat_CurrentHealth)
	{
		DeveloperCombatSettings.DeveloperCombatCurrentHealth = Combat_CurrentHealth->GetEditableParameterValue();
	}

	if (Combat_MeleeAttackDamage)
	{
		DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage = Combat_MeleeAttackDamage->GetEditableParameterValue();
	}

	if (Combat_MeleeAttackDelay)
	{
		DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay = Combat_MeleeAttackDelay->GetEditableParameterValue();
	}

	if (Combat_RangeBaseAttackDamage)
	{
		DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage = Combat_RangeBaseAttackDamage->GetEditableParameterValue();
	}

	if (Combat_RangeBaseAttackSpeed)
	{
		DeveloperCombatSettings.DeveloperCombatRangeBaseAttackSpeed = Combat_RangeBaseAttackSpeed->GetEditableParameterValue();
	}

	if (Combat_RangeChargedAttackDamage)
	{
		DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage = Combat_RangeChargedAttackDamage->GetEditableParameterValue();
	}

	if (Combat_RangeChargedAttackSpeed)
	{
		DeveloperCombatSettings.DeveloperCombatRangeChargedAttackSpeed = Combat_RangeChargedAttackSpeed->GetEditableParameterValue();
	}

	if (Combat_RangeAttackDelay)
	{
		DeveloperCombatSettings.DeveloperCombatRangeAttackDelay = Combat_RangeAttackDelay->GetEditableParameterValue();
	}

	return DeveloperCombatSettings;
}

FPlatformerChargeShotTuning UPlatformerDeveloperSettingsWidget::BuildDeveloperChargeShotSettingsFromWidgets(
	const FPlatformerChargeShotTuning& BaseDeveloperChargeShotSettings,
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings) const
{
	FPlatformerChargeShotTuning DeveloperChargeShotSettings = BaseDeveloperChargeShotSettings;

	if (Combat_RangePartialAttackDamage)
	{
		DeveloperChargeShotSettings.PartialDamageMultiplier = ResolveDeveloperPartialDamageMultiplierFromDisplay(
			Combat_RangePartialAttackDamage->GetEditableParameterValue(),
			DeveloperCombatSettings,
			BaseDeveloperChargeShotSettings);
	}

	if (Combat_RangePartialAttackSpeed)
	{
		DeveloperChargeShotSettings.PartialProjectileSpeedMultiplier = ResolveDeveloperPartialSpeedMultiplierFromDisplay(
			Combat_RangePartialAttackSpeed->GetEditableParameterValue(),
			DeveloperCombatSettings,
			BaseDeveloperChargeShotSettings);
	}

	if (Combat_PartialChargeTime)
	{
		DeveloperChargeShotSettings.PartialChargeTime = Combat_PartialChargeTime->GetEditableParameterValue();
	}

	if (Combat_FullChargeTime)
	{
		DeveloperChargeShotSettings.FullChargeTime = Combat_FullChargeTime->GetEditableParameterValue();
	}

	return DeveloperChargeShotSettings;
}

FPlatformerLedgeTraversalSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperLedgeSettingsFromWidgets(
	const FPlatformerLedgeTraversalSettings& BaseDeveloperLedgeSettings) const
{
	FPlatformerLedgeTraversalSettings DeveloperLedgeSettings = BaseDeveloperLedgeSettings;

	if (Ledge_DetectionDistance)
	{
		DeveloperLedgeSettings.DetectionDistance = Ledge_DetectionDistance->GetEditableParameterValue();
	}

	if (Ledge_MaxReachHeight)
	{
		DeveloperLedgeSettings.MaxReachHeight = Ledge_MaxReachHeight->GetEditableParameterValue();
	}

	if (Ledge_MinHangHeight)
	{
		DeveloperLedgeSettings.MinHangHeight = Ledge_MinHangHeight->GetEditableParameterValue();
	}

	if (Ledge_ForwardProbeRadius)
	{
		DeveloperLedgeSettings.ForwardProbeRadius = Ledge_ForwardProbeRadius->GetEditableParameterValue();
	}

	if (Ledge_TopSurfaceForwardOffset)
	{
		DeveloperLedgeSettings.TopSurfaceProbeForwardOffset = Ledge_TopSurfaceForwardOffset->GetEditableParameterValue();
	}

	if (Ledge_HangForwardOffset)
	{
		DeveloperLedgeSettings.HangForwardOffset = Ledge_HangForwardOffset->GetEditableParameterValue();
	}

	if (Ledge_HangVerticalOffset)
	{
		DeveloperLedgeSettings.HangVerticalOffset = Ledge_HangVerticalOffset->GetEditableParameterValue();
	}

	if (Ledge_ClimbSpeed)
	{
		DeveloperLedgeSettings.ClimbSpeed = Ledge_ClimbSpeed->GetEditableParameterValue();
	}

	if (Ledge_ForgivenessWindow)
	{
		DeveloperLedgeSettings.ForgivenessWindow = Ledge_ForgivenessWindow->GetEditableParameterValue();
	}

	if (Ledge_RegrabCooldown)
	{
		DeveloperLedgeSettings.RegrabCooldown = Ledge_RegrabCooldown->GetEditableParameterValue();
	}

	return DeveloperLedgeSettings;
}

FPlatformerSlideDashSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperSlideDashSettingsFromWidgets(
	const FPlatformerSlideDashSettings& BaseDeveloperSlideDashSettings) const
{
	FPlatformerSlideDashSettings DeveloperSlideDashSettings = BaseDeveloperSlideDashSettings;

	if (Dash_DashSpeed)
	{
		DeveloperSlideDashSettings.DashSpeed = Dash_DashSpeed->GetEditableParameterValue();
	}

	if (Dash_DashDistance)
	{
		DeveloperSlideDashSettings.DashDistance = Dash_DashDistance->GetEditableParameterValue();
	}

	if (Dash_DashDuration)
	{
		DeveloperSlideDashSettings.DashDuration = Dash_DashDuration->GetEditableParameterValue();
	}

	if (Dash_DashRecovery)
	{
		DeveloperSlideDashSettings.DashRecovery = Dash_DashRecovery->GetEditableParameterValue();
	}

	if (Dash_DashHitboxScale)
	{
		DeveloperSlideDashSettings.DashHitboxScale = Dash_DashHitboxScale->GetEditableParameterValue();
	}

	return DeveloperSlideDashSettings;
}

FPlatformerWallTraversalSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperWallSettingsFromWidgets(
	const FPlatformerWallTraversalSettings& BaseDeveloperWallSettings) const
{
	FPlatformerWallTraversalSettings DeveloperWallSettings = BaseDeveloperWallSettings;

	if (Wall_ProbeDistance)
	{
		DeveloperWallSettings.ProbeDistance = Wall_ProbeDistance->GetEditableParameterValue();
	}

	if (Wall_ProbeHeightOffset)
	{
		DeveloperWallSettings.ProbeHeightOffset = Wall_ProbeHeightOffset->GetEditableParameterValue();
	}

	if (Wall_SlideSpeed)
	{
		DeveloperWallSettings.SlideSpeed = Wall_SlideSpeed->GetEditableParameterValue();
	}

	if (Wall_WallJumpForce)
	{
		DeveloperWallSettings.WallJumpForce = Wall_WallJumpForce->GetEditableParameterValue();
	}

	if (Wall_WallJumpAngleDegrees)
	{
		DeveloperWallSettings.WallJumpAngleDegrees = Wall_WallJumpAngleDegrees->GetEditableParameterValue();
	}

	if (Wall_ClingTime)
	{
		DeveloperWallSettings.ClingTime = Wall_ClingTime->GetEditableParameterValue();
	}

	if (Wall_SameWallReattachCooldown)
	{
		DeveloperWallSettings.SameWallReattachCooldown = Wall_SameWallReattachCooldown->GetEditableParameterValue();
	}

	if (Wall_MinFallSpeedForSlide)
	{
		DeveloperWallSettings.MinFallSpeedForSlide = Wall_MinFallSpeedForSlide->GetEditableParameterValue();
	}

	return DeveloperWallSettings;
}

void UPlatformerDeveloperSettingsWidget::ApplyDeveloperSettingsSnapshotToTargets(
	const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot) const
{
	if (APlatformerCharacterBase* DeveloperTargetCharacter = GetDeveloperTargetCharacter())
	{
		DeveloperTargetCharacter->ApplyDeveloperSettingsSnapshot(DeveloperSettingsSnapshot);
	}

	if (APlatformerCameraManager* DeveloperTargetCameraManager = GetDeveloperTargetCameraManager())
	{
		DeveloperTargetCameraManager->ApplyDeveloperCameraManagerSettings(DeveloperSettingsSnapshot.CameraManagerSettings);
	}
}

APlatformerCharacterBase* UPlatformerDeveloperSettingsWidget::GetDeveloperTargetCharacter() const
{
	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		return Cast<APlatformerCharacterBase>(OwningPlayerController->GetPawn());
	}

	return nullptr;
}

APlatformerCameraManager* UPlatformerDeveloperSettingsWidget::GetDeveloperTargetCameraManager() const
{
	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		return Cast<APlatformerCameraManager>(OwningPlayerController->PlayerCameraManager);
	}

	return nullptr;
}

UPlatformerDeveloperSettingsSubsystem* UPlatformerDeveloperSettingsWidget::GetDeveloperSettingsSubsystem() const
{
	if (UWorld* WidgetWorld = GetWorld())
	{
		if (UGameInstance* GameInstance = WidgetWorld->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UPlatformerDeveloperSettingsSubsystem>();
		}
	}

	return nullptr;
}

void UPlatformerDeveloperSettingsWidget::CloseDeveloperSettingsWidget()
{
	RemoveFromParent();

	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		OwningPlayerController->SetInputMode(InputMode);
		OwningPlayerController->bShowMouseCursor = false;
		OwningPlayerController->SetPause(false);
	}
}

bool UPlatformerDeveloperSettingsWidget::HasDeveloperCombatWidgetBindings() const
{
	return Combat_MaxHealth
		|| Combat_CurrentHealth
		|| Combat_MeleeAttackDamage
		|| Combat_MeleeAttackDelay
		|| Combat_RangeBaseAttackDamage
		|| Combat_RangeBaseAttackSpeed
		|| Combat_RangeChargedAttackDamage
		|| Combat_RangeChargedAttackSpeed
		|| Combat_RangeAttackDelay;
}

bool UPlatformerDeveloperSettingsWidget::HasDeveloperChargeShotWidgetBindings() const
{
	return Combat_RangePartialAttackDamage
		|| Combat_RangePartialAttackSpeed
		|| Combat_PartialChargeTime
		|| Combat_FullChargeTime;
}

bool UPlatformerDeveloperSettingsWidget::HasDeveloperTraversalWidgetBindings() const
{
	return Ledge_DetectionDistance
		|| Ledge_MaxReachHeight
		|| Ledge_MinHangHeight
		|| Ledge_ForwardProbeRadius
		|| Ledge_TopSurfaceForwardOffset
		|| Ledge_HangForwardOffset
		|| Ledge_HangVerticalOffset
		|| Ledge_ClimbSpeed
		|| Ledge_ForgivenessWindow
		|| Ledge_RegrabCooldown
		|| Dash_DashSpeed
		|| Dash_DashDistance
		|| Dash_DashDuration
		|| Dash_DashRecovery
		|| Dash_DashHitboxScale
		|| Wall_ProbeDistance
		|| Wall_ProbeHeightOffset
		|| Wall_SlideSpeed
		|| Wall_WallJumpForce
		|| Wall_WallJumpAngleDegrees
		|| Wall_ClingTime
		|| Wall_SameWallReattachCooldown
		|| Wall_MinFallSpeedForSlide;
}

void UPlatformerDeveloperSettingsWidget::RefreshDeveloperSlotWidgets()
{
	CachedDeveloperSlots.Reset();

	UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	if (DeveloperSettingsSubsystem)
	{
		CachedDeveloperSlots = DeveloperSettingsSubsystem->GetAvailableSlots();
	}

	const FString CurrentSlotName = DeveloperSettingsSubsystem ? DeveloperSettingsSubsystem->GetCurrentSlotDisplayName() : FString();
	const FString PreferredSlotName = !CurrentSlotName.IsEmpty()
		? CurrentSlotName
		: (CachedDeveloperSlots.Num() > 0 ? CachedDeveloperSlots[0].DisplayName : FString());
	const FString RequestedSlotName = CurrentSlotName;

	TGuardValue<bool> SynchronizationGuard(bIsSynchronizingSlotWidgets, true);

	if (Combo_Slots)
	{
		Combo_Slots->ClearOptions();

		for (const FPlatformerDeveloperSettingsSlotDescriptor& SlotDescriptor : CachedDeveloperSlots)
		{
			Combo_Slots->AddOption(SlotDescriptor.DisplayName);
		}

		if (!PreferredSlotName.IsEmpty())
		{
			Combo_Slots->SetSelectedOption(PreferredSlotName);
		}
		else
		{
			Combo_Slots->ClearSelection();
		}
	}

	if (Txt_CurrSlotName)
	{
		Txt_CurrSlotName->SetText(FText::FromString(CurrentSlotName));
	}

	SyncRequestedSlotName(RequestedSlotName);
	UpdateDeveloperSlotActionStates();
}

const FPlatformerDeveloperSettingsSlotDescriptor* UPlatformerDeveloperSettingsWidget::FindCachedSlotDescriptorByDisplayName(
	const FString& DisplayName) const
{
	const FString SanitizedDisplayName = DisplayName.TrimStartAndEnd();
	if (SanitizedDisplayName.IsEmpty())
	{
		return nullptr;
	}

	return CachedDeveloperSlots.FindByPredicate([&SanitizedDisplayName](const FPlatformerDeveloperSettingsSlotDescriptor& SlotDescriptor)
	{
		return SlotDescriptor.DisplayName.Equals(SanitizedDisplayName, ESearchCase::IgnoreCase);
	});
}

FString UPlatformerDeveloperSettingsWidget::GetRequestedDeveloperSlotName() const
{
	if (Editable_SlotName)
	{
		return Editable_SlotName->GetText().ToString().TrimStartAndEnd();
	}

	const FString SelectedSlotName = GetSelectedDeveloperSlotName();
	if (!SelectedSlotName.IsEmpty())
	{
		return SelectedSlotName;
	}

	return GetDeveloperSettingsSubsystem() ? GetDeveloperSettingsSubsystem()->GetCurrentSlotDisplayName() : FString();
}

FString UPlatformerDeveloperSettingsWidget::GetSelectedDeveloperSlotName() const
{
	if (Combo_Slots)
	{
		return Combo_Slots->GetSelectedOption().TrimStartAndEnd();
	}

	return FString();
}

void UPlatformerDeveloperSettingsWidget::SyncRequestedSlotName(const FString& SlotName)
{
	if (Editable_SlotName)
	{
		Editable_SlotName->SetText(FText::FromString(SlotName));
	}
}

void UPlatformerDeveloperSettingsWidget::UpdateDeveloperSlotActionStates()
{
	const UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	const bool bHasCurrentSlot = DeveloperSettingsSubsystem && DeveloperSettingsSubsystem->HasCurrentSlot();
	const bool bHasSelectedSlot = FindCachedSlotDescriptorByDisplayName(GetSelectedDeveloperSlotName()) != nullptr;
	const bool bHasRequestedSlotName = !GetRequestedDeveloperSlotName().IsEmpty();

	if (Butt_Save)
	{
		Butt_Save->SetIsEnabled(bHasCurrentSlot);
	}

	if (Butt_SaveAs)
	{
		Butt_SaveAs->SetIsEnabled(bHasRequestedSlotName);
	}

	if (Butt_Load)
	{
		Butt_Load->SetIsEnabled(bHasSelectedSlot);
	}

	if (Butt_Delete)
	{
		Butt_Delete->SetIsEnabled(bHasSelectedSlot);
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveClicked()
{
	UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	if (!DeveloperSettingsSubsystem)
	{
		return;
	}

	PatchWorkingCopyFromWidgets();

	if (DeveloperSettingsSubsystem->SaveCurrent(WorkingCopy))
	{
		ApplyDeveloperSettingsSnapshotToTargets(WorkingCopy);
		RefreshDeveloperSlotWidgets();
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveAsClicked()
{
	UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	if (!DeveloperSettingsSubsystem)
	{
		return;
	}

	PatchWorkingCopyFromWidgets();

	FPlatformerDeveloperSettingsSlotDescriptor SavedSlot;
	if (DeveloperSettingsSubsystem->SaveAs(GetRequestedDeveloperSlotName(), WorkingCopy, SavedSlot))
	{
		ApplyDeveloperSettingsSnapshotToTargets(WorkingCopy);
		RefreshDeveloperSlotWidgets();
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperLoadClicked()
{
	UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	if (!DeveloperSettingsSubsystem)
	{
		return;
	}

	const FPlatformerDeveloperSettingsSlotDescriptor* SelectedSlot =
		FindCachedSlotDescriptorByDisplayName(GetSelectedDeveloperSlotName());
	if (!SelectedSlot)
	{
		return;
	}

	FPlatformerDeveloperSettingsSnapshot LoadedSnapshot;
	if (DeveloperSettingsSubsystem->LoadSlot(SelectedSlot->SlotId, LoadedSnapshot))
	{
		WorkingCopy = LoadedSnapshot;
		ApplyDeveloperSettingsSnapshotToTargets(WorkingCopy);
		LoadDeveloperSettingsSnapshotIntoWidgets(WorkingCopy);
		RefreshDeveloperSlotWidgets();
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperDeleteClicked()
{
	UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem = GetDeveloperSettingsSubsystem();
	if (!DeveloperSettingsSubsystem)
	{
		return;
	}

	const FPlatformerDeveloperSettingsSlotDescriptor* SelectedSlot =
		FindCachedSlotDescriptorByDisplayName(GetSelectedDeveloperSlotName());
	if (!SelectedSlot)
	{
		return;
	}

	const bool bDeletedCurrentSlot = DeveloperSettingsSubsystem->HasCurrentSlot()
		&& DeveloperSettingsSubsystem->GetCurrentSlotDisplayName().Equals(SelectedSlot->DisplayName, ESearchCase::IgnoreCase);

	if (DeveloperSettingsSubsystem->DeleteSlot(SelectedSlot->SlotId))
	{
		if (bDeletedCurrentSlot)
		{
			WorkingCopy = CaptureDeveloperSettingsSnapshotFromRuntime();
			LoadDeveloperSettingsSnapshotIntoWidgets(WorkingCopy);
		}

		RefreshDeveloperSlotWidgets();
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperSlotSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	if (!bIsSynchronizingSlotWidgets)
	{
		SyncRequestedSlotName(SelectedItem);
	}

	UpdateDeveloperSlotActionStates();
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperSlotNameChanged(const FText& InText)
{
	(void)InText;

	if (!bIsSynchronizingSlotWidgets)
	{
		UpdateDeveloperSlotActionStates();
	}
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperCloseClicked()
{
	CloseDeveloperSettingsWidget();
}
