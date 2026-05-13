#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Platformer/Character/DragonDashMovementComponent.h"
#include "GA_DragonDash.generated.h"

/**
 * Dragon-specific dash ability. It allows ground/air directional dash and
 * delegates movement execution to UDragonDashMovementComponent.
 */
UCLASS()
class DRAGONSLAYER_API UGA_DragonDash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_DragonDash();

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

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UFUNCTION()
	void HandleDragonDashStateChanged(EDragonDashState PreviousState, EDragonDashState NewState);

	UDragonDashMovementComponent* GetDragonDashMovementComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
};
