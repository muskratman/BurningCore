#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * ABaseProjectile
 * Root class for all projectiles in BurningCORE.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	UFUNCTION(BlueprintCallable, Category="Developer")
	void ApplyDeveloperProjectileSpeed(float DeveloperProjectileSpeed);

	UFUNCTION(BlueprintPure, Category="Developer")
	float GetDeveloperProjectileSpeed() const { return Speed; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void HandleImpact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile", meta=(ClampMin=0.0f, ClampMax=5000.0f, Units="cm/s"))
	float Speed = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Projectile", meta=(ClampMin=0.0f, ClampMax=10.0f, Units="s"))
	float Lifetime = 3.0f;
};
