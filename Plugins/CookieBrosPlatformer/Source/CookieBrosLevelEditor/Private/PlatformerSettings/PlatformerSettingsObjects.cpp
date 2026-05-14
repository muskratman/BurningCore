#include "PlatformerSettings/PlatformerSettingsObjects.h"

#include "AI/PlatformerEnemyBase.h"
#include "AI/PlatformerEnemyElite.h"
#include "AI/PlatformerEnemyRanged.h"
#include "Platformer/Environment/PlatformerCameraVolume.h"
#include "Platformer/Environment/PlatformerConveyor.h"
#include "Platformer/Environment/PlatformerDangerBlock.h"
#include "Platformer/Environment/PlatformerDestructibleBlock.h"
#include "Platformer/Environment/PlatformerFallingPlatform.h"
#include "Platformer/Environment/PlatformerGravityVolume.h"
#include "Platformer/Environment/PlatformerHazardProjectile.h"
#include "Platformer/Environment/PlatformerJumpPad.h"
#include "Platformer/Environment/PlatformerMovingPlatform.h"
#include "Platformer/Environment/PlatformerSlipperyBlock.h"
#include "Platformer/Environment/PlatformerSpikes.h"
#include "Platformer/Environment/PlatformerStream.h"
#include "Platformer/Environment/PlatformerSwitch.h"
#include "Platformer/Environment/PlatformerTeleporter.h"
#include "Platformer/Environment/PlatformerTriggeredLift.h"
#include "Components/SceneComponent.h"
#include "Platformer/Environment/PlatformerVanishingBlock.h"
#include "Platformer/Environment/PlatformerWallTurret.h"
#include "Platformer/Environment/PlatformerYokuBlocks.h"
#include "UObject/UnrealType.h"

namespace PlatformerSettingsPrivate
{
	template<typename PropertyType, typename ValueType>
	bool GetTypedPropertyValue(const UObject* Object, const FName PropertyName, ValueType& OutValue)
	{
		if (Object == nullptr)
		{
			return false;
		}

		const PropertyType* Property = FindFProperty<PropertyType>(Object->GetClass(), PropertyName);
		if (Property == nullptr)
		{
			return false;
		}

		OutValue = Property->GetPropertyValue_InContainer(Object);
		return true;
	}

	template<typename PropertyType, typename ValueType>
	bool SetTypedPropertyValue(UObject* Object, const FName PropertyName, const ValueType& InValue)
	{
		if (Object == nullptr)
		{
			return false;
		}

		PropertyType* Property = FindFProperty<PropertyType>(Object->GetClass(), PropertyName);
		if (Property == nullptr)
		{
			return false;
		}

		Object->Modify();
		Object->PreEditChange(Property);
		Property->SetPropertyValue_InContainer(Object, InValue);
		FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet);
		Object->PostEditChangeProperty(PropertyChangedEvent);
		return true;
	}

	bool GetVectorPropertyValue(const UObject* Object, const FName PropertyName, FVector& OutValue)
	{
		if (Object == nullptr)
		{
			return false;
		}

		const FStructProperty* Property = FindFProperty<FStructProperty>(Object->GetClass(), PropertyName);
		if ((Property == nullptr) || (Property->Struct != TBaseStructure<FVector>::Get()))
		{
			return false;
		}

		OutValue = *Property->ContainerPtrToValuePtr<FVector>(Object);
		return true;
	}

	bool SetVectorPropertyValue(UObject* Object, const FName PropertyName, const FVector& InValue)
	{
		if (Object == nullptr)
		{
			return false;
		}

		FStructProperty* Property = FindFProperty<FStructProperty>(Object->GetClass(), PropertyName);
		if ((Property == nullptr) || (Property->Struct != TBaseStructure<FVector>::Get()))
		{
			return false;
		}

		Object->Modify();
		Object->PreEditChange(Property);
		*Property->ContainerPtrToValuePtr<FVector>(Object) = InValue;
		FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet);
		Object->PostEditChangeProperty(PropertyChangedEvent);
		return true;
	}

	FVector ResolveDirectionOrDefault(const FVector& Vector)
	{
		return Vector.IsNearlyZero() ? FVector(1.0f, 0.0f, 0.0f) : Vector.GetSafeNormal();
	}

	FVector ResolveWorldPoint(const AActor* Actor, const FVector& RelativePoint)
	{
		return Actor != nullptr ? Actor->GetActorTransform().TransformPosition(RelativePoint) : RelativePoint;
	}

	FVector ResolveRelativePoint(const AActor* Actor, const FVector& WorldPoint)
	{
		return Actor != nullptr ? Actor->GetActorTransform().InverseTransformPosition(WorldPoint) : WorldPoint;
	}

	void MoveActorInEditor(AActor* Actor, const FVector& NewWorldLocation)
	{
		if (Actor == nullptr)
		{
			return;
		}

		Actor->Modify();
		Actor->SetActorLocation(NewWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Actor->PostEditMove(true);
	}

	void MoveSceneComponentInEditor(USceneComponent* SceneComponent, const FVector& NewWorldLocation)
	{
		if (SceneComponent == nullptr)
		{
			return;
		}

		SceneComponent->Modify();
		SceneComponent->SetWorldLocation(NewWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);

		if (AActor* Owner = SceneComponent->GetOwner())
		{
			Owner->Modify();
			Owner->PostEditMove(true);
		}
	}

	USceneComponent* ResolveSceneComponentProperty(const AActor* Actor, const FName PropertyName)
	{
		if (Actor == nullptr)
		{
			return nullptr;
		}

		const FObjectProperty* Property = FindFProperty<FObjectProperty>(Actor->GetClass(), PropertyName);
		if (Property == nullptr)
		{
			return nullptr;
		}

		return Cast<USceneComponent>(Property->GetObjectPropertyValue_InContainer(Actor));
	}

}

APlatformerCameraVolume* UPlatformerCameraVolumeSettingsObject::GetEditedCameraVolume() const
{
	return Cast<APlatformerCameraVolume>(GetEditedActor());
}

void UPlatformerCameraVolumeSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	const APlatformerCameraVolume* CameraVolume = Cast<APlatformerCameraVolume>(Actor);
	if (!CameraVolume)
	{
		return;
	}

	VolumeSizeNew = CameraVolume->GetVolumeSizeNew();
	LocationNew = CameraVolume->GetLocationNew();
	VolumeSizeDefault = CameraVolume->GetVolumeSizeDefault();
	LocationDefault = CameraVolume->GetLocationDefault();
	BlendTime = CameraVolume->GetBlendTime();
	EaseExponent = CameraVolume->GetEaseExponent();
	TargetCameraSettings = CameraVolume->GetTargetCameraSettings();
}

void UPlatformerCameraVolumeSettingsObject::PushToActor()
{
	APlatformerCameraVolume* CameraVolume = GetEditedCameraVolume();
	if (!CameraVolume)
	{
		return;
	}

	CameraVolume->Modify();
	CameraVolume->SetVolumeSizeNew(VolumeSizeNew);
	CameraVolume->SetLocationNew(LocationNew);
	CameraVolume->SetVolumeSizeDefault(VolumeSizeDefault);
	CameraVolume->SetLocationDefault(LocationDefault);
	CameraVolume->SetCameraBlendSettings(BlendTime, EaseExponent);
	CameraVolume->SetTargetCameraSettings(TargetCameraSettings);
	CameraVolume->PostEditChange();
}

void UPlatformerConveyorSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	FVector LocalVelocity = FVector::ZeroVector;
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("LocalConveyorVelocity"), LocalVelocity);
	Direction = LocalVelocity.X < 0.0f ? FVector(-1.0f, 0.0f, 0.0f) : FVector(1.0f, 0.0f, 0.0f);
	Speed = FMath::Abs(LocalVelocity.X);
}

void UPlatformerConveyorSettingsObject::PushToActor()
{
	const float DirectionSign = Direction.X < 0.0f ? -1.0f : 1.0f;
	PlatformerSettingsPrivate::SetVectorPropertyValue(
		GetEditedActor(),
		TEXT("LocalConveyorVelocity"),
		FVector(DirectionSign * FMath::Max(0.0f, Speed), 0.0f, 0.0f));
}

void UPlatformerDangerBlockSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageAmount"), Damage);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageCooldown"), DamageCooldown);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("VerticalKnockback"), UpwardImpulse);
}

void UPlatformerDangerBlockSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("DamageAmount"), FMath::Max(0.0f, Damage));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("DamageCooldown"), FMath::Max(0.0f, DamageCooldown));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("VerticalKnockback"), FMath::Max(0.0f, UpwardImpulse));
}

void UPlatformerDestructibleBlockSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	float MaxHitPoints = 3.0f;
	CachedDamagePerHit = 1.0f;
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MaxHitPoints"), MaxHitPoints);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamagePerHit"), CachedDamagePerHit);
	CachedDamagePerHit = FMath::Max(CachedDamagePerHit, KINDA_SMALL_NUMBER);
	HitsToDestroy = FMath::Max(1, FMath::RoundToInt(MaxHitPoints / CachedDamagePerHit));
}

void UPlatformerDestructibleBlockSettingsObject::PushToActor()
{
	const float ResolvedMaxHitPoints = FMath::Max(1, HitsToDestroy) * FMath::Max(CachedDamagePerHit, KINDA_SMALL_NUMBER);
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("MaxHitPoints"), ResolvedMaxHitPoints);
}

void UPlatformerFallingPlatformSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("FallDelay"), FallDelay);
}

void UPlatformerFallingPlatformSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("FallDelay"), FMath::Max(0.0f, FallDelay));
}

void UPlatformerGravityVolumeSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("GravityScaleOverride"), GravityMultiplier);
}

void UPlatformerGravityVolumeSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("GravityScaleOverride"), GravityMultiplier);
}

APlatformerEnemyBase* UPlatformerEnemySettingsObject::GetEditedEnemy() const
{
	return Cast<APlatformerEnemyBase>(GetEditedActor());
}

void UPlatformerEnemySettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	const APlatformerEnemyBase* Enemy = Cast<APlatformerEnemyBase>(Actor);
	if (!Enemy)
	{
		return;
	}

	Health = Enemy->GetEnemyHealth();
	MovementSpeed = Enemy->GetEnemyMovementSpeed();
	Damage = Enemy->GetEnemyDamage();
	HitDelay = Enemy->GetEnemyHitDelay();
	MovementDelayOnHit = Enemy->GetMovementDelayOnHit();
	OnHitTakenImpulse = Enemy->GetOnHitTakenImpulse();
	bEnablePlayerChase = Enemy->GetEnablePlayerChase();
	ChaseAgroRadius = Enemy->GetChaseAgroRadius();
	if (const UPlatformerPathComponent* PathComponent = Enemy->GetPatrolPathComponent())
	{
		bRepeatPath = PathComponent->ShouldRepeatPath();
		PathPoints = PathComponent->GetPathPoints();
	}
}

void UPlatformerEnemySettingsObject::PushToActor()
{
	APlatformerEnemyBase* Enemy = GetEditedEnemy();
	if (!Enemy)
	{
		return;
	}

	Enemy->Modify();
	Enemy->SetEnemyHealth(Health);
	Enemy->SetEnemyMovementSpeed(MovementSpeed);
	Enemy->SetEnemyDamage(Damage);
	Enemy->SetEnemyHitDelay(HitDelay);
	Enemy->SetMovementDelayOnHit(MovementDelayOnHit);
	Enemy->SetOnHitTakenImpulse(OnHitTakenImpulse);
	Enemy->SetEnablePlayerChase(bEnablePlayerChase);
	Enemy->SetChaseAgroRadius(ChaseAgroRadius);
	if (UPlatformerPathComponent* PathComponent = Enemy->GetPatrolPathComponent())
	{
		PathComponent->Modify();
		PathComponent->SetRepeatPath(bRepeatPath);
		PathComponent->SetPathPoints(PathPoints);
	}
	Enemy->PostEditChange();
}

APlatformerEnemyRanged* UPlatformerRangedEnemySettingsObject::GetEditedRangedEnemy() const
{
	return Cast<APlatformerEnemyRanged>(GetEditedActor());
}

void UPlatformerRangedEnemySettingsObject::PullFromActor(AActor* Actor)
{
	Super::PullFromActor(Actor);

	const APlatformerEnemyRanged* Enemy = Cast<APlatformerEnemyRanged>(Actor);
	if (!Enemy)
	{
		return;
	}

	ProjectileSpeed = Enemy->GetEnemyProjectileSpeed();
	ProjectileDistance = Enemy->GetEnemyProjectileDistance();
}

void UPlatformerRangedEnemySettingsObject::PushToActor()
{
	Super::PushToActor();

	APlatformerEnemyRanged* Enemy = GetEditedRangedEnemy();
	if (!Enemy)
	{
		return;
	}

	Enemy->Modify();
	Enemy->SetEnemyProjectileSpeed(ProjectileSpeed);
	Enemy->SetEnemyProjectileDistance(ProjectileDistance);
	Enemy->PostEditChange();
}

APlatformerEnemyElite* UPlatformerEliteEnemySettingsObject::GetEditedEliteEnemy() const
{
	return Cast<APlatformerEnemyElite>(GetEditedActor());
}

void UPlatformerEliteEnemySettingsObject::PullFromActor(AActor* Actor)
{
	Super::PullFromActor(Actor);

	const APlatformerEnemyElite* Enemy = Cast<APlatformerEnemyElite>(Actor);
	if (!Enemy)
	{
		return;
	}

	CombatProfile = Enemy->GetEliteCombatProfile();
	ProjectileSpeed = Enemy->GetEnemyProjectileSpeed();
	ProjectileDistance = Enemy->GetEnemyProjectileDistance();
}

void UPlatformerEliteEnemySettingsObject::PushToActor()
{
	Super::PushToActor();

	APlatformerEnemyElite* Enemy = GetEditedEliteEnemy();
	if (!Enemy)
	{
		return;
	}

	Enemy->Modify();
	Enemy->SetEliteCombatProfile(CombatProfile);
	Enemy->SetEnemyProjectileSpeed(ProjectileSpeed);
	Enemy->SetEnemyProjectileDistance(ProjectileDistance);
	Enemy->PostEditChange();
}

void UPlatformerHazardProjectileSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("Speed"), ProjectileSpeed);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageAmount"), ProjectileDamage);
}

void UPlatformerHazardProjectileSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("Speed"), FMath::Max(0.0f, ProjectileSpeed));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("DamageAmount"), FMath::Max(0.0f, ProjectileDamage));
}

void UPlatformerJumpPadSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ZStrength"), Impulse);
}

void UPlatformerJumpPadSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("ZStrength"), FMath::Max(0.0f, Impulse));
}

void UPlatformerMovingPlatformSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	if (const APlatformerMovingPlatform* MovingPlatform = Cast<APlatformerMovingPlatform>(Actor))
	{
		if (const UPlatformerPathComponent* PathComponent = MovingPlatform->GetMovementPathComponent())
		{
			bRepeatPath = PathComponent->ShouldRepeatPath();
			PathPoints = PathComponent->GetPathPoints();
		}
	}
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), MoveSpeed);
}

void UPlatformerMovingPlatformSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	if (Actor == nullptr)
	{
		return;
	}

	if (APlatformerMovingPlatform* MovingPlatform = Cast<APlatformerMovingPlatform>(Actor))
	{
		MovingPlatform->Modify();
		if (UPlatformerPathComponent* PathComponent = MovingPlatform->GetMovementPathComponent())
		{
			PathComponent->Modify();
			PathComponent->SetRepeatPath(bRepeatPath);
		}
		MovingPlatform->SetMovementPathPoints(PathPoints);
	}
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), FMath::Max(1.0f, MoveSpeed));
}

void UPlatformerSlipperyBlockSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("GroundFrictionOverride"), Friction);
}

void UPlatformerSlipperyBlockSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("GroundFrictionOverride"), FMath::Max(0.0f, Friction));
}

void UPlatformerSpikesSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageAmount"), Damage);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageCooldown"), DamageCooldown);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("VerticalKnockback"), UpwardImpulse);
}

void UPlatformerSpikesSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageAmount"), FMath::Max(0.0f, Damage));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("DamageCooldown"), FMath::Max(0.0f, DamageCooldown));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("VerticalKnockback"), FMath::Max(0.0f, UpwardImpulse));
}

void UPlatformerStreamSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	FVector LocalFlowVelocity = FVector::ZeroVector;
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("LocalFlowVelocity"), LocalFlowVelocity);
	CachedDirection = PlatformerSettingsPrivate::ResolveDirectionOrDefault(LocalFlowVelocity);
	StreamStrength = LocalFlowVelocity.Size();
}

void UPlatformerStreamSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetVectorPropertyValue(GetEditedActor(), TEXT("LocalFlowVelocity"), CachedDirection * FMath::Max(0.0f, StreamStrength));
}

void UPlatformerSwitchSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FStrProperty>(Actor, TEXT("SwitchId"), SwitchId);
}

void UPlatformerSwitchSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FStrProperty>(GetEditedActor(), TEXT("SwitchId"), SwitchId);
}

void UPlatformerTeleporterSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PointA = Actor != nullptr ? Actor->GetActorLocation() : FVector::ZeroVector;
	TeleporterId.Reset();
	ExitTeleporter = nullptr;

	PlatformerSettingsPrivate::GetTypedPropertyValue<FStrProperty>(Actor, TEXT("TeleporterId"), TeleporterId);

	UObject* ExitTeleporterObject = nullptr;
	if (PlatformerSettingsPrivate::GetTypedPropertyValue<FObjectProperty>(Actor, TEXT("ExitTeleporter"), ExitTeleporterObject))
	{
		ExitTeleporter = Cast<APlatformerTeleporter>(ExitTeleporterObject);
	}
}

void UPlatformerTeleporterSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	if (Actor == nullptr)
	{
		return;
	}

	PlatformerSettingsPrivate::MoveActorInEditor(Actor, PointA);

	PlatformerSettingsPrivate::SetTypedPropertyValue<FStrProperty>(Actor, TEXT("TeleporterId"), TeleporterId);
	PlatformerSettingsPrivate::SetTypedPropertyValue<FObjectProperty>(Actor, TEXT("ExitTeleporter"), ExitTeleporter.Get());
}

void UPlatformerTriggeredLiftSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	if (const APlatformerMovingPlatform* MovingPlatform = Cast<APlatformerMovingPlatform>(Actor))
	{
		if (const UPlatformerPathComponent* PathComponent = MovingPlatform->GetMovementPathComponent())
		{
			bRepeatPath = PathComponent->ShouldRepeatPath();
			PathPoints = PathComponent->GetPathPoints();
		}
	}
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), MoveSpeed);
}

void UPlatformerTriggeredLiftSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	if (Actor == nullptr)
	{
		return;
	}

	if (APlatformerMovingPlatform* MovingPlatform = Cast<APlatformerMovingPlatform>(Actor))
	{
		MovingPlatform->Modify();
		if (UPlatformerPathComponent* PathComponent = MovingPlatform->GetMovementPathComponent())
		{
			PathComponent->Modify();
			PathComponent->SetRepeatPath(bRepeatPath);
		}
		MovingPlatform->SetMovementPathPoints(PathPoints);
	}
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), FMath::Max(1.0f, MoveSpeed));
}

void UPlatformerVanishingBlockSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("VisibleDuration"), DisappearDelay);
}

void UPlatformerVanishingBlockSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(GetEditedActor(), TEXT("VisibleDuration"), FMath::Max(0.0f, DisappearDelay));
}

void UPlatformerWallTurretSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ProjectileSpeed"), ProjectileSpeed);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ProjectileDamage"), ProjectileDamage);

	float FireInterval = 1.5f;
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("FireInterval"), FireInterval);
	FireRate = FireInterval > 0.0f ? 1.0f / FireInterval : 0.0f;
}

void UPlatformerWallTurretSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ProjectileSpeed"), FMath::Max(0.0f, ProjectileSpeed));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ProjectileDamage"), FMath::Max(0.0f, ProjectileDamage));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("FireInterval"), FireRate > 0.0f ? 1.0f / FireRate : 0.0f);
}

void UPlatformerYokuBlocksSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("InitialDelay"), InitialDelay);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ShowDuration"), ShowDuration);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("HidenDuration"), HidenDuration);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("HideSpeed"), HideSpeed);
}

void UPlatformerYokuBlocksSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("InitialDelay"), FMath::Max(0.0f, InitialDelay));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("ShowDuration"), FMath::Max(0.0f, ShowDuration));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("HidenDuration"), FMath::Max(0.0f, HidenDuration));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("HideSpeed"), FMath::Max(0.0f, HideSpeed));
}
