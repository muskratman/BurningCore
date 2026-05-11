#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Logging/LogMacros.h"
#include "Projectiles/Combat/CombatProjectile.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlatformerCombatAbilityBase.generated.h"

// Shared log category for combat-ability diagnostics (animation resolution,
// projectile spawn failures, etc). Defined in GA_PlatformerCombatAbilityBase.cpp.
COOKIEBROSPLATFORMER_API DECLARE_LOG_CATEGORY_EXTERN(LogPlatformerCombatAbility, Log, All);

class APlatformerCombatCharacterBase;
class APlatformerCharacterBase;
class UAbilitySystemComponent;
class UGameplayEffect;
class UPlatformerCharacterAttributeSet;
class UPlatformerAnimInstance;
class UAnimMontage;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerMeleeHitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee", meta=(ClampMin="0.0", Units="cm"))
	float TraceDistance = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee", meta=(ClampMin="0.0", Units="cm"))
	float TraceRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee", meta=(ClampMin="0.0", Units="cm"))
	float TraceForwardOffset = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee", meta=(Units="cm"))
	float TraceVerticalOffset = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	bool bHitMultipleTargets = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Cues")
	FGameplayTag HitCueTag;
};

struct COOKIEBROSPLATFORMER_API FPlatformerProjectileShotData
{
	TSubclassOf<ACombatProjectile> ProjectileClass;
	FGameplayEffectSpecHandle DamageEffectSpec;
	FGameplayEffectSpecHandle StatusEffectSpec;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	float ProjectileSpeedOverride = 0.0f;
	float ProjectileMaxDistance = 0.0f;

	bool IsValid() const
	{
		return ProjectileClass.Get() != nullptr;
	}
};

/**
 * Shared GAS helpers for combat-oriented platformer abilities.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UGA_PlatformerCombatAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	APlatformerCombatCharacterBase* GetPlatformerCombatCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;
	APlatformerCharacterBase* GetPlatformerCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;
	const UPlatformerCharacterAttributeSet* GetPlatformerAttributeSet(const FGameplayAbilityActorInfo* ActorInfo) const;
	UAbilitySystemComponent* GetPlatformerAbilitySystem(const FGameplayAbilityActorInfo* ActorInfo) const;
	float GetAbilityWorldTime(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool PassesActivationDelay(const FGameplayAbilityActorInfo* ActorInfo, float DelaySeconds, float LastActivationTime) const;
	void ExecuteGameplayCue(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTag& CueTag) const;
	ACombatProjectile* SpawnConfiguredCombatProjectile(const FGameplayAbilityActorInfo* ActorInfo, const FPlatformerProjectileShotData& ShotData) const;
	bool PerformMeleeHit(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FPlatformerMeleeHitSettings& HitSettings,
		float DamageAmount,
		TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr,
		float EffectLevel = 1.0f) const;

	/** Get the platformer AnimInstance (if the character's AnimBP inherits UPlatformerAnimInstance). */
	UPlatformerAnimInstance* GetPlatformerAnimInstance(const FGameplayAbilityActorInfo* ActorInfo) const;

	/**
	 * Resolve a montage for AnimTag with fallbacks, without playing it.
	 * Priority:
	 *   1. UPlatformerAnimInstance::AnimData (when AnimBP inherits the platformer base).
	 *   2. APlatformerCharacterBase::AnimDataAsset (works for any AnimBP class).
	 *   3. Caller-supplied FallbackMontage.
	 */
	UAnimMontage* ResolveAbilityMontage(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTag& AnimTag,
		UAnimMontage* FallbackMontage = nullptr) const;

	/**
	 * Play a montage via data-driven lookup.
	 * Logs a warning when the tag cannot be resolved through any path.
	 * @return Duration of the played montage, or 0 if nothing was played.
	 */
	float PlayAbilityAnimation(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTag& AnimTag,
		UAnimMontage* FallbackMontage = nullptr,
		float PlayRate = 1.0f) const;

	/** Stop a montage via data-driven lookup, with fallback. */
	void StopAbilityAnimation(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTag& AnimTag,
		UAnimMontage* FallbackMontage = nullptr,
		float BlendOutTime = 0.25f) const;
};
