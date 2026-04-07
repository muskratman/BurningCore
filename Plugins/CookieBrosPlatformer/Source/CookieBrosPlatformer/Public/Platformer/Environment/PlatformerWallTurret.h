#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerWallTurret.generated.h"

class APlatformerHazardProjectile;
class ACharacter;
class UArrowComponent;
class UBoxComponent;
class UGameplayEffect;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

/**
 * Simple wall turret that periodically spawns configurable hazard projectiles.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerWallTurret : public AActor
{
	GENERATED_BODY()

public:
	APlatformerWallTurret();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TurretMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> MuzzlePointLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UArrowComponent> MuzzlePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> ActivationVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> ActivationVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile")
	TSubclassOf<APlatformerHazardProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile", meta=(ClampMin=0.1, Units="s"))
	float FireInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile", meta=(ClampMin=0.0, Units="cm/s"))
	float ProjectileSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile", meta=(ClampMin=0.0, Units="s"))
	float ProjectileLifetime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile", meta=(ClampMin=0.0))
	float ProjectileDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Projectile")
	TSubclassOf<UGameplayEffect> ProjectileDamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Behaviour")
	bool bAutoActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Behaviour")
	bool bRequireTargetInRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Behaviour")
	bool bAimAtTrackedTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Behaviour")
	FVector ActivationExtent = FVector(800.0f, 200.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Behaviour", meta=(Units="cm"))
	float MuzzleOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Components")
	FPlatformerComponentTransformOffset TurretMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Components")
	FPlatformerComponentTransformOffset MuzzlePointTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret|Components")
	FPlatformerComponentTransformOffset ActivationVolumeTransformOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	FTimerHandle FireTimerHandle;
	TWeakObjectPtr<ACharacter> TrackedTarget;

	UFUNCTION()
	void OnActivationVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnActivationVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void FireProjectile();

	bool IsAllowedToFire() const;
	ACharacter* ResolveTrackedTarget() const;
};
