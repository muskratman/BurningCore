#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PlatformerDamageGameplayEffect.generated.h"

/**
 * Native instant-damage gameplay effect used as the default combat payload.
 * Damage amount is passed through a set-by-caller tag.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerDamageGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPlatformerDamageGameplayEffect();
};
