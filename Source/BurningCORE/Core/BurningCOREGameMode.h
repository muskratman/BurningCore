#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BurningCOREGameMode.generated.h"

class ABossBase;
class ACheckpointActor;

/**
 * ABurningCOREGameMode
 * Orchestrates level flow, respawns, and boss encounters.
 */
UCLASS(minimalapi)
class ABurningCOREGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABurningCOREGameMode();

	virtual void BeginPlay() override;

	/** Respawns the player at the last activated checkpoint */
	UFUNCTION(BlueprintCallable, Category="Flow")
	virtual void RespawnPlayerAtCheckpoint();

	/** Notifies mode a boss was activated */
	UFUNCTION(BlueprintCallable, Category="Flow")
	virtual void ActivateBossEncounter(ABossBase* Boss);

	/** Triggers level completion logic */
	UFUNCTION(BlueprintCallable, Category="Flow")
	virtual void OnLevelCompleted();

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void RegisterCheckpoint(ACheckpointActor* Checkpoint);

protected:
	UPROPERTY()
	TObjectPtr<ACheckpointActor> LastCheckpoint;
};
