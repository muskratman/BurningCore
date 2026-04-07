#include "GAS/Abilities/GA_DragonChargeShot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/Effects/DeveloperDamageGameplayEffect.h"
#include "Projectiles/Combat/DragonProjectile.h"

UGA_DragonChargeShot::UGA_DragonChargeShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Shoot.Charge"))));
	// SetActivationOwnedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Action.Charging"))));
}

bool UGA_DragonChargeShot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ADragonCharacter* Character = ActorInfo ? Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !Character->HasActiveDeveloperCombatSettings())
	{
		return true;
	}

	const UPlatformerCharacterAttributeSet* PlatformerAttributeSet = Character->GetPlatformerAttributeSet();
	if (!PlatformerAttributeSet)
	{
		return true;
	}

	const float DeveloperRangeAttackDelay = PlatformerAttributeSet->GetRangeAttackDelay();
	if (DeveloperRangeAttackDelay <= 0.0f)
	{
		return true;
	}

	const UWorld* World = Character->GetWorld();
	if (!World)
	{
		return true;
	}

	return (World->GetTimeSeconds() - LastDeveloperChargedShotActivationTime) >= DeveloperRangeAttackDelay;
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
		if (UWorld* World = Character->GetWorld())
		{
			LastDeveloperChargedShotActivationTime = World->GetTimeSeconds();
		}

		const UDragonFormDataAsset* FormData = Character->GetFormComponent()->GetActiveFormData();
		if (FormData && FormData->ChargeProjectileClass)
		{
			const FVector SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
			const FRotator SpawnRot = Character->GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = Character;
			SpawnParams.Owner = Character;

			ADragonProjectile* Projectile = GetWorld()->SpawnActor<ADragonProjectile>(FormData->ChargeProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
			if (Projectile)
			{
				FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
				ContextHandle.AddInstigator(Character, Character);

				const UPlatformerCharacterAttributeSet* PlatformerAttributeSet = Character->GetPlatformerAttributeSet();
				const bool bUseDeveloperCombatDamage =
					Character->HasActiveDeveloperCombatSettings()
					&& PlatformerAttributeSet
					&& PlatformerAttributeSet->GetRangeChargedAttackDamage() > 0.0f;

				if (bUseDeveloperCombatDamage)
				{
					FGameplayEffectSpecHandle DamageSpecHandle =
						Character->GetAbilitySystemComponent()->MakeOutgoingSpec(UDeveloperDamageGameplayEffect::StaticClass(), GetAbilityLevel(), ContextHandle);
					if (DamageSpecHandle.IsValid())
					{
						UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
							DamageSpecHandle,
							UDeveloperDamageGameplayEffect::GetDeveloperDamageSetByCallerTag(),
							PlatformerAttributeSet->GetRangeChargedAttackDamage());
						Projectile->DamageEffectSpec = DamageSpecHandle;
					}
				}
				else if (ChargeDamageEffectClass)
				{
					Projectile->DamageEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(ChargeDamageEffectClass, GetAbilityLevel(), ContextHandle);
				}

				if (FormData->OnHitStatusEffect)
				{
					Projectile->StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(), ContextHandle);
				}

				if (Character->HasActiveDeveloperCombatSettings())
				{
					Projectile->ApplyDeveloperProjectileSpeed(Character->GetActiveDeveloperCombatSettings().DeveloperCombatRangeChargedAttackSpeed);
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
