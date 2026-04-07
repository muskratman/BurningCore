#include "Platformer/Environment/PlatformerDangerBlock.h"

#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"

APlatformerDangerBlock::APlatformerDangerBlock()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerDangerBlock.PlatformerDangerBlock")));
}

void APlatformerDangerBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClearInvalidOverlappingCharacters();

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : GetOverlappingCharacters())
	{
		if (CharacterPtr.IsValid())
		{
			TryDamageCharacter(CharacterPtr.Get());
		}
	}
}

void APlatformerDangerBlock::TryDamageCharacter(ACharacter* Character)
{
	if (!CanDamageActor(Character))
	{
		return;
	}

	FHitResult HitResult;
	HitResult.Location = Character->GetActorLocation();
	HitResult.ImpactPoint = Character->GetActorLocation();
	HitResult.ImpactNormal = FVector::UpVector;

	if (PlatformerEnvironment::ApplyConfiguredDamage(this, Character, DamageEffectClass, DamageAmount, HitResult))
	{
		LastDamageTimeByActor.Add(Character, GetWorld()->GetTimeSeconds());

		if (bLaunchCharacters)
		{
			const FVector AwayDirection = FVector(
				Character->GetActorLocation().X - GetActorLocation().X,
				Character->GetActorLocation().Y - GetActorLocation().Y,
				0.0f).GetSafeNormal();
			Character->LaunchCharacter((AwayDirection * HorizontalKnockback) + (FVector::UpVector * VerticalKnockback), true, true);
		}
	}
}

bool APlatformerDangerBlock::CanDamageActor(AActor* Actor) const
{
	if ((Actor == nullptr) || (Actor == this))
	{
		return false;
	}

	if (const float* LastDamageTime = LastDamageTimeByActor.Find(Actor))
	{
		return (GetWorld()->GetTimeSeconds() - *LastDamageTime) >= DamageCooldown;
	}

	return true;
}
