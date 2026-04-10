#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "GA_PlatformerChargeShot.generated.h"

class UAnimMontage;

/**
 * Generic hold-to-charge projectile ability flow.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UGA_PlatformerChargeShot : public UGA_PlatformerCombatAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlatformerChargeShot();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

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
	virtual bool GetChargeShotTuning(const FGameplayAbilityActorInfo* ActorInfo, FPlatformerChargeShotTuning& OutChargeTuning) const PURE_VIRTUAL(UGA_PlatformerChargeShot::GetChargeShotTuning, return false;);

	virtual bool BuildChargeShotData(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FPlatformerChargeShotTuning& ChargeTuning,
		EPlatformerChargeShotStage ChargeStage,
		FPlatformerProjectileShotData& OutShotData) const PURE_VIRTUAL(UGA_PlatformerChargeShot::BuildChargeShotData, return false;);

	virtual UAnimMontage* GetChargeLoopMontage(const FGameplayAbilityActorInfo* ActorInfo) const;
	virtual float GetChargeShotAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const;

private:
	void HandlePartialChargeReached();
	void HandleFullChargeReached();
	void UpdateChargeStageFromElapsedTime();
	void SetChargeStage(const FGameplayAbilityActorInfo* ActorInfo, const FPlatformerChargeShotTuning& ChargeTuning, EPlatformerChargeShotStage NewStage);
	void ClearChargeState(const FGameplayAbilityActorInfo* ActorInfo);

	FTimerHandle PartialChargeTimerHandle;
	FTimerHandle FullChargeTimerHandle;
	float ChargeStartTime = 0.0f;
	EPlatformerChargeShotStage CurrentChargeStage = EPlatformerChargeShotStage::None;
	bool bChargeReleased = false;
	mutable float LastChargedShotActivationTime = -1.0f;
};
