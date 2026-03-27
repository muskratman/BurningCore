#include "GAS/Abilities/GA_DragonChargeShot.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "Projectiles/BaseProjectile.h"
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

	ADragonCharacter* Character = Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetFormComponent())
	{
		const UDragonFormDataAsset* FormData = Character->GetFormComponent()->GetActiveFormData();
		if (FormData && FormData->ChargeProjectileClass)
		{
			FVector SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
			FRotator SpawnRot = Character->GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = Character;
			SpawnParams.Owner = Character;

			ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(FormData->ChargeProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
			
			if (Projectile)
			{
				FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
				ContextHandle.AddInstigator(Character, Character);

				if (ChargeDamageEffectClass)
				{
					Projectile->DamageEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(ChargeDamageEffectClass, GetAbilityLevel(), ContextHandle);
				}
				
				if (FormData->OnHitStatusEffect)
				{
					Projectile->StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(), ContextHandle);
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
