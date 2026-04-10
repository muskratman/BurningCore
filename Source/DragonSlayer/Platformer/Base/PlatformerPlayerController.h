// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/PlatformerPlayerControllerBase.h"
#include "PlatformerPlayerController.generated.h"

class APlayableDragonCharacter;
class UInputAction;

/**
 *  A simple platformer player controller
 *  Manages input mappings and the defeat flow for the controlled player character.
 */
UCLASS(Config="Game")
class APlatformerPlayerController : public APlatformerPlayerControllerBase
{
	GENERATED_BODY()

public:
	APlatformerPlayerController();

protected:

	/** Production pawn class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<APlayableDragonCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Input Mappings")
	TObjectPtr<UInputAction> ToggleDeveloperAction;

protected:
	virtual void SetupInputComponent() override;
	virtual void HandleControlledCharacterDeath() override;
	virtual APawn* SpawnRespawnPawn(const FTransform& SpawnTransform) override;
	virtual void HandlePauseRequested() override;

private:
	void HandleDeveloperSettingsToggleRequested();
	bool ShouldAutoRestartLevelAfterDefeat() const;
	void RestartCurrentLevel();
};
