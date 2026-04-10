#include "GAS/Abilities/GA_PlatformerChargeShot.h"

#include "AbilitySystemComponent.h"
#include "Character/PlatformerCharacterBase.h"
#include "GameFramework/Character.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "TimerManager.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_PlatformerChargeShot::UGA_PlatformerChargeShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_SlideDash);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	ActivationBlockedTags.AddTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
}

bool UGA_PlatformerChargeShot::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return PassesActivationDelay(ActorInfo, GetChargeShotAttackDelay(ActorInfo), LastChargedShotActivationTime);
}

void UGA_PlatformerChargeShot::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FPlatformerChargeShotTuning ChargeTuning;
	if (!GetChargeShotTuning(ActorInfo, ChargeTuning))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ChargeStartTime = GetAbilityWorldTime(ActorInfo);
	CurrentChargeStage = EPlatformerChargeShotStage::None;
	bChargeReleased = false;

	if (UAnimMontage* ChargeMontage = GetChargeLoopMontage(ActorInfo))
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->PlayAnimMontage(ChargeMontage);
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (ChargeTuning.GetSafePartialChargeTime() > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				PartialChargeTimerHandle,
				this,
				&UGA_PlatformerChargeShot::HandlePartialChargeReached,
				ChargeTuning.GetSafePartialChargeTime(),
				false);
		}

		if (ChargeTuning.GetSafeFullChargeTime() > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				FullChargeTimerHandle,
				this,
				&UGA_PlatformerChargeShot::HandleFullChargeReached,
				ChargeTuning.GetSafeFullChargeTime(),
				false);
		}
	}
}

void UGA_PlatformerChargeShot::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bChargeReleased)
	{
		return;
	}

	bChargeReleased = true;
	UpdateChargeStageFromElapsedTime();

	FPlatformerChargeShotTuning ChargeTuning;
	const bool bHasChargeTuning = GetChargeShotTuning(ActorInfo, ChargeTuning);

	bool bShotFired = false;
	if (bHasChargeTuning)
	{
		FPlatformerProjectileShotData ShotData;
		if (BuildChargeShotData(Handle, ActorInfo, ChargeTuning, CurrentChargeStage, ShotData)
			&& SpawnConfiguredCombatProjectile(ActorInfo, ShotData))
		{
			LastChargedShotActivationTime = GetAbilityWorldTime(ActorInfo);
			bShotFired = true;
		}
	}

	if (bShotFired && bHasChargeTuning && ChargeTuning.ReleaseCueTag.IsValid())
	{
		ExecuteGameplayCue(ActorInfo, ChargeTuning.ReleaseCueTag);
	}

	ClearChargeState(ActorInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bShotFired);
}

void UGA_PlatformerChargeShot::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UGA_PlatformerChargeShot::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	ClearChargeState(ActorInfo);
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

UAnimMontage* UGA_PlatformerChargeShot::GetChargeLoopMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return nullptr;
}

float UGA_PlatformerChargeShot::GetChargeShotAttackDelay(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const APlatformerCharacterBase* Character = GetPlatformerCharacter(ActorInfo);
	const UPlatformerCharacterAttributeSet* AttributeSet = GetPlatformerAttributeSet(ActorInfo);
	if (!Character || !Character->HasActiveDeveloperCombatSettings() || !AttributeSet)
	{
		return 0.0f;
	}

	return FMath::Max(AttributeSet->GetRangeAttackDelay(), 0.0f);
}

void UGA_PlatformerChargeShot::HandlePartialChargeReached()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	FPlatformerChargeShotTuning ChargeTuning;
	if (ActorInfo && GetChargeShotTuning(ActorInfo, ChargeTuning))
	{
		SetChargeStage(ActorInfo, ChargeTuning, EPlatformerChargeShotStage::Partial);
	}
}

void UGA_PlatformerChargeShot::HandleFullChargeReached()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	FPlatformerChargeShotTuning ChargeTuning;
	if (ActorInfo && GetChargeShotTuning(ActorInfo, ChargeTuning))
	{
		SetChargeStage(ActorInfo, ChargeTuning, EPlatformerChargeShotStage::Full);
	}
}

void UGA_PlatformerChargeShot::UpdateChargeStageFromElapsedTime()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	FPlatformerChargeShotTuning ChargeTuning;
	if (!ActorInfo || !GetChargeShotTuning(ActorInfo, ChargeTuning))
	{
		return;
	}

	const float ElapsedChargeTime = GetAbilityWorldTime(ActorInfo) - ChargeStartTime;
	EPlatformerChargeShotStage EvaluatedStage = EPlatformerChargeShotStage::None;
	if (ElapsedChargeTime >= ChargeTuning.GetSafeFullChargeTime())
	{
		EvaluatedStage = EPlatformerChargeShotStage::Full;
	}
	else if (ElapsedChargeTime >= ChargeTuning.GetSafePartialChargeTime())
	{
		EvaluatedStage = EPlatformerChargeShotStage::Partial;
	}

	SetChargeStage(ActorInfo, ChargeTuning, EvaluatedStage);
}

void UGA_PlatformerChargeShot::SetChargeStage(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FPlatformerChargeShotTuning& ChargeTuning,
	EPlatformerChargeShotStage NewStage)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetPlatformerAbilitySystem(ActorInfo);
	if (!AbilitySystemComponent || CurrentChargeStage == NewStage)
	{
		return;
	}

	if (CurrentChargeStage == EPlatformerChargeShotStage::Partial)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargePartial);
	}
	else if (CurrentChargeStage == EPlatformerChargeShotStage::Full)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargeFull);
	}

	CurrentChargeStage = NewStage;

	if (CurrentChargeStage == EPlatformerChargeShotStage::Partial)
	{
		AbilitySystemComponent->AddLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargePartial);
		if (ChargeTuning.PartialChargeCueTag.IsValid())
		{
			ExecuteGameplayCue(ActorInfo, ChargeTuning.PartialChargeCueTag);
		}
	}
	else if (CurrentChargeStage == EPlatformerChargeShotStage::Full)
	{
		AbilitySystemComponent->AddLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargeFull);
		if (ChargeTuning.FullChargeCueTag.IsValid())
		{
			ExecuteGameplayCue(ActorInfo, ChargeTuning.FullChargeCueTag);
		}
	}
}

void UGA_PlatformerChargeShot::ClearChargeState(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PartialChargeTimerHandle);
		World->GetTimerManager().ClearTimer(FullChargeTimerHandle);
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetPlatformerAbilitySystem(ActorInfo))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargePartial);
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargeFull);
	}

	if (UAnimMontage* ChargeMontage = GetChargeLoopMontage(ActorInfo))
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->StopAnimMontage(ChargeMontage);
		}
	}

	CurrentChargeStage = EPlatformerChargeShotStage::None;
	ChargeStartTime = 0.0f;
	bChargeReleased = false;
}
