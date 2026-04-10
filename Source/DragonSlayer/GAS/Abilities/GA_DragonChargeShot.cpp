#include "GAS/Abilities/GA_DragonChargeShot.h"

#include "AbilitySystemComponent.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Character/DragonOverdriveComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Projectiles/Combat/DragonProjectile.h"

UGA_DragonChargeShot::UGA_DragonChargeShot()
{
}

UAnimMontage* UGA_DragonChargeShot::GetChargeLoopMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ChargeLoopMontage;
}

bool UGA_DragonChargeShot::GetChargeShotTuning(const FGameplayAbilityActorInfo* ActorInfo, FPlatformerChargeShotTuning& OutChargeTuning) const
{
	const ADragonCharacter* Character = ActorInfo ? Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !Character->GetFormComponent())
	{
		return false;
	}

	return Character->GetFormComponent()->TryGetResolvedChargeShotTuning(OutChargeTuning);
}

bool UGA_DragonChargeShot::BuildChargeShotData(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FPlatformerChargeShotTuning& ChargeTuning,
	EPlatformerChargeShotStage ChargeStage,
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
	TSubclassOf<ADragonProjectile> ProjectileClass = nullptr;
	if (ChargeStage == EPlatformerChargeShotStage::None)
	{
		ProjectileClass = FormData->ProjectileClass;
		if (bOverdrive && FormData->OverdriveProjectileClass)
		{
			ProjectileClass = FormData->OverdriveProjectileClass;
		}
	}
	else
	{
		ProjectileClass = FormData->ChargeProjectileClass;
	}

	if (ChargeStage == EPlatformerChargeShotStage::Partial && FormData->PartialChargeProjectileClass)
	{
		ProjectileClass = FormData->PartialChargeProjectileClass;
	}

	if (!ProjectileClass)
	{
		return false;
	}

	FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddInstigator(Character, Character);

	const UPlatformerCharacterAttributeSet* PlatformerAttributeSet = Character->GetPlatformerAttributeSet();
	const float BaseShotDamage = PlatformerAttributeSet ? PlatformerAttributeSet->GetRangeBaseAttackDamage() : 0.0f;
	const float FullChargeDamage = PlatformerAttributeSet ? PlatformerAttributeSet->GetRangeChargedAttackDamage() : 0.0f;

	float ResolvedDamage = BaseShotDamage;
	float ProjectileSpeedMultiplier = 1.0f;
	if (ChargeStage == EPlatformerChargeShotStage::Full)
	{
		ResolvedDamage = FullChargeDamage;
		ProjectileSpeedMultiplier = ChargeTuning.FullProjectileSpeedMultiplier;
	}
	else if (ChargeStage == EPlatformerChargeShotStage::Partial)
	{
		const float PartialDamage = BaseShotDamage * ChargeTuning.PartialDamageMultiplier;
		ResolvedDamage = FullChargeDamage > 0.0f
			? FMath::Clamp(PartialDamage, BaseShotDamage, FMath::Max(BaseShotDamage, FullChargeDamage - KINDA_SMALL_NUMBER))
			: PartialDamage;
		ProjectileSpeedMultiplier = ChargeTuning.PartialProjectileSpeedMultiplier;
	}

	const FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f;
	const FRotator SpawnRotation = Character->GetActorRotation();
	OutShotData.ProjectileClass = ProjectileClass;
	OutShotData.SpawnLocation = SpawnLocation;
	OutShotData.SpawnRotation = SpawnRotation;
	OutShotData.ProjectileMaxDistance = Character->GetProjectileMaxDistance();

	if (ResolvedDamage > 0.0f)
	{
		FHitResult DamageHitResult;
		DamageHitResult.Location = SpawnLocation;
		DamageHitResult.ImpactPoint = SpawnLocation;
		OutShotData.DamageEffectSpec = Character->MakeCombatDamageEffectSpec(ResolvedDamage, DamageHitResult, nullptr, GetAbilityLevel(Handle, ActorInfo));
	}
	else if (ChargeDamageEffectClass)
	{
		OutShotData.DamageEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(ChargeDamageEffectClass, GetAbilityLevel(Handle, ActorInfo), ContextHandle);
	}

	if (FormData->OnHitStatusEffect)
	{
		OutShotData.StatusEffectSpec = Character->GetAbilitySystemComponent()->MakeOutgoingSpec(FormData->OnHitStatusEffect, GetAbilityLevel(Handle, ActorInfo), ContextHandle);
	}

	if (Character->HasActiveDeveloperCombatSettings())
	{
		const float BaseProjectileSpeed = Character->GetActiveDeveloperCombatSettings().DeveloperCombatRangeBaseAttackSpeed;
		const float ChargedProjectileSpeed = Character->GetActiveDeveloperCombatSettings().DeveloperCombatRangeChargedAttackSpeed;
		OutShotData.ProjectileSpeedOverride =
			(ChargeStage == EPlatformerChargeShotStage::None ? BaseProjectileSpeed : ChargedProjectileSpeed) * ProjectileSpeedMultiplier;
	}

	return OutShotData.IsValid();
}
