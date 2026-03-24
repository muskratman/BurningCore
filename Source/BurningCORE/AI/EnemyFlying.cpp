#include "AI/EnemyFlying.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyFlying::AEnemyFlying()
{
	// Disable gravity for flying
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	}
}
