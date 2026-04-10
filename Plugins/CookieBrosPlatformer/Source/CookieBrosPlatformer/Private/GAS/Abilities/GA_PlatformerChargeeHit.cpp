#include "GAS/Abilities/GA_PlatformerChargeeHit.h"

#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

UGA_PlatformerChargeeHit::UGA_PlatformerChargeeHit()
{
	ActivationOwnedTags.AddTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
}

void UGA_PlatformerChargeeHit::ActivateAbility(
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

	bChargeReleased = false;
	bFullyCharged = ChargeTime <= 0.0f;

	if (ChargeLoopMontage)
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->PlayAnimMontage(ChargeLoopMontage);
		}
	}

	if (bFullyCharged)
	{
		if (FullyChargedCueTag.IsValid())
		{
			ExecuteGameplayCue(ActorInfo, FullyChargedCueTag);
		}
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ChargeTimerHandle,
			this,
			&UGA_PlatformerChargeeHit::HandleChargeReached,
			ChargeTime,
			false);
	}
}

void UGA_PlatformerChargeeHit::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bChargeReleased)
	{
		return;
	}

	bChargeReleased = true;

	if (ChargeLoopMontage)
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->StopAnimMontage(ChargeLoopMontage);
		}
	}

	if (const UAnimMontage* AttackMontage = GetMeleeHitSettings().AttackMontage)
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->PlayAnimMontage(const_cast<UAnimMontage*>(AttackMontage));
		}
	}

	const float DamageMultiplier = bFullyCharged ? ChargedDamageMultiplier : 1.0f;
	const bool bHitTarget = ExecuteConfiguredMeleeHit(Handle, ActorInfo, DamageMultiplier);
	LastMeleeActivationTime = GetAbilityWorldTime(ActorInfo);

	if (bHitTarget && GetMeleeHitSettings().HitCueTag.IsValid())
	{
		ExecuteGameplayCue(ActorInfo, GetMeleeHitSettings().HitCueTag);
	}

	ClearChargeState(ActorInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_PlatformerChargeeHit::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UGA_PlatformerChargeeHit::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	ClearChargeState(ActorInfo);
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_PlatformerChargeeHit::HandleChargeReached()
{
	bFullyCharged = true;

	if (FullyChargedCueTag.IsValid())
	{
		ExecuteGameplayCue(GetCurrentActorInfo(), FullyChargedCueTag);
	}
}

void UGA_PlatformerChargeeHit::ClearChargeState(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTimerHandle);
	}

	if (ChargeLoopMontage)
	{
		if (ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr)
		{
			Character->StopAnimMontage(ChargeLoopMontage);
		}
	}

	bChargeReleased = false;
	bFullyCharged = false;
}
