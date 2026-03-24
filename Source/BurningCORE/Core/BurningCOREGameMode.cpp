#include "Core/BurningCOREGameMode.h"
#include "Core/BurningCOREPlayerController.h"
#include "Core/BurningCOREGameState.h"
#include "Systems/CheckpointActor.h"
#include "BurningCORE.h"

ABurningCOREGameMode::ABurningCOREGameMode()
{
	GameStateClass = ABurningCOREGameState::StaticClass();
	PlayerControllerClass = ABurningCOREPlayerController::StaticClass();
}

void ABurningCOREGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ABurningCOREGameMode::RespawnPlayerAtCheckpoint()
{
	// Checkpoint respawn logic
	UE_LOG(LogBurningCORE, Log, TEXT("Respawing player at checkpoint..."));
}

void ABurningCOREGameMode::ActivateBossEncounter(ABossBase* Boss)
{
	UE_LOG(LogBurningCORE, Log, TEXT("Boss encounter activated!"));
}

void ABurningCOREGameMode::OnLevelCompleted()
{
	UE_LOG(LogBurningCORE, Log, TEXT("Level completed!"));
}

void ABurningCOREGameMode::RegisterCheckpoint(ACheckpointActor* Checkpoint)
{
	LastCheckpoint = Checkpoint;
}
