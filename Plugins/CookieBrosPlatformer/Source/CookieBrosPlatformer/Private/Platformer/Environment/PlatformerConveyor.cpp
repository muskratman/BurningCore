#include "Platformer/Environment/PlatformerConveyor.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

APlatformerConveyor::APlatformerConveyor()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerConveyor.PlatformerConveyor")));
}

void APlatformerConveyor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClearInvalidOverlappingCharacters();

	if (LocalConveyorVelocity.IsNearlyZero())
	{
		return;
	}

	const FVector WorldDelta = GetActorTransform().TransformVectorNoScale(LocalConveyorVelocity) * DeltaTime;

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : GetOverlappingCharacters())
	{
		ACharacter* Character = CharacterPtr.Get();
		if (Character == nullptr)
		{
			continue;
		}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			const bool bStandingOnThisConveyor = IsCharacterStandingOnSurface(Character);
			if (MovementComponent->IsMovingOnGround() && !bStandingOnThisConveyor)
			{
				continue;
			}

			if (!bAffectCharactersInAir && !MovementComponent->IsMovingOnGround())
			{
				continue;
			}
		}

		Character->AddActorWorldOffset(WorldDelta, true);
	}
}
