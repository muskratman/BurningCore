#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyBase.h"
#include "PlatformerEnemyFlying.generated.h"

/**
 * APlatformerEnemyFlying
 * Generic airborne platformer enemy shell.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerEnemyFlying : public APlatformerEnemyBase
{
	GENERATED_BODY()

public:
	APlatformerEnemyFlying(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
