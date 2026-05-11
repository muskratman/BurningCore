#include "GAS/Abilities/GA_PlatformerCombatAbilityBase.h"

#include "AbilitySystemComponent.h"
#include "Animation/PlatformerAnimDataAsset.h"
#include "Animation/PlatformerAnimInstance.h"
#include "Character/PlatformerCharacterBase.h"
#include "Combat/PlatformerCombatCharacterBase.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "Projectiles/Combat/CombatProjectile.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogPlatformerCombatAbility);

APlatformerCombatCharacterBase* UGA_PlatformerCombatAbilityBase::GetPlatformerCombatCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? Cast<APlatformerCombatCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
}

APlatformerCharacterBase* UGA_PlatformerCombatAbilityBase::GetPlatformerCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? Cast<APlatformerCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
}

const UPlatformerCharacterAttributeSet* UGA_PlatformerCombatAbilityBase::GetPlatformerAttributeSet(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (const APlatformerCombatCharacterBase* Character = GetPlatformerCombatCharacter(ActorInfo))
	{
		return Character->GetPlatformerAttributeSet();
	}

	return nullptr;
}

UAbilitySystemComponent* UGA_PlatformerCombatAbilityBase::GetPlatformerAbilitySystem(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (const APlatformerCombatCharacterBase* Character = GetPlatformerCombatCharacter(ActorInfo))
	{
		return Character->GetAbilitySystemComponent();
	}

	return nullptr;
}

float UGA_PlatformerCombatAbilityBase::GetAbilityWorldTime(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	const UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	return World ? World->GetTimeSeconds() : 0.0f;
}

bool UGA_PlatformerCombatAbilityBase::PassesActivationDelay(const FGameplayAbilityActorInfo* ActorInfo, float DelaySeconds, float LastActivationTime) const
{
	if (DelaySeconds <= 0.0f)
	{
		return true;
	}

	return (GetAbilityWorldTime(ActorInfo) - LastActivationTime) >= DelaySeconds;
}

void UGA_PlatformerCombatAbilityBase::ExecuteGameplayCue(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTag& CueTag) const
{
	if (!CueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetPlatformerAbilitySystem(ActorInfo))
	{
		FGameplayCueParameters CueParameters;
		if (const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)
		{
			CueParameters.Location = AvatarActor->GetActorLocation();
		}

		AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
	}
}

ACombatProjectile* UGA_PlatformerCombatAbilityBase::SpawnConfiguredCombatProjectile(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FPlatformerProjectileShotData& ShotData) const
{
	APlatformerCombatCharacterBase* Character = GetPlatformerCombatCharacter(ActorInfo);
	UWorld* World = Character ? Character->GetWorld() : nullptr;
	if (!World || !Character || !ShotData.IsValid())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	ACombatProjectile* Projectile = World->SpawnActor<ACombatProjectile>(
		ShotData.ProjectileClass,
		ShotData.SpawnLocation,
		ShotData.SpawnRotation,
		SpawnParams);
	if (!Projectile)
	{
		return nullptr;
	}

	Projectile->DamageEffectSpec = ShotData.DamageEffectSpec;
	Projectile->StatusEffectSpec = ShotData.StatusEffectSpec;

	if (ShotData.ProjectileMaxDistance > 0.0f)
	{
		Projectile->ApplyProjectileMaxDistance(ShotData.ProjectileMaxDistance);
	}

	if (ShotData.ProjectileSpeedOverride > 0.0f)
	{
		Projectile->ApplyDeveloperProjectileSpeed(ShotData.ProjectileSpeedOverride);
	}

	return Projectile;
}

bool UGA_PlatformerCombatAbilityBase::PerformMeleeHit(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FPlatformerMeleeHitSettings& HitSettings,
	float DamageAmount,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float EffectLevel) const
{
	APlatformerCombatCharacterBase* Character = GetPlatformerCombatCharacter(ActorInfo);
	UWorld* World = Character ? Character->GetWorld() : nullptr;
	if (!World || !Character || !HitSettings.bEnabled || DamageAmount <= 0.0f)
	{
		return false;
	}

	const FVector ForwardVector = Character->GetActorForwardVector().GetSafeNormal2D().IsNearlyZero()
		? FVector::ForwardVector
		: Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector Start = Character->GetActorLocation()
		+ FVector(0.0f, 0.0f, HitSettings.TraceVerticalOffset)
		+ ForwardVector * HitSettings.TraceForwardOffset;
	const FVector End = Start + ForwardVector * HitSettings.TraceDistance;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlatformerMeleeHit), false, Character);

	TArray<FHitResult> HitResults;
	const bool bHit = World->SweepMultiByObjectType(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(HitSettings.TraceRadius),
		QueryParams);

	if (!bHit)
	{
		return false;
	}

	bool bAppliedAnyDamage = false;
	TSet<const AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* TargetActor = HitResult.GetActor();
		if (!TargetActor || TargetActor == Character || HitActors.Contains(TargetActor))
		{
			continue;
		}

		HitActors.Add(TargetActor);
		if (Character->ApplyCombatDamageToActor(TargetActor, DamageAmount, HitResult, DamageEffectClass, EffectLevel))
		{
			bAppliedAnyDamage = true;
		}

		if (!HitSettings.bHitMultipleTargets)
		{
			break;
		}
	}

	return bAppliedAnyDamage;
}

UPlatformerAnimInstance* UGA_PlatformerCombatAbilityBase::GetPlatformerAnimInstance(const FGameplayAbilityActorInfo* ActorInfo) const
{
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character)
	{
		return nullptr;
	}

	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		return Cast<UPlatformerAnimInstance>(Mesh->GetAnimInstance());
	}

	return nullptr;
}

UAnimMontage* UGA_PlatformerCombatAbilityBase::ResolveAbilityMontage(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTag& AnimTag,
	UAnimMontage* FallbackMontage) const
{
	// Path 1: PlatformerAnimInstance.AnimData (preferred — AnimBP can override the asset).
	if (UPlatformerAnimInstance* AnimInstance = GetPlatformerAnimInstance(ActorInfo))
	{
		if (UAnimMontage* Montage = AnimInstance->ResolveAbilityMontage(AnimTag))
		{
			return Montage;
		}
	}

	// Path 2: Character's AnimDataAsset directly. Works when the AnimBP does not
	// inherit from UPlatformerAnimInstance (e.g. project uses a vanilla UAnimInstance).
	if (const APlatformerCharacterBase* PlatformerCharacter = GetPlatformerCharacter(ActorInfo))
	{
		if (const UPlatformerAnimDataAsset* AnimDataAsset = PlatformerCharacter->GetAnimDataAsset())
		{
			if (UAnimMontage* Montage = AnimDataAsset->FindMontage(AnimTag))
			{
				return Montage;
			}
		}
	}

	// Path 3: caller-supplied fallback.
	return FallbackMontage;
}

float UGA_PlatformerCombatAbilityBase::PlayAbilityAnimation(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTag& AnimTag,
	UAnimMontage* FallbackMontage,
	float PlayRate) const
{
	const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	const FString AvatarName = AvatarActor ? AvatarActor->GetName() : TEXT("<null avatar>");

	UAnimMontage* Montage = ResolveAbilityMontage(ActorInfo, AnimTag, FallbackMontage);
	if (!Montage)
	{
		UE_LOG(LogPlatformerCombatAbility, Warning,
			TEXT("PlayAbilityAnimation[%s]: no montage resolved for tag '%s'. Check (1) DA entry for the tag, (2) AnimDataAsset assigned on character or AnimBP, (3) fallback montage."),
			*AvatarName, *AnimTag.ToString());
		return 0.0f;
	}

	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character)
	{
		UE_LOG(LogPlatformerCombatAbility, Warning,
			TEXT("PlayAbilityAnimation[%s]: tag '%s' resolved to montage '%s', but avatar is not an ACharacter — cannot play."),
			*AvatarName, *AnimTag.ToString(), *Montage->GetName());
		return 0.0f;
	}

	float Duration = 0.0f;
	if (UAbilitySystemComponent* AbilitySystemComponent = GetPlatformerAbilitySystem(ActorInfo))
	{
		Duration = AbilitySystemComponent->PlayMontage(
			const_cast<UGA_PlatformerCombatAbilityBase*>(this),
			GetCurrentActivationInfo(),
			Montage,
			PlayRate);
	}
	else
	{
		Duration = Character->PlayAnimMontage(Montage, PlayRate);
	}

	UE_LOG(LogPlatformerCombatAbility, Log,
		TEXT("PlayAbilityAnimation[%s]: tag '%s' -> montage '%s' (rate=%.2f, duration=%.2fs). If anim is invisible despite duration>0, check montage slot/skeleton vs AnimBP."),
		*AvatarName, *AnimTag.ToString(), *Montage->GetName(), PlayRate, Duration);
	return Duration;
}

void UGA_PlatformerCombatAbilityBase::StopAbilityAnimation(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTag& AnimTag,
	UAnimMontage* FallbackMontage,
	float BlendOutTime) const
{
	UAnimMontage* Montage = ResolveAbilityMontage(ActorInfo, AnimTag, FallbackMontage);
	if (!Montage)
	{
		return;
	}

	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (Character)
	{
		Character->StopAnimMontage(Montage);
	}
}
