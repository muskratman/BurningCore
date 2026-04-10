#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyMelee.h"
#include "EnemyMelee.generated.h"

/**
 * AEnemyMelee
 * Project-local melee enemy shell that binds DragonSlayer enemy attributes to the reusable platformer base.
 */
UCLASS()
class DRAGONSLAYER_API AEnemyMelee : public APlatformerEnemyMelee
{
	GENERATED_BODY()
	
public:
	AEnemyMelee(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
