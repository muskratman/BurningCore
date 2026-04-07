#pragma once

#include "CoreMinimal.h"
#include "Character/DragonCharacter.h"
#include "GameplayTagContainer.h"
#include "PlayableDragonCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UGameplayAbility;
class APlatformerLadder;

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
	virtual void ApplyDeveloperCharacterMovementSettings(const struct FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings) override;

	// Enhanced Input setup
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category="Input|Combat")
	TObjectPtr<UInputAction> BaseShotAction;

	UPROPERTY(EditAnywhere, Category="Input|Combat")
	TObjectPtr<UInputAction> ChargeShotAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FlyAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> GlideAction;

	// Input Handlers
	void Input_Move(const FInputActionValue& Value);
	void Input_JumpStart(const FInputActionValue& Value);
	void Input_JumpEnd(const FInputActionValue& Value);
	void Input_Dash(const FInputActionValue& Value);
	void Input_CrouchStart(const FInputActionValue& Value);
	void Input_CrouchEnd(const FInputActionValue& Value);
	void Input_BaseShot(const FInputActionValue& Value);
	void Input_ChargeShotStart(const FInputActionValue& Value);
	void Input_ChargeShotEnd(const FInputActionValue& Value);
	void Input_FlyToggle(const FInputActionValue& Value);
	void Input_GlideStart(const FInputActionValue& Value);
	void Input_GlideEnd(const FInputActionValue& Value);
	class USideViewMovementComponent* GetSideViewMovementComponent() const;

	// Abilities to grant on possession for testing purposes
	UPROPERTY(EditAnywhere, Category="Abilities|Test Setup")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditAnywhere, Category="Abilities|Test Setup")
	TSubclassOf<UGameplayAbility> JumpAbilityClass;

	UPROPERTY(EditAnywhere, Category="Abilities|Test Setup")
	TSubclassOf<UGameplayAbility> CrouchAbilityClass;

	UPROPERTY(EditAnywhere, Category="Abilities|Test Setup")
	TSubclassOf<UGameplayAbility> BaseShotAbilityClass;

	UPROPERTY(EditAnywhere, Category="Abilities|Test Setup")
	TSubclassOf<UGameplayAbility> ChargeShotAbilityClass;

	// State trackers for temporary mechanics
	bool bIsGliding;
	bool bIsFlying;
	bool bLadderClimbUpHeld = false;
	bool bLadderClimbDownHeld = false;
	float DefaultGravityScale;
	bool bHadPreGlideGravityOverride = false;
	float PreGlideGravityOverride = 1.0f;

	// Overrides for systemic handling
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnEnteredLadder(APlatformerLadder* Ladder) override;
	virtual void OnExitedLadder(APlatformerLadder* Ladder) override;
};
