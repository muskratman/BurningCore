#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerBossBase.h"
#include "BossBase.generated.h"

/**
 * ABossBase
 * Project-local boss shell that binds DragonSlayer enemy attributes to the reusable platformer boss base.
 */
UCLASS()
class DRAGONSLAYER_API ABossBase : public APlatformerBossBase
{
	GENERATED_BODY()

public:
	ABossBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
