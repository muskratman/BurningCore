#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BurningCOREGameState.generated.h"

/**
 * ABurningCOREGameState
 * Holds state for the current level (score, live enemies, phase).
 */
UCLASS()
class BURNINGCORE_API ABurningCOREGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABurningCOREGameState();

	UPROPERTY(BlueprintReadOnly, Category="Score")
	int32 CurrentLevelScore = 0;

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	int32 ActiveEnemyCount = 0;
};
