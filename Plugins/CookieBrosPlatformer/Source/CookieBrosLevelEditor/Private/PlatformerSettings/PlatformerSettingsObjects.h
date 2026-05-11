#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyCombatProfile.h"
#include "Platformer/Environment/PlatformerCameraVolume.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "UObject/Object.h"
#include "PlatformerSettingsObjects.generated.h"

class AActor;
class APlatformerEnemyBase;
class APlatformerEnemyElite;
class APlatformerEnemyRanged;
class APlatformerCameraVolume;
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
class UPlatformerCameraVolumeSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm", DisplayName="Volume Size New"))
	FVector VolumeSizeNew = FVector(400.0f, 1000.0f, 400.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Location New", MakeEditWidget=true))
	FVector LocationNew = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm", DisplayName="Volume Size Default"))
	FVector VolumeSizeDefault = FVector(400.0f, 1000.0f, 400.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(DisplayName="Location Default", MakeEditWidget=true))
	FVector LocationDefault = FVector(500.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Blend Time"))
	float BlendTime = 0.5f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.01, DisplayName="Ease Exponent"))
	float EaseExponent = 2.0f;

	UPROPERTY(EditAnywhere, Category="Camera Settings")
	FPlatformerCameraVolumeSettings TargetCameraSettings;

private:
	APlatformerCameraVolume* GetEditedCameraVolume() const;
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
class UPlatformerEnemySettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=1.0, DisplayName="Health"))
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=1.0, Units="cm/s", DisplayName="Movement Speed"))
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=0.0, DisplayName="Damage"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=0.0, Units="s", DisplayName="Hit Delay"))
	float HitDelay = 1.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=0.0, Units="s", DisplayName="Movement Delay On Hit"))
	float MovementDelayOnHit = 0.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="On Hit Taken Impulse"))
	float OnHitTakenImpulse = 0.0f;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(DisplayName="Enable Player Chase"))
	bool bEnablePlayerChase = false;

	UPROPERTY(EditAnywhere, Category="CommonEnemySettings", meta=(ClampMin=0.0, Units="cm", DisplayName="Chase Agro Radius"))
	float ChaseAgroRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Repeat Path"))
	bool bRepeatPath = true;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Path Points"))
	TArray<FPlatformerPathPoint> PathPoints;

protected:
	APlatformerEnemyBase* GetEditedEnemy() const;
};

UCLASS(Transient)
class UPlatformerRangedEnemySettingsObject : public UPlatformerEnemySettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Projectile Speed"))
	float ProjectileSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm", DisplayName="Projectile Distance"))
	float ProjectileDistance = 600.0f;

private:
	APlatformerEnemyRanged* GetEditedRangedEnemy() const;
};

UCLASS(Transient)
class UPlatformerEliteEnemySettingsObject : public UPlatformerEnemySettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="EliteEnemySettings", meta=(DisplayName="Combat Profile"))
	EPlatformerEnemyCombatProfile CombatProfile = EPlatformerEnemyCombatProfile::Hybrid;

	UPROPERTY(EditAnywhere, Category="EliteEnemySettings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Projectile Speed"))
	float ProjectileSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category="EliteEnemySettings", meta=(ClampMin=0.0, Units="cm", DisplayName="Projectile Distance"))
	float ProjectileDistance = 600.0f;

private:
	APlatformerEnemyElite* GetEditedEliteEnemy() const;
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

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm/s", DisplayName="Move Speed"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Repeat Path"))
	bool bRepeatPath = false;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Path Points"))
	TArray<FPlatformerPathPoint> PathPoints;
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

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=1.0, Units="cm/s", DisplayName="Move Speed"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Repeat Path"))
	bool bRepeatPath = false;

	UPROPERTY(EditAnywhere, Category="Path Component", meta=(DisplayName="Path Points"))
	TArray<FPlatformerPathPoint> PathPoints;
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

UCLASS(Transient)
class UPlatformerYokuBlocksSettingsObject : public UPlatformerActorSettingsObject
{
	GENERATED_BODY()

public:
	virtual void PullFromActor(AActor* Actor) override;
	virtual void PushToActor() override;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Initial Delay"))
	float InitialDelay = 0.0f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Show Duration"))
	float ShowDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="s", DisplayName="Hiden Duration"))
	float HidenDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category="Quick Settings", meta=(ClampMin=0.0, Units="cm/s", DisplayName="Hide Speed"))
	float HideSpeed = 300.0f;
};
