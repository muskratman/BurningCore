#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyFlying.h"
#include "EnemyFlying.generated.h"

/**
 * AEnemyFlying
 * Project-local flying enemy shell that binds DragonSlayer enemy attributes to the reusable platformer base.
 */
UCLASS()
class DRAGONSLAYER_API AEnemyFlying : public APlatformerEnemyFlying
{
	GENERATED_BODY()

public:
	AEnemyFlying(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
