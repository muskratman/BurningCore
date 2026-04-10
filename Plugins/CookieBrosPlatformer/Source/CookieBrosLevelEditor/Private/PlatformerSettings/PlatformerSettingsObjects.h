#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlatformerSettingsObjects.generated.h"

class AActor;
class APlatformerTeleporter;

UCLASS(Abstract, Transient)
class UPlatformerActorSettingsObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) PURE_VIRTUAL(UPlatformerActorSettingsObject::PullFromActor, );
	virtual void PushToActor() PURE_VIRTUAL(UPlatformerActorSettingsObject::PushToActor, );

	AActor* GetEditedActor() const { return EditedActor.Get(); }

protected:
	void SetEditedActor(AActor* Actor) { EditedActor = Actor; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> EditedActor;
};

UCLASS(Transient)
class UPlatformerConveyorSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings")
	FVector Direction = FVector(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Speed"))
	float Speed = 250.0f;
};

UCLASS(Transient)
class UPlatformerDangerBlockSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Damage"))
	float Damage = 1.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Damage Cooldown"))
	float DamageCooldown = 0.35f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Upward Impulse"))
	float UpwardImpulse = 250.0f;
};

UCLASS(Transient)
class UPlatformerDestructibleBlockSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1, DisplayName="Hits To Destroy"))
	int32 HitsToDestroy = 3;

private:
	float CachedDamagePerHit = 1.0f;
};

UCLASS(Transient)
class UPlatformerFallingPlatformSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Fall Delay"))
	float FallDelay = 0.3f;
};

UCLASS(Transient)
class UPlatformerGravityVolumeSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Gravity Multiplier"))
	float GravityMultiplier = 0.3f;
};

UCLASS(Transient)
class UPlatformerHazardProjectileSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Projectile Speed"))
	float ProjectileSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Damage"))
	float ProjectileDamage = 1.0f;
};

UCLASS(Transient)
class UPlatformerJumpPadSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Impulse"))
	float Impulse = 1000.0f;
};

UCLASS(Transient)
class UPlatformerMovingPlatformSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Point A"))
	FVector PointA = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Point B"))
	FVector PointB = FVector(500.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm/s", DisplayName="Move Speed"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Delay At Point A"))
	float PointADelay = 0.5f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Delay At Point B"))
	float PointBDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Repeat A <-> B"))
	bool bRepeatMovement = false;
};

UCLASS(Transient)
class UPlatformerSlipperyBlockSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Friction"))
	float Friction = 0.35f;
};

UCLASS(Transient)
class UPlatformerSpikesSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Damage"))
	float Damage = 1.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Damage Cooldown"))
	float DamageCooldown = 0.4f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Upward Impulse"))
	float UpwardImpulse = 275.0f;
};

UCLASS(Transient)
class UPlatformerStreamSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Stream Strength"))
	float StreamStrength = 250.0f;

private:
	FVector CachedDirection = FVector(1.0f, 0.0f, 0.0f);
};

UCLASS(Transient)
class UPlatformerSwitchSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Switch ID"))
	FString SwitchId;
};

UCLASS(Transient)
class UPlatformerTeleporterSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Teleporter ID"))
	FString TeleporterId;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Exit Teleporter"))
	TObjectPtr<APlatformerTeleporter> ExitTeleporter;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Point A"))
	FVector PointA = FVector::ZeroVector;
};

UCLASS(Transient)
class UPlatformerTriggeredLiftSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Point A"))
	FVector PointA = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Point B"))
	FVector PointB = FVector(0.0f, 0.0f, 500.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm/s", DisplayName="Move Speed"))
	float MoveSpeed = 250.0f;
};

UCLASS(Transient)
class UPlatformerVanishingBlockSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Disappear Delay"))
	float DisappearDelay = 0.5f;
};

UCLASS(Transient)
class UPlatformerWallTurretSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Projectile Speed"))
	float ProjectileSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Fire Rate"))
	float FireRate = 0.666667f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, DisplayName="Projectile Damage"))
	float ProjectileDamage = 1.0f;
};
