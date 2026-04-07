#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"

class AActor;
class UGameplayEffect;
class USceneComponent;
struct FHitResult;

namespace PlatformerEnvironment
{
	void ApplyRelativeTransform(
		USceneComponent* Component,
		const FVector& BaseLocation,
		const FRotator& BaseRotation,
		const FVector& BaseScale,
		const FPlatformerComponentTransformOffset& TransformOffset);

	bool ApplyConfiguredDamage(AActor* SourceActor, AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DirectDamage, const FHitResult& HitResult);
}
