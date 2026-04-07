#include "UI/PlatformerDeveloperSettingsWidget.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/Button.h"
#include "Core/SaveGame/SaveDeveloperSettings.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/PlayerController.h"
#include "Platformer/Camera/PlatformerCameraManager.h"
#include "UI/DeveloperParameterWidget.h"
#include "UI/DeveloperVectorWidget.h"

void UPlatformerDeveloperSettingsWidget::RefreshDeveloperSettingsWidget()
{
	if (USaveDeveloperSettings* LoadedDeveloperSettings = USaveDeveloperSettings::LoadDeveloperSettingsFromSlot(this))
	{
		LoadDeveloperCharacterSettingsIntoWidgets(LoadedDeveloperSettings->DeveloperCharacterSettings);
		if (LoadedDeveloperSettings->bHasSavedDeveloperCombatSettings)
		{
			LoadDeveloperCombatSettingsIntoWidgets(LoadedDeveloperSettings->DeveloperCharacterSettings.DeveloperCombatSettings);
		}
		else if (APlatformerCharacterBase* DeveloperTargetCharacter = GetDeveloperTargetCharacter())
		{
			LoadDeveloperCombatSettingsIntoWidgets(DeveloperTargetCharacter->CaptureDeveloperCharacterSettings().DeveloperCombatSettings);
		}
		LoadDeveloperCameraManagerSettingsIntoWidgets(LoadedDeveloperSettings->DeveloperCameraManagerSettings);
		return;
	}

	if (APlatformerCharacterBase* DeveloperTargetCharacter = GetDeveloperTargetCharacter())
	{
		LoadDeveloperCharacterSettingsIntoWidgets(DeveloperTargetCharacter->CaptureDeveloperCharacterSettings());
	}

	if (APlatformerCameraManager* DeveloperTargetCameraManager = GetDeveloperTargetCameraManager())
	{
		LoadDeveloperCameraManagerSettingsIntoWidgets(DeveloperTargetCameraManager->CaptureDeveloperCameraManagerSettings());
	}
}

void UPlatformerDeveloperSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDeveloperSettingsWidget();
}

void UPlatformerDeveloperSettingsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Butt_Save)
	{
		Butt_Save->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveClicked);
	}

	if (Butt_Close)
	{
		Butt_Close->OnClicked.AddDynamic(this, &UPlatformerDeveloperSettingsWidget::HandleDeveloperCloseClicked);
	}
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
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperCameraManagerSettingsIntoWidgets(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings)
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
		CameraManager_HOffsetInterpSpeedStart->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart);
	}

	if (CameraManager_HOffsetInterpSpeedEnd)
	{
		CameraManager_HOffsetInterpSpeedEnd->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd);
	}

	if (CameraManager_VOffset)
	{
		CameraManager_VOffset->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset);
	}

	if (CameraManager_VOffsetInterpSpeed)
	{
		CameraManager_VOffsetInterpSpeed->SetParameterValue(DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed);
	}
}

void UPlatformerDeveloperSettingsWidget::LoadDeveloperMovementSettingsIntoWidgets(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings)
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
		Movement_BrakingDecelerationWalking->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking);
	}

	if (Movement_JumpZVelocity)
	{
		Movement_JumpZVelocity->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity);
	}

	if (Movement_JumpApexGravityMultiplier)
	{
		Movement_JumpApexGravityMultiplier->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier);
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
		Movement_BrakingFrictionFactor->SetParameterValue(DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor);
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

void UPlatformerDeveloperSettingsWidget::LoadDeveloperCombatSettingsIntoWidgets(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings)
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

FDeveloperPlatformerCharacterSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCharacterSettingsFromWidgets() const
{
	FDeveloperPlatformerCharacterSettings DeveloperSettings;

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

	DeveloperSettings.DeveloperCharacterMovementSettings = BuildDeveloperMovementSettingsFromWidgets();
	DeveloperSettings.DeveloperCombatSettings = BuildDeveloperCombatSettingsFromWidgets();

	return DeveloperSettings;
}

FDeveloperPlatformerCameraManagerSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCameraManagerSettingsFromWidgets() const
{
	FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings;

	if (CameraManager_IdleSpeedThreshold)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold = CameraManager_IdleSpeedThreshold->GetEditableParameterValue();
	}

	if (CameraManager_HOffset)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset = CameraManager_HOffset->GetEditableParameterValue();
	}

	if (CameraManager_HOffsetInterpSpeedStart)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart = CameraManager_HOffsetInterpSpeedStart->GetEditableParameterValue();
	}

	if (CameraManager_HOffsetInterpSpeedEnd)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd = CameraManager_HOffsetInterpSpeedEnd->GetEditableParameterValue();
	}

	if (CameraManager_VOffset)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset = CameraManager_VOffset->GetEditableParameterValue();
	}

	if (CameraManager_VOffsetInterpSpeed)
	{
		DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed = CameraManager_VOffsetInterpSpeed->GetEditableParameterValue();
	}

	return DeveloperCameraManagerSettings;
}

FDeveloperPlatformerCharacterMovementSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperMovementSettingsFromWidgets() const
{
	FDeveloperPlatformerCharacterMovementSettings DeveloperCharacterMovementSettings;

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
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking = Movement_BrakingDecelerationWalking->GetEditableParameterValue();
	}

	if (Movement_JumpZVelocity)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity = Movement_JumpZVelocity->GetEditableParameterValue();
	}

	if (Movement_JumpApexGravityMultiplier)
	{
		DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier = Movement_JumpApexGravityMultiplier->GetEditableParameterValue();
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
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor = Movement_BrakingFrictionFactor->GetEditableParameterValue();
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

FDeveloperPlatformerCombatSettings UPlatformerDeveloperSettingsWidget::BuildDeveloperCombatSettingsFromWidgets() const
{
	FDeveloperPlatformerCombatSettings DeveloperCombatSettings;

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

void UPlatformerDeveloperSettingsWidget::ApplyDeveloperSettingsToTargetCharacter(const FDeveloperPlatformerCharacterSettings& DeveloperSettings) const
{
	if (APlatformerCharacterBase* DeveloperTargetCharacter = GetDeveloperTargetCharacter())
	{
		DeveloperTargetCharacter->ApplyDeveloperCharacterSettings(DeveloperSettings);
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

void UPlatformerDeveloperSettingsWidget::ApplyDeveloperSettingsToTargetCameraManager(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings) const
{
	if (APlatformerCameraManager* DeveloperTargetCameraManager = GetDeveloperTargetCameraManager())
	{
		DeveloperTargetCameraManager->ApplyDeveloperCameraManagerSettings(DeveloperCameraManagerSettings);
	}
}

APlatformerCameraManager* UPlatformerDeveloperSettingsWidget::GetDeveloperTargetCameraManager() const
{
	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		return Cast<APlatformerCameraManager>(OwningPlayerController->PlayerCameraManager);
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

void UPlatformerDeveloperSettingsWidget::HandleDeveloperSaveClicked()
{
	const FDeveloperPlatformerCharacterSettings DeveloperSettings = BuildDeveloperCharacterSettingsFromWidgets();
	const FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings = BuildDeveloperCameraManagerSettingsFromWidgets();

	if (USaveDeveloperSettings* DeveloperSaveObject = USaveDeveloperSettings::LoadOrCreateDeveloperSettings(this))
	{
		DeveloperSaveObject->DeveloperCharacterSettings = DeveloperSettings;
		DeveloperSaveObject->DeveloperCameraManagerSettings = DeveloperCameraManagerSettings;
		DeveloperSaveObject->bHasSavedDeveloperCombatSettings = HasDeveloperCombatWidgetBindings();
		USaveDeveloperSettings::WriteDeveloperSettingsToSlot(this, DeveloperSaveObject);
	}

	ApplyDeveloperSettingsToTargetCharacter(DeveloperSettings);
	ApplyDeveloperSettingsToTargetCameraManager(DeveloperCameraManagerSettings);
	CloseDeveloperSettingsWidget();
}

void UPlatformerDeveloperSettingsWidget::HandleDeveloperCloseClicked()
{
	CloseDeveloperSettingsWidget();
}
