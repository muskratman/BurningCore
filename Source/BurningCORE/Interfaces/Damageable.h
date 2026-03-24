#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayEffectTypes.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, NotBlueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IDamageable
 * Native interface for actors that can take damage via GAS.
 */
class BURNINGCORE_API IDamageable
{
	GENERATED_BODY()

public:
	/** Applies damage via gameplay effect spec */
	UFUNCTION(BlueprintCallable, Category="Damage")
	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) = 0;

	/** Returns true if health > 0 */
	UFUNCTION(BlueprintCallable, Category="Damage")
	virtual bool IsAlive() const = 0;
};
