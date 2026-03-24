#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyFlying.generated.h"

/**
 * AEnemyFlying
 * Airborne patrol enemy that swoops or shoots from above.
 */
UCLASS()
class BURNINGCORE_API AEnemyFlying : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemyFlying();
};
