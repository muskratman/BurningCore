#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "BurningCORESaveGame.generated.h"

UENUM(BlueprintType)
enum class ELevelCompletionStatus : uint8
{
	Locked,
	Unlocked,
	Completed,
	Perfected
};

USTRUCT(BlueprintType)
struct FCheckpointSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGameplayTag CheckpointTag;

	UPROPERTY(SaveGame)
	FName LevelName;
};

USTRUCT(BlueprintType)
struct FPlayerProgressionData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<FGameplayTag, bool> UnlockedRegions;

	UPROPERTY(SaveGame)
	TMap<FGameplayTag, ELevelCompletionStatus> LevelStates;

	UPROPERTY(SaveGame)
	TSet<FGameplayTag> UnlockedForms;

	UPROPERTY(SaveGame)
	TSet<FGameplayTag> PurchasedUpgrades;

	UPROPERTY(SaveGame)
	TSet<FName> CollectedSecrets;

	UPROPERTY(SaveGame)
	int32 UpgradeCurrency = 0;

	UPROPERTY(SaveGame)
	int32 TotalDeaths = 0;

	UPROPERTY(SaveGame)
	FCheckpointSaveData LastCheckpoint;
};

/**
 * UBurningCORESaveGame
 * Save game object containing all progression data.
 */
UCLASS()
class BURNINGCORE_API UBurningCORESaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UBurningCORESaveGame();

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Progression")
	FPlayerProgressionData ProgressionData;
};
