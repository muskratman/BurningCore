#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyRanged.generated.h"

/**
 * AEnemyRanged
 * Prefers keeping distance from player and shooting projectiles.
 */
UCLASS()
class BURNINGCORE_API AEnemyRanged : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemyRanged();
};
