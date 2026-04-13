#include "PlatformerSettings/PlatformerSettingsObjects.h"

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

void UPlatformerConveyorSettingsObject::PullFromActor(AActor* Actor)
{
	SetEditedActor(Actor);

	FVector LocalVelocity = FVector::ZeroVector;
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("LocalConveyorVelocity"), LocalVelocity);
	Direction = PlatformerSettingsPrivate::ResolveDirectionOrDefault(LocalVelocity);
	Speed = LocalVelocity.Size();
}

void UPlatformerConveyorSettingsObject::PushToActor()
{
	PlatformerSettingsPrivate::SetVectorPropertyValue(GetEditedActor(), TEXT("LocalConveyorVelocity"), PlatformerSettingsPrivate::ResolveDirectionOrDefault(Direction) * FMath::Max(0.0f, Speed));
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

	FVector PointARelative = FVector::ZeroVector;
	FVector PointBRelative = FVector(500.0f, 0.0f, 0.0f);
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("PointABaseRelativeLocation"), PointARelative);
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("PointBBaseRelativeLocation"), PointBRelative);

	PointA = PlatformerSettingsPrivate::ResolveWorldPoint(Actor, PointARelative);
	PointB = PlatformerSettingsPrivate::ResolveWorldPoint(Actor, PointBRelative);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), MoveSpeed);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("PointADelay"), PointADelay);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("PointBDelay"), PointBDelay);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FBoolProperty>(Actor, TEXT("bIsRepeatable"), bRepeatMovement);
}

void UPlatformerMovingPlatformSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	if (Actor == nullptr)
	{
		return;
	}

	PlatformerSettingsPrivate::MoveActorInEditor(Actor, PointA);
	PlatformerSettingsPrivate::SetVectorPropertyValue(Actor, TEXT("PointABaseRelativeLocation"), FVector::ZeroVector);
	PlatformerSettingsPrivate::SetVectorPropertyValue(Actor, TEXT("PointBBaseRelativeLocation"), PlatformerSettingsPrivate::ResolveRelativePoint(Actor, PointB));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), FMath::Max(1.0f, MoveSpeed));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("PointADelay"), FMath::Max(0.0f, PointADelay));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FFloatProperty>(Actor, TEXT("PointBDelay"), FMath::Max(0.0f, PointBDelay));
	PlatformerSettingsPrivate::SetTypedPropertyValue<FBoolProperty>(Actor, TEXT("bIsRepeatable"), bRepeatMovement);
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

	FVector PointARelative = FVector::ZeroVector;
	FVector PointBRelative = FVector(0.0f, 0.0f, 500.0f);
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("PointABaseRelativeLocation"), PointARelative);
	PlatformerSettingsPrivate::GetVectorPropertyValue(Actor, TEXT("PointBBaseRelativeLocation"), PointBRelative);

	PointA = PlatformerSettingsPrivate::ResolveWorldPoint(Actor, PointARelative);
	PointB = PlatformerSettingsPrivate::ResolveWorldPoint(Actor, PointBRelative);
	PlatformerSettingsPrivate::GetTypedPropertyValue<FFloatProperty>(Actor, TEXT("MoveSpeed"), MoveSpeed);
}

void UPlatformerTriggeredLiftSettingsObject::PushToActor()
{
	AActor* Actor = GetEditedActor();
	if (Actor == nullptr)
	{
		return;
	}

	PlatformerSettingsPrivate::MoveActorInEditor(Actor, PointA);
	PlatformerSettingsPrivate::SetVectorPropertyValue(Actor, TEXT("PointABaseRelativeLocation"), FVector::ZeroVector);
	PlatformerSettingsPrivate::SetVectorPropertyValue(Actor, TEXT("PointBBaseRelativeLocation"), PlatformerSettingsPrivate::ResolveRelativePoint(Actor, PointB));
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
