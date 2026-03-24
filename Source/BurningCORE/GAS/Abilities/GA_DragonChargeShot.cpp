#include "GAS/Abilities/GA_DragonChargeShot.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "Combat/DragonProjectile.h"
#include "AbilitySystemComponent.h"
#include "BurningCORE.h"

UGA_DragonChargeShot::UGA_DragonChargeShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Shoot.Charge"))));
	// SetActivationOwnedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Action.Charging"))));
}

void UGA_DragonChargeShot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	// In full implementation, play ChargeLoopMontage here using WaitDelay/WaitInputRelease tasks
}

void UGA_DragonChargeShot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bIsCharging)
	{
		FireChargeShot();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_DragonChargeShot::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	bIsCharging = false;
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_DragonChargeShot::FireChargeShot()
{
	bIsCharging = false;
	float HoldDuration = GetWorld()->GetTimeSeconds() - ChargeStartTime;

	ADragonCharacter* Character = Cast<ADragonCharacter>(GetActorInfo().AvatarActor.Get());
	if (Character && Character->GetFormComponent())
	{
		const UDragonFormDataAsset* FormData = Character->GetFormComponent()->GetActiveFormData();
		if (FormData && HoldDuration >= FormData->ChargeTime)
		{
			// Full charge reached
			if (FormData->ChargeProjectileClass && ChargeDamageEffectClass)
			{
				FVector SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
				FRotator SpawnRot = Character->GetActorRotation();

				FActorSpawnParameters SpawnParams;
				SpawnParams.Instigator = Character;
				SpawnParams.Owner = Character;

				ADragonProjectile* Projectile = GetWorld()->SpawnActor<ADragonProjectile>(FormData->ChargeProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
				
				if (Projectile)
				{
					FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
					ContextHandle.AddInstigator(Character, Character);

					Projectile->DamageEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(ChargeDamageEffectClass, GetAbilityLevel(), ContextHandle);
					
					if (FormData->OnHitStatusEffect)
					{
						Projectile->StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(), ContextHandle);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogDragon, Verbose, TEXT("Charge released too early: %f < %f"), HoldDuration, FormData ? FormData->ChargeTime : 0.0f);
		}
	}
}
