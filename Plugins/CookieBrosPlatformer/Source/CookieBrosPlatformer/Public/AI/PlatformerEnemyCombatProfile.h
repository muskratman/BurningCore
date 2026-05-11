#pragma once

#include "CoreMinimal.h"
#include "PlatformerEnemyCombatProfile.generated.h"

/**
 * Abstract reusable combat capability for platformer enemies.
 */
UENUM(BlueprintType)
enum class EPlatformerEnemyCombatProfile : uint8
{
	Melee UMETA(DisplayName="Melee"),
	Ranged UMETA(DisplayName="Ranged"),
	Hybrid UMETA(DisplayName="Melee + Ranged")
};
