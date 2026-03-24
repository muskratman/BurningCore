#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BurningCOREPlayerState.generated.h"

/**
 * ABurningCOREPlayerState
 * Holds session-level stats for the player (e.g. deaths, run time).
 */
UCLASS()
class BURNINGCORE_API ABurningCOREPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABurningCOREPlayerState();

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	int32 SessionDeaths = 0;

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	float SessionTime = 0.0f;
};
