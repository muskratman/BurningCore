#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "EnemyProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 * AEnemyProjectile
 * Base class for all hostile projectiles.
 */
UCLASS(abstract)
class BURNINGCORE_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionSphere;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn=true), Category="Damage")
	FGameplayEffectSpecHandle DamageEffectSpec;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile")
	float Speed = 1500.0f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile")
	float Lifetime = 5.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
