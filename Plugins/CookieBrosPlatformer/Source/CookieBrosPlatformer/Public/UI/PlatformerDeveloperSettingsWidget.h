#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "PlatformerDeveloperSettingsWidget.generated.h"

class APlatformerCharacterBase;
class APlatformerCameraManager;
class UButton;
class UDeveloperCheckboxWidget;
class UDeveloperParameterWidget;
class UDeveloperVectorWidget;
class UEditableTextBox;
class UPlatformerDeveloperSettingsSubsystem;
class UTextBlock;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UPlatformerDeveloperSettingsWidget : public UUserWidget
{
	GENERATED_BODY()
//
public:
	UFUNCTION(BlueprintCallable, Category="Developer")
	void RefreshDeveloperSettingsWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> SpringArm_ArmLength;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperVectorWidget> SpringArm_Location;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperVectorWidget> SpringArm_Rotation;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Camera_FOV;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperVectorWidget> Camera_Location;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperVectorWidget> Camera_Rotation;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Save;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Butt_SaveAs;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Butt_Load;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Butt_Delete;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Close;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UComboBoxString> Combo_Slots;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UEditableTextBox> Editable_SlotName;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_CurrSlotName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_IdleSpeedThreshold;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_HOffset;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_HOffsetInterpSpeedStart;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_HOffsetInterpSpeedEnd;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_VOffset;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_VOffsetInterpSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_DeadZoneWidth;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_DeadZoneHeight;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_BoundBoxWidth;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_BoundBoxHeight;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> CameraManager_CrouchInterpSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperCheckboxWidget> Camera_IsOrthographic;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Camera_OrthoWidth;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_MaxWalkSpeed;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_MaxFlySpeed;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_MaxAcceleration;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_BrakingDecelerationWalking;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_JumpZVelocity;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_JumpApexGravityMultiplier;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Movement_JumpHorizontalSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Movement_CrouchCapsuleScale;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperCheckboxWidget> Movement_ShowJumpTrajectory;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_GravityScale;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_Mass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_BrakingFrictionFactor;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_GroundFriction;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Movement_AirControl;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_MaxHealth;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_CurrentHealth;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_MeleeAttackDamage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_MeleeAttackDelay;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangeBaseAttackDamage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangeBaseAttackSpeed;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangeChargedAttackDamage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangeChargedAttackSpeed;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangeAttackDelay;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangePartialAttackDamage;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Combat_RangePartialAttackSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Combat_PartialChargeTime;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Combat_FullChargeTime;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_DetectionDistance;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_MaxReachHeight;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_MinHangHeight;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_ForwardProbeRadius;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_TopSurfaceForwardOffset;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_HangForwardOffset;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_HangVerticalOffset;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_ClimbSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_ForgivenessWindow;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Ledge_RegrabCooldown;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Dash_DashSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Dash_DashDistance;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Dash_DashDuration;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Dash_DashRecovery;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Dash_DashHitboxScale;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_ProbeDistance;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_ProbeHeightOffset;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_SlideSpeed;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_WallJumpForce;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_WallJumpAngleDegrees;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_ClingTime;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_SameWallReattachCooldown;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperParameterWidget> Wall_MinFallSpeedForSlide;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UDeveloperCheckboxWidget> Common_AutoRestartLevel;

	void LoadDeveloperSettingsSnapshotIntoWidgets(const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot);
	void LoadDeveloperCharacterSettingsIntoWidgets(const FDeveloperPlatformerCharacterSettings& DeveloperSettings);
	void LoadDeveloperCameraManagerSettingsIntoWidgets(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings);
	void LoadDeveloperMovementSettingsIntoWidgets(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings);
	void LoadDeveloperCombatSettingsIntoWidgets(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings);
	void LoadDeveloperChargeShotSettingsIntoWidgets(
		const FPlatformerChargeShotTuning& DeveloperChargeShotSettings,
		const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings);
	void LoadDeveloperTraversalSettingsIntoWidgets(
		const FPlatformerLedgeTraversalSettings& DeveloperLedgeSettings,
		const FPlatformerSlideDashSettings& DeveloperSlideDashSettings,
		const FPlatformerWallTraversalSettings& DeveloperWallSettings);
	FPlatformerDeveloperSettingsSnapshot CaptureDeveloperSettingsSnapshotFromRuntime() const;
	void PatchWorkingCopyFromWidgets();
	FDeveloperPlatformerCharacterSettings BuildDeveloperCharacterSettingsFromWidgets(const FDeveloperPlatformerCharacterSettings& BaseDeveloperSettings) const;
	FDeveloperPlatformerCameraManagerSettings BuildDeveloperCameraManagerSettingsFromWidgets(const FDeveloperPlatformerCameraManagerSettings& BaseDeveloperCameraManagerSettings) const;
	FDeveloperPlatformerCharacterMovementSettings BuildDeveloperMovementSettingsFromWidgets(const FDeveloperPlatformerCharacterMovementSettings& BaseDeveloperCharacterMovementSettings) const;
	FDeveloperPlatformerCombatSettings BuildDeveloperCombatSettingsFromWidgets(const FDeveloperPlatformerCombatSettings& BaseDeveloperCombatSettings) const;
	FPlatformerChargeShotTuning BuildDeveloperChargeShotSettingsFromWidgets(
		const FPlatformerChargeShotTuning& BaseDeveloperChargeShotSettings,
		const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings) const;
	FPlatformerLedgeTraversalSettings BuildDeveloperLedgeSettingsFromWidgets(
		const FPlatformerLedgeTraversalSettings& BaseDeveloperLedgeSettings) const;
	FPlatformerSlideDashSettings BuildDeveloperSlideDashSettingsFromWidgets(
		const FPlatformerSlideDashSettings& BaseDeveloperSlideDashSettings) const;
	FPlatformerWallTraversalSettings BuildDeveloperWallSettingsFromWidgets(
		const FPlatformerWallTraversalSettings& BaseDeveloperWallSettings) const;
	void ApplyDeveloperSettingsSnapshotToTargets(const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot) const;
	APlatformerCharacterBase* GetDeveloperTargetCharacter() const;
	APlatformerCameraManager* GetDeveloperTargetCameraManager() const;
	UPlatformerDeveloperSettingsSubsystem* GetDeveloperSettingsSubsystem() const;
	void CloseDeveloperSettingsWidget();
	bool HasDeveloperCombatWidgetBindings() const;
	bool HasDeveloperChargeShotWidgetBindings() const;
	bool HasDeveloperTraversalWidgetBindings() const;
	void RefreshDeveloperSlotWidgets();
	const FPlatformerDeveloperSettingsSlotDescriptor* FindCachedSlotDescriptorByDisplayName(const FString& DisplayName) const;
	FString GetRequestedDeveloperSlotName() const;
	FString GetSelectedDeveloperSlotName() const;
	void SyncRequestedSlotName(const FString& SlotName);
	void UpdateDeveloperSlotActionStates();

private:
	UFUNCTION()
	void HandleDeveloperSaveClicked();

	UFUNCTION()
	void HandleDeveloperSaveAsClicked();

	UFUNCTION()
	void HandleDeveloperLoadClicked();

	UFUNCTION()
	void HandleDeveloperDeleteClicked();

	UFUNCTION()
	void HandleDeveloperSlotSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleDeveloperSlotNameChanged(const FText& InText);

	UFUNCTION()
	void HandleDeveloperCloseClicked();

	UPROPERTY(Transient)
	FPlatformerDeveloperSettingsSnapshot WorkingCopy;

	UPROPERTY(Transient)
	TArray<FPlatformerDeveloperSettingsSlotDescriptor> CachedDeveloperSlots;

	bool bIsSynchronizingSlotWidgets = false;
};
