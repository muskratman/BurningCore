// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/PlatformerGameModeBase.h"

#include "AI/PlatformerBossBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Platformer/Environment/PlatformerCheckpoint.h"
#include "Platformer/Systems/PlatformerCheckpointSubsystem.h"

APlatformerGameModeBase::APlatformerGameModeBase()
{
}

void APlatformerGameModeBase::RespawnPlayerAtCheckpoint()
{
	if (UWorld* World = GetWorld())
	{
		if (UPlatformerCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UPlatformerCheckpointSubsystem>())
		{
			CheckpointSubsystem->RespawnPlayerAtCheckpoint(UGameplayStatics::GetPlayerController(this, 0));
		}
	}
}

void APlatformerGameModeBase::ActivateBossEncounter(APlatformerBossBase* Boss)
{
	const FString BossName = IsValid(Boss) ? Boss->GetName() : TEXT("None");
	UE_LOG(LogTemp, Log, TEXT("Boss encounter activated: %s"), *BossName);
}

void APlatformerGameModeBase::OnLevelCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("Level completed."));
}

void APlatformerGameModeBase::RegisterCheckpoint_Implementation(APlatformerCheckpoint* Checkpoint)
{
	if (!IsValid(Checkpoint))
	{
		return;
	}

	LastCheckpoint = Checkpoint;
	if (UWorld* World = GetWorld())
	{
		if (UPlatformerCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UPlatformerCheckpointSubsystem>())
		{
			CheckpointSubsystem->ActivateCheckpoint(Checkpoint);
		}
	}
}

void APlatformerGameModeBase::RegisterBossEncounterActor_Implementation(APlatformerBossBase* Boss)
{
	ActivateBossEncounter(Boss);
}

void APlatformerGameModeBase::ProcessPlatformerPickup_Implementation(APlatformerPickup* Pickup, ACharacter* Collector)
{
	++PickupsCollected;
	HandlePickupCollected(Pickup, Collector);
}

void APlatformerGameModeBase::HandlePickupCollected(APlatformerPickup* Pickup, ACharacter* Collector)
{
}
