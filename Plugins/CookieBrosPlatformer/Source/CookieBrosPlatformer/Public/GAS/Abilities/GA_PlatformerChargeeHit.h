#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerBaseHit.h"
#include "GA_PlatformerChargeeHit.generated.h"

class UAnimMontage;

/**
 * Hold-to-charge melee attack. Name kept as requested for asset/API consistency.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UGA_PlatformerChargeeHit : public UGA_PlatformerBaseHit
{
	GENERATED_BODY()

public:
	UGA_PlatformerChargeeHit();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Melee|Charge", meta=(ClampMin="0.0", Units="s"))
	float ChargeTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Melee|Charge", meta=(ClampMin="1.0"))
	float ChargedDamageMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category="Melee|Charge|Animation")
	TObjectPtr<UAnimMontage> ChargeLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category="Melee|Charge|Cues")
	FGameplayTag FullyChargedCueTag;

private:
	void HandleChargeReached();
	void ClearChargeState(const FGameplayAbilityActorInfo* ActorInfo);

	FTimerHandle ChargeTimerHandle;
	bool bChargeReleased = false;
	bool bFullyCharged = false;
};
