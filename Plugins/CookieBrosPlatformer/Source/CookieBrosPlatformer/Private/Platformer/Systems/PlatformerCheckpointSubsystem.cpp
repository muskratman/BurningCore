#include "Platformer/Systems/PlatformerCheckpointSubsystem.h"

#include "Core/PlatformerGameInstance.h"
#include "Core/SaveGame/PlatformerSaveGame.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Platformer/Environment/PlatformerCheckpoint.h"

void UPlatformerCheckpointSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	RestoreActiveCheckpointFromSave();
}

void UPlatformerCheckpointSubsystem::RegisterCheckpoint(APlatformerCheckpoint* Checkpoint)
{
	if (!IsValid(Checkpoint))
	{
		return;
	}

	RegisteredCheckpoints.AddUnique(Checkpoint);

	const UPlatformerGameInstance* PlatformerGameInstance = GetWorld() ? Cast<UPlatformerGameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	const UPlatformerSaveGame* SaveGame = PlatformerGameInstance ? PlatformerGameInstance->GetPlatformerSave() : nullptr;
	if (SaveGame
		&& !ActiveCheckpoint.IsValid()
		&& SaveGame->LastCheckpoint.LevelName == GetCurrentLevelName()
		&& ((SaveGame->LastCheckpoint.CheckpointTag.IsValid() && Checkpoint->CheckpointID == SaveGame->LastCheckpoint.CheckpointTag)
			|| (!SaveGame->LastCheckpoint.CheckpointActorName.IsNone() && Checkpoint->GetFName() == SaveGame->LastCheckpoint.CheckpointActorName)))
	{
		if (ActivateCheckpoint(Checkpoint))
		{
			MoveExistingPlayerToActiveCheckpoint();
		}
	}
}

void UPlatformerCheckpointSubsystem::UnregisterCheckpoint(APlatformerCheckpoint* Checkpoint)
{
	if (!Checkpoint)
	{
		return;
	}

	RegisteredCheckpoints.RemoveAll(
		[Checkpoint](const TWeakObjectPtr<APlatformerCheckpoint>& RegisteredCheckpoint)
		{
			return !RegisteredCheckpoint.IsValid() || RegisteredCheckpoint.Get() == Checkpoint;
		});

	if (ActiveCheckpoint.Get() == Checkpoint)
	{
		ActiveCheckpoint.Reset();
	}
}

bool UPlatformerCheckpointSubsystem::ActivateCheckpoint(APlatformerCheckpoint* Checkpoint, AActor* Activator)
{
	if (!IsValid(Checkpoint))
	{
		return false;
	}

	bool bAlreadyRegistered = false;
	for (const TWeakObjectPtr<APlatformerCheckpoint>& RegisteredCheckpoint : RegisteredCheckpoints)
	{
		if (RegisteredCheckpoint.Get() == Checkpoint)
		{
			bAlreadyRegistered = true;
			break;
		}
	}

	if (!bAlreadyRegistered)
	{
		RegisteredCheckpoints.Add(Checkpoint);
	}

	for (const TWeakObjectPtr<APlatformerCheckpoint>& RegisteredCheckpoint : RegisteredCheckpoints)
	{
		if (APlatformerCheckpoint* ExistingCheckpoint = RegisteredCheckpoint.Get())
		{
			ExistingCheckpoint->SetCheckpointActive(ExistingCheckpoint == Checkpoint);
		}
	}

	ActiveCheckpoint = Checkpoint;
	SaveActiveCheckpoint();
	return true;
}

bool UPlatformerCheckpointSubsystem::RestoreActiveCheckpointFromSave()
{
	const UPlatformerGameInstance* PlatformerGameInstance = GetWorld() ? Cast<UPlatformerGameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	const UPlatformerSaveGame* SaveGame = PlatformerGameInstance ? PlatformerGameInstance->GetPlatformerSave() : nullptr;
	if (!SaveGame || SaveGame->LastCheckpoint.LevelName != GetCurrentLevelName())
	{
		return false;
	}

	APlatformerCheckpoint* Checkpoint = FindCheckpointByTag(SaveGame->LastCheckpoint.CheckpointTag);
	if (!Checkpoint)
	{
		Checkpoint = FindCheckpointByActorName(SaveGame->LastCheckpoint.CheckpointActorName);
	}

	if (Checkpoint)
	{
		return ActivateCheckpoint(Checkpoint);
	}

	return false;
}

bool UPlatformerCheckpointSubsystem::RespawnPlayerAtCheckpoint(AController* Controller)
{
	if (!Controller)
	{
		return false;
	}

	APlatformerCheckpoint* Checkpoint = ActiveCheckpoint.Get();
	if (!IsValid(Checkpoint) && !RestoreActiveCheckpointFromSave())
	{
		return false;
	}

	Checkpoint = ActiveCheckpoint.Get();
	if (!IsValid(Checkpoint))
	{
		return false;
	}

	APawn* OldPawn = Controller->GetPawn();
	if (OldPawn)
	{
		Controller->UnPossess();
		OldPawn->Destroy();
	}

	if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this))
	{
		GameMode->RestartPlayerAtTransform(Controller, Checkpoint->GetRespawnTransform());
		return Controller->GetPawn() != nullptr;
	}

	return false;
}

APlatformerCheckpoint* UPlatformerCheckpointSubsystem::FindCheckpointByTag(FGameplayTag CheckpointTag) const
{
	if (!CheckpointTag.IsValid())
	{
		return nullptr;
	}

	for (const TWeakObjectPtr<APlatformerCheckpoint>& RegisteredCheckpoint : RegisteredCheckpoints)
	{
		APlatformerCheckpoint* Checkpoint = RegisteredCheckpoint.Get();
		if (IsValid(Checkpoint) && Checkpoint->CheckpointID == CheckpointTag)
		{
			return Checkpoint;
		}
	}

	return nullptr;
}

APlatformerCheckpoint* UPlatformerCheckpointSubsystem::FindCheckpointByActorName(FName CheckpointActorName) const
{
	if (CheckpointActorName.IsNone())
	{
		return nullptr;
	}

	for (const TWeakObjectPtr<APlatformerCheckpoint>& RegisteredCheckpoint : RegisteredCheckpoints)
	{
		APlatformerCheckpoint* Checkpoint = RegisteredCheckpoint.Get();
		if (IsValid(Checkpoint) && Checkpoint->GetFName() == CheckpointActorName)
		{
			return Checkpoint;
		}
	}

	return nullptr;
}

void UPlatformerCheckpointSubsystem::SaveActiveCheckpoint() const
{
	const APlatformerCheckpoint* Checkpoint = ActiveCheckpoint.Get();
	UPlatformerGameInstance* PlatformerGameInstance = GetWorld() ? Cast<UPlatformerGameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	UPlatformerSaveGame* SaveGame = PlatformerGameInstance ? PlatformerGameInstance->GetPlatformerSave() : nullptr;
	if (!IsValid(Checkpoint) || !SaveGame)
	{
		return;
	}

	SaveGame->bHasStartedGame = true;
	SaveGame->LastCheckpoint.CheckpointTag = Checkpoint->CheckpointID;
	SaveGame->LastCheckpoint.CheckpointActorName = Checkpoint->GetFName();
	SaveGame->LastCheckpoint.LevelName = GetCurrentLevelName();
	PlatformerGameInstance->SaveProgress();
}

bool UPlatformerCheckpointSubsystem::MoveExistingPlayerToActiveCheckpoint() const
{
	const APlatformerCheckpoint* Checkpoint = ActiveCheckpoint.Get();
	if (!IsValid(Checkpoint))
	{
		return false;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return false;
	}

	Pawn->SetActorTransform(Checkpoint->GetRespawnTransform(), false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

FName UPlatformerCheckpointSubsystem::GetCurrentLevelName() const
{
	return FName(*UGameplayStatics::GetCurrentLevelName(this, true));
}
