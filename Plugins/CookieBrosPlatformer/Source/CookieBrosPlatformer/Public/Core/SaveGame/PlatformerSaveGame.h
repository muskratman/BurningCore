// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "PlatformerSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FPlatformerCheckpointSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	FGameplayTag CheckpointTag;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	FName CheckpointActorName = NAME_None;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	FName LevelName = NAME_None;
};

/**
 * Generic platformer progression save shell.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	FPlatformerCheckpointSaveData LastCheckpoint;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	bool bHasStartedGame = false;
};
