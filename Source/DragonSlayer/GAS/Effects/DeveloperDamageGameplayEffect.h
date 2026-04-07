#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DeveloperDamageGameplayEffect.generated.h"

UCLASS()
class DRAGONSLAYER_API UDeveloperDamageGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDeveloperDamageGameplayEffect();

	static FGameplayTag GetDeveloperDamageSetByCallerTag();
};
