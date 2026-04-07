#pragma once

#include "Blueprint/UserWidget.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "PlatformerDeveloperSettingsWidget.generated.h"

class APlatformerCharacterBase;
class APlatformerCameraManager;
class UButton;
class UDeveloperParameterWidget;
class UDeveloperVectorWidget;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UPlatformerDeveloperSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

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

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Butt_Close;

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

	void LoadDeveloperCharacterSettingsIntoWidgets(const FDeveloperPlatformerCharacterSettings& DeveloperSettings);
	void LoadDeveloperCameraManagerSettingsIntoWidgets(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings);
	void LoadDeveloperMovementSettingsIntoWidgets(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings);
	void LoadDeveloperCombatSettingsIntoWidgets(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings);
	FDeveloperPlatformerCharacterSettings BuildDeveloperCharacterSettingsFromWidgets() const;
	FDeveloperPlatformerCameraManagerSettings BuildDeveloperCameraManagerSettingsFromWidgets() const;
	FDeveloperPlatformerCharacterMovementSettings BuildDeveloperMovementSettingsFromWidgets() const;
	FDeveloperPlatformerCombatSettings BuildDeveloperCombatSettingsFromWidgets() const;
	void ApplyDeveloperSettingsToTargetCharacter(const FDeveloperPlatformerCharacterSettings& DeveloperSettings) const;
	void ApplyDeveloperSettingsToTargetCameraManager(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings) const;
	APlatformerCharacterBase* GetDeveloperTargetCharacter() const;
	APlatformerCameraManager* GetDeveloperTargetCameraManager() const;
	void CloseDeveloperSettingsWidget();
	bool HasDeveloperCombatWidgetBindings() const;

private:
	UFUNCTION()
	void HandleDeveloperSaveClicked();

	UFUNCTION()
	void HandleDeveloperCloseClicked();
};
