#include "GAS/Abilities/GA_PlatformerChargeeHit.h"

#include "Animation/PlatformerAnimGameplayTags.h"
#include "Character/PlatformerCharacterBase.h"
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

	PlayAbilityAnimation(ActorInfo,
		PlatformerAnimGameplayTags::Anim_Combat_MeleeChargeLoop,
		ChargeLoopMontage);

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

	const APlatformerCharacterBase* PlatformerCharacter = GetPlatformerCharacter(ActorInfo);
	if (PlatformerCharacter && (PlatformerCharacter->IsOnLadder() || PlatformerCharacter->IsLadderTopFinishActive()))
	{
		ClearChargeState(ActorInfo);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StopAbilityAnimation(ActorInfo,
		PlatformerAnimGameplayTags::Anim_Combat_MeleeChargeLoop,
		ChargeLoopMontage);

	PlayAbilityAnimation(ActorInfo,
		PlatformerAnimGameplayTags::Anim_Combat_MeleeHit,
		const_cast<UAnimMontage*>(GetMeleeHitSettings().AttackMontage.Get()));

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

	StopAbilityAnimation(ActorInfo,
		PlatformerAnimGameplayTags::Anim_Combat_MeleeChargeLoop,
		ChargeLoopMontage);

	bChargeReleased = false;
	bFullyCharged = false;
}
