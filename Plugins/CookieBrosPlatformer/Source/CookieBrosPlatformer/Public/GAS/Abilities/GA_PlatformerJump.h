#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlatformerJump.generated.h"

UENUM(BlueprintType)
enum class EPlatformerJumpHorizontalSpeedMode : uint8
{
	None = 0,
	ClampMin,
	Override
};

/**
 * UGA_PlatformerJump
 * GAS Ability wrapping the default character jump to correctly apply Movement.Airborne and Ability.Movement.Jump tags.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UGA_PlatformerJump : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlatformerJump();
	FORCEINLINE float GetJumpMaxHoldTime() const { return JumpMaxHoldTime; }
	FORCEINLINE float GetJumpHorizontalSpeed() const { return JumpHorizontalSpeed; }
	float ResolvePreviewHorizontalJumpSpeed(const class ACharacter* Character, float CurrentHorizontalSpeed, float DirectionSign) const;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Jump", meta=(ClampMin="0.0", Units="s"))
	float JumpMaxHoldTime = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Horizontal")
	EPlatformerJumpHorizontalSpeedMode JumpHorizontalSpeedMode = EPlatformerJumpHorizontalSpeedMode::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Jump|Horizontal", meta=(ClampMin="0.0", Units="cm/s"))
	float JumpHorizontalSpeed = 0.0f;

	void ApplyHorizontalJumpSpeedDeferred(TWeakObjectPtr<class ACharacter> Character, float DirectionSign);
	virtual void ApplyHorizontalJumpSpeed(class ACharacter* Character, float DirectionSign) const;
	virtual EPlatformerJumpHorizontalSpeedMode ResolveHorizontalJumpSpeedMode(const class ACharacter* Character) const;
	virtual float ResolveHorizontalJumpSpeed(const class ACharacter* Character) const;
	virtual float ResolveHorizontalJumpDirection(const class ACharacter* Character) const;
};
