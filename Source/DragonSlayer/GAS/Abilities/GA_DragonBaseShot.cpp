#include "GAS/Abilities/GA_DragonBaseShot.h"

#include "AbilitySystemComponent.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Character/DragonOverdriveComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Projectiles/Combat/DragonProjectile.h"

UGA_DragonBaseShot::UGA_DragonBaseShot()
{
}

bool UGA_DragonBaseShot::BuildBaseShotData(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FPlatformerProjectileShotData& OutShotData) const
{
	ADragonCharacter* Character = ActorInfo ? Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !Character->GetFormComponent())
	{
		return false;
	}

	const UDragonFormDataAsset* FormData = Character->GetFormComponent()->GetActiveFormData();
	if (!FormData)
	{
		return false;
	}

	const bool bOverdrive = Character->GetOverdriveComponent() && Character->GetOverdriveComponent()->IsOverdriveActive();
	TSubclassOf<ADragonProjectile> ProjectileClass = FormData->ProjectileClass;
	if (bOverdrive && FormData->OverdriveProjectileClass)
	{
		ProjectileClass = FormData->OverdriveProjectileClass;
	}

	if (!ProjectileClass)
	{
		return false;
	}

	const FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
	const FRotator SpawnRotation = Character->GetActorRotation();

	OutShotData.ProjectileClass = ProjectileClass;
	OutShotData.SpawnLocation = SpawnLocation;
	OutShotData.SpawnRotation = SpawnRotation;
	OutShotData.ProjectileMaxDistance = Character->GetProjectileMaxDistance();

	FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddInstigator(Character, Character);

	const UPlatformerCharacterAttributeSet* PlatformerAttributeSet = Character->GetPlatformerAttributeSet();
	const float BaseShotDamage = PlatformerAttributeSet ? PlatformerAttributeSet->GetRangeBaseAttackDamage() : 0.0f;
	if (BaseShotDamage > 0.0f)
	{
		FHitResult DamageHitResult;
		DamageHitResult.Location = SpawnLocation;
		DamageHitResult.ImpactPoint = SpawnLocation;
		OutShotData.DamageEffectSpec = Character->MakeCombatDamageEffectSpec(BaseShotDamage, DamageHitResult, nullptr, GetAbilityLevel(Handle, ActorInfo));
	}
	else if (BaseDamageEffectClass)
	{
		OutShotData.DamageEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(BaseDamageEffectClass, GetAbilityLevel(Handle, ActorInfo), ContextHandle);
	}

	if (FormData->OnHitStatusEffect)
	{
		OutShotData.StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(Handle, ActorInfo), ContextHandle);
	}

	if (Character->HasActiveDeveloperCombatSettings())
	{
		OutShotData.ProjectileSpeedOverride = Character->GetActiveDeveloperCombatSettings().DeveloperCombatRangeBaseAttackSpeed;
	}

	return OutShotData.IsValid();
}
