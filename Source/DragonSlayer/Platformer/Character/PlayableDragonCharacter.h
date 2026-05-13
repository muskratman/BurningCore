#pragma once

#include "CoreMinimal.h"
#include "Character/DragonCharacter.h"
#include "PlayableDragonCharacter.generated.h"

class UInputAction;
struct FGameplayTag;
struct FInputActionValue;
class APlatformerLadder;
class UDragonFormComponent;
class UDragonDashBounceDataAsset;
class UDragonDashMovementComponent;
class UPlatformerInputConfig;
class UPlatformerTraversalComponent;
class UPlatformerTraversalMovementComponent;

/**
 * APlayableDragonCharacter
 * Concrete playable character class for testing the core gameplay loop, Enhanced Input, and GAS.
 */
UCLASS()
class DRAGONSLAYER_API APlayableDragonCharacter : public ADragonCharacter
{
	GENERATED_BODY()

public:
	APlayableDragonCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ApplyDeveloperCharacterSettings(const struct FDeveloperPlatformerCharacterSettings& DeveloperSettings) override;
	virtual void ApplyDeveloperSettingsSnapshot(const struct FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot) override;
	virtual FDeveloperPlatformerCharacterSettings CaptureDeveloperCharacterSettings() const override;
	virtual FPlatformerDeveloperSettingsSnapshot CaptureDeveloperSettingsSnapshot() const override;
	virtual void ApplyDeveloperCharacterMovementSettings(const struct FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Traversal", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlatformerTraversalComponent> TraversalComponent;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UPlatformerInputConfig> InputConfig;

	UPROPERTY(EditAnywhere, Category="Input|Ladder", meta=(ClampMin="0.0"))
	float LadderLookInputThreshold = 0.1f;

	UPROPERTY(EditAnywhere, Category="Input|Combat")
	bool bUseUnifiedShotInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Dragon Dash", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDragonDashBounceDataAsset> DragonDashConfig;

	// Input Handlers
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	UInputAction* GetInputAction(const FGameplayTag& InputTag) const;
	void Input_JumpStart(const FInputActionValue& Value);
	void Input_JumpEnd(const FInputActionValue& Value);
	void Input_Dash(const FInputActionValue& Value);
	void Input_CrouchStart(const FInputActionValue& Value);
	void Input_CrouchEnd(const FInputActionValue& Value);
	void BeginCrouchRequest(const FInputActionValue& Value);
	void EndCrouchRequest(const FInputActionValue& Value);
	void Input_BaseShot(const FInputActionValue& Value);
	void Input_ChargeShotStart(const FInputActionValue& Value);
	void Input_ChargeShotEnd(const FInputActionValue& Value);
	void Input_FlyToggle(const FInputActionValue& Value);
	void Input_GlideStart(const FInputActionValue& Value);
	void Input_GlideEnd(const FInputActionValue& Value);
	class USideViewMovementComponent* GetSideViewMovementComponent() const;
	class UDragonFormComponent* GetDragonFormComponent() const;
	UPlatformerTraversalComponent* GetTraversalComponent() const;
	UPlatformerTraversalMovementComponent* GetTraversalMovementComponent() const;
	UDragonDashMovementComponent* GetDragonDashMovementComponent() const;
	void ApplyDeveloperChargeShotSettings(
		const FPlatformerChargeShotTuning& DeveloperChargeShotSettings,
		bool bHasSavedChargeShotSettings);
	void ApplyDeveloperTraversalSettings(
		const FPlatformerLedgeTraversalSettings& DeveloperLedgeSettings,
		const FPlatformerDashSettings& DeveloperDashSettings,
		const FPlatformerWallTraversalSettings& DeveloperWallSettings,
		bool bHasSavedTraversalSettings);

	// State trackers for temporary mechanics
	bool bIsGliding;
	bool bIsFlying;
	bool bCrouchActionHeld = false;
	bool bMoveCrouchHeld = false;
	bool bChargeShotInputHeld = false;
	float DefaultGravityScale;
	bool bHadPreGlideGravityOverride = false;
	float PreGlideGravityOverride = 1.0f;

	// Overrides for systemic handling
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnEnteredLadder(APlatformerLadder* Ladder) override;
	virtual void OnExitedLadder(APlatformerLadder* Ladder) override;
};
