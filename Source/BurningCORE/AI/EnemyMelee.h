#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyMelee.generated.h"

/**
 * AEnemyMelee
 * Charges straight at the player to deal close combat damage.
 */
UCLASS()
class BURNINGCORE_API AEnemyMelee : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemyMelee();
};
