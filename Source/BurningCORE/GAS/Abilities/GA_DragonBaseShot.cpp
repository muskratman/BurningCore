#include "GAS/Abilities/GA_DragonBaseShot.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Character/DragonOverdriveComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "Projectiles/BaseProjectile.h"
#include "AbilitySystemComponent.h"
#include "BurningCORE.h"

UGA_DragonBaseShot::UGA_DragonBaseShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Shoot.Base"))));
}

void UGA_DragonBaseShot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
		if (FormData)
		{
			bool bOverdrive = Character->GetOverdriveComponent() && Character->GetOverdriveComponent()->IsOverdriveActive();
			
			TSubclassOf<ABaseProjectile> ProjClass = FormData->ProjectileClass;
			if (bOverdrive && FormData->OverdriveProjectileClass)
			{
				ProjClass = FormData->OverdriveProjectileClass;
			}

			if (ProjClass)
			{
				// In a full implementation, we spawn at a socket, but the basic architecture:
				FVector SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
				FRotator SpawnRot = Character->GetActorRotation();

				FActorSpawnParameters SpawnParams;
				SpawnParams.Instigator = Character;
				SpawnParams.Owner = Character;

				ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjClass, SpawnLoc, SpawnRot, SpawnParams);
				
				if (Projectile)
				{
					// Apply effect specs to projectile
					FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
					ContextHandle.AddInstigator(Character, Character);

					if (BaseDamageEffectClass)
					{
						FGameplayEffectSpecHandle DamageSpecHandle = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(BaseDamageEffectClass, GetAbilityLevel(), ContextHandle);
						
						if (bOverdrive)
						{
							// In full impl, modify spec damage by FormData->OverdriveDamageMultiplier
						}

						Projectile->DamageEffectSpec = DamageSpecHandle;
					}
					
					if (FormData->OnHitStatusEffect)
					{
						Projectile->StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(), ContextHandle);
					}
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
