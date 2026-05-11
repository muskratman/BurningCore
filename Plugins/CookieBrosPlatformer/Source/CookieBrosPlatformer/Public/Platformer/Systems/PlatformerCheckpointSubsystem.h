#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlatformerCheckpointSubsystem.generated.h"

class AController;
class APlatformerCheckpoint;

/**
 * World-local checkpoint service.
 * Keeps checkpoint actor logic thin and owns respawn/save coordination for generic platformer projects.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerCheckpointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void RegisterCheckpoint(APlatformerCheckpoint* Checkpoint);

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void UnregisterCheckpoint(APlatformerCheckpoint* Checkpoint);

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	bool ActivateCheckpoint(APlatformerCheckpoint* Checkpoint, AActor* Activator = nullptr);

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	bool RestoreActiveCheckpointFromSave();

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	bool RespawnPlayerAtCheckpoint(AController* Controller);

	UFUNCTION(BlueprintPure, Category="Checkpoint")
	APlatformerCheckpoint* GetActiveCheckpoint() const { return ActiveCheckpoint.Get(); }

	UFUNCTION(BlueprintPure, Category="Checkpoint")
	APlatformerCheckpoint* FindCheckpointByTag(FGameplayTag CheckpointTag) const;

	UFUNCTION(BlueprintPure, Category="Checkpoint")
	APlatformerCheckpoint* FindCheckpointByActorName(FName CheckpointActorName) const;

private:
	void SaveActiveCheckpoint() const;
	bool MoveExistingPlayerToActiveCheckpoint() const;
	FName GetCurrentLevelName() const;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APlatformerCheckpoint>> RegisteredCheckpoints;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlatformerCheckpoint> ActiveCheckpoint;
};
