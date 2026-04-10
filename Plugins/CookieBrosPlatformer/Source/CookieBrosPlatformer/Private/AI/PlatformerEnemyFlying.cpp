#include "AI/PlatformerEnemyFlying.h"

#include "Character/SideViewMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlatformerEnemyFlying::APlatformerEnemyFlying(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Flying enemies ignore gravity and stay in MOVE_Flying by default.
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			SideViewMovementComponent->SetBaseGravityScale(0.0f);
		}
		else
		{
			MovementComponent->GravityScale = 0.0f;
		}

		MovementComponent->DefaultLandMovementMode = MOVE_Flying;
	}
}
