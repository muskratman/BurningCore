#pragma once

#include "CoreMinimal.h"
#include "Combat/PlatformerCombatCharacterBase.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlatformerCharacterBase.generated.h"

class UPlatformerAbilitySet;
class USpringArmComponent;
class UCameraComponent;
class APlatformerLadder;
class UDamageType;

/**
 * APlatformerCharacterBase
 * Generic playable platformer shell with camera rig, side-view movement, and combat bootstrap.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerCharacterBase : public APlatformerCombatCharacterBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category="Abilities")
	TObjectPtr<UPlatformerAbilitySet> DefaultAbilitySet;

public:
	APlatformerCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeAbilities(const UPlatformerAbilitySet* AbilitySet);
	virtual void NotifyLadderAvailable(APlatformerLadder* Ladder);
	virtual void NotifyLadderUnavailable(APlatformerLadder* Ladder);
	virtual bool EnterLadder(APlatformerLadder* Ladder);
	virtual void ExitLadder(APlatformerLadder* Ladder = nullptr);
	virtual void ApplyDeveloperCharacterSettings(const FDeveloperPlatformerCharacterSettings& DeveloperSettings);
	virtual void ApplyDeveloperSettingsSnapshot(const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot);
	virtual FDeveloperPlatformerCharacterSettings CaptureDeveloperCharacterSettings() const;
	virtual FPlatformerDeveloperSettingsSnapshot CaptureDeveloperSettingsSnapshot() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE bool IsOnLadder() const { return bIsOnLadder; }
	FORCEINLINE APlatformerLadder* GetActiveLadder() const { return ActiveLadder; }
	FORCEINLINE APlatformerLadder* GetAvailableLadder() const { return AvailableLadder; }
	bool HasActiveDeveloperCombatSettings() const;
	const FDeveloperPlatformerCombatSettings& GetActiveDeveloperCombatSettings() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;
	virtual void OnCombatDeath(AActor* DamageInstigatorActor) override;
	virtual void OnCombatRevived() override;
	virtual void OnEnteredLadder(APlatformerLadder* Ladder);
	virtual void OnExitedLadder(APlatformerLadder* Ladder);
	virtual void ApplyDeveloperCameraSettings(const FDeveloperPlatformerCameraSettings& DeveloperCameraSettings);
	virtual FDeveloperPlatformerCameraSettings CaptureDeveloperCameraSettings() const;
	virtual void ApplyDeveloperCharacterMovementSettings(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings);
	virtual FDeveloperPlatformerCharacterMovementSettings CaptureDeveloperCharacterMovementSettings() const;
	virtual void ApplyDeveloperCombatSettings(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings);
	virtual FDeveloperPlatformerCombatSettings CaptureDeveloperCombatSettings() const;
	virtual float GetHealthWidgetVerticalPadding() const override;
	FDeveloperPlatformerCombatSettings ResolveDeveloperCombatSettingsForApplication(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings) const;
	void LoadAndApplyDeveloperSettings();
	void SetHasActiveDeveloperCombatSettings(bool bInHasActiveDeveloperCombatSettings);

	UPROPERTY(Transient)
	TObjectPtr<APlatformerLadder> ActiveLadder;

	UPROPERTY(Transient)
	TObjectPtr<APlatformerLadder> AvailableLadder;

	UPROPERTY(Transient)
	bool bIsOnLadder = false;

	UPROPERTY(Transient)
	float SavedPreLadderGravityScale = 1.0f;

	UPROPERTY(Transient)
	TEnumAsByte<EMovementMode> SavedPreLadderMovementMode = MOVE_Walking;

	UPROPERTY(Transient)
	bool bHadPreLadderGravityOverride = false;

	UPROPERTY(Transient)
	float PreLadderGravityOverride = 1.0f;

	UPROPERTY(Transient)
	bool bHasActiveDeveloperCombatSettings = false;

	UPROPERTY(Transient)
	FDeveloperPlatformerCombatSettings ActiveDeveloperCombatSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(ClampMin=0.0, Units="cm"))
	float PlatformerHealthWidgetVerticalPadding = 20.0f;
};
