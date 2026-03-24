#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "DragonProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

/**
 * ADragonProjectile
 * Base class for all projectiles fired by the player.
 */
UCLASS(abstract)
class BURNINGCORE_API ADragonProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ADragonProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionSphere;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> TrailVFX;
    
	// Set by the firing ability
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn=true), Category="Damage")
	FGameplayEffectSpecHandle DamageEffectSpec;
    
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn=true), Category="Damage")
	FGameplayEffectSpecHandle StatusEffectSpec; // Form-specific status
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile", meta=(ClampMin=0.0f, ClampMax=5000.0f, Units="cm/s"))
	float Speed = 2000.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile")
	bool bPiercing = false;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile", meta=(ClampMin=0.0f, ClampMax=10.0f, Units="s"))
	float Lifetime = 3.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    
	void ApplyDamageAndStatus(AActor* Target);
};
