#pragma once

#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/Damageable.h"
#include "PlatformerCharacterBase.generated.h"

class UAbilitySystemComponent;
class UPlatformerAbilitySet;
class UPlatformerCharacterAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class APlatformerLadder;

/**
 * APlatformerCharacterBase
 * Generic playable platformer shell with camera rig, side-view movement, and GAS bootstrap.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerCharacterBase : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCamera;

protected:
	UPROPERTY()
	TObjectPtr<UPlatformerCharacterAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category="Abilities")
	TObjectPtr<UPlatformerAbilitySet> DefaultAbilitySet;

public:
	APlatformerCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) override;
	virtual bool IsAlive() const override;

	virtual void InitializeAbilities(const UPlatformerAbilitySet* AbilitySet);
	virtual void NotifyLadderAvailable(APlatformerLadder* Ladder);
	virtual void NotifyLadderUnavailable(APlatformerLadder* Ladder);
	virtual bool EnterLadder(APlatformerLadder* Ladder);
	virtual void ExitLadder(APlatformerLadder* Ladder = nullptr);
	virtual void ApplyDeveloperCharacterSettings(const FDeveloperPlatformerCharacterSettings& DeveloperSettings);
	virtual FDeveloperPlatformerCharacterSettings CaptureDeveloperCharacterSettings() const;

	FORCEINLINE UPlatformerCharacterAttributeSet* GetPlatformerAttributeSet() const { return AttributeSet; }
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
	virtual void OnEnteredLadder(APlatformerLadder* Ladder);
	virtual void OnExitedLadder(APlatformerLadder* Ladder);
	virtual void ApplyDeveloperCameraSettings(const FDeveloperPlatformerCameraSettings& DeveloperCameraSettings);
	virtual FDeveloperPlatformerCameraSettings CaptureDeveloperCameraSettings() const;
	virtual void ApplyDeveloperCharacterMovementSettings(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings);
	virtual FDeveloperPlatformerCharacterMovementSettings CaptureDeveloperCharacterMovementSettings() const;
	virtual void ApplyDeveloperCombatSettings(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings);
	virtual FDeveloperPlatformerCombatSettings CaptureDeveloperCombatSettings() const;
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
};
