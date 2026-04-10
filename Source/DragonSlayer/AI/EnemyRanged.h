#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyRanged.h"
#include "EnemyRanged.generated.h"

/**
 * AEnemyRanged
 * Project-local ranged enemy shell that binds DragonSlayer enemy attributes to the reusable platformer base.
 */
UCLASS()
class DRAGONSLAYER_API AEnemyRanged : public APlatformerEnemyRanged
{
	GENERATED_BODY()
	
public:
	AEnemyRanged(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
