// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/PlatformerGameInstance.h"

#include "Core/PlatformerGameModeBase.h"
#include "Core/SaveGame/PlatformerSaveGame.h"
#include "Core/SaveGame/PlatformerSaveGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Platformer/Environment/PlatformerCheckpoint.h"

UPlatformerGameInstance::UPlatformerGameInstance()
{
	PlatformerSaveClass = UPlatformerSaveGame::StaticClass();
	SettingsSaveClass = UPlatformerSaveGameSettings::StaticClass();
}

void UPlatformerGameInstance::Init()
{
	Super::Init();

	bGameInstanceSetupCompleted = InitializeSaveGames();
}

void UPlatformerGameInstance::OnStart()
{
	Super::OnStart();

	if (!bGameInstanceSetupCompleted || bGameInstanceSetupBroadcasted)
	{
		return;
	}

	bGameInstanceSetupBroadcasted = true;
	OnGameInstanceSetuped();
}

bool UPlatformerGameInstance::SavePlatformerProgress_Implementation()
{
	if (CurrentSave)
	{
		CurrentSave->bHasStartedGame = true;
	}

	return SaveProgress();
}

bool UPlatformerGameInstance::SaveProgress()
{
	if (!CurrentSave)
	{
		return false;
	}

	CapturePlatformerProgress(CurrentSave);
	return UGameplayStatics::SaveGameToSlot(CurrentSave.Get(), SaveSlotName, SaveUserIndex);
}

bool UPlatformerGameInstance::LoadProgress(int32 SlotIndex)
{
	const int32 EffectiveUserIndex = SlotIndex >= 0 ? SlotIndex : SaveUserIndex;
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, EffectiveUserIndex))
	{
		return false;
	}

	UPlatformerSaveGame* LoadedSave = Cast<UPlatformerSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, EffectiveUserIndex));
	if (!ValidateLoadedPlatformerSave(LoadedSave))
	{
		CurrentSave = nullptr;
		return false;
	}

	CurrentSave = LoadedSave;
	return true;
}

bool UPlatformerGameInstance::SaveSettings()
{
	if (!SettingsSave)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(SettingsSave.Get(), SettingsSlotName, SaveUserIndex);
}

bool UPlatformerGameInstance::LoadSettings(int32 SlotIndex)
{
	const int32 EffectiveUserIndex = SlotIndex >= 0 ? SlotIndex : SaveUserIndex;
	if (!UGameplayStatics::DoesSaveGameExist(SettingsSlotName, EffectiveUserIndex))
	{
		return false;
	}

	SettingsSave = Cast<UPlatformerSaveGameSettings>(UGameplayStatics::LoadGameFromSlot(SettingsSlotName, EffectiveUserIndex));
	return SettingsSave != nullptr;
}

void UPlatformerGameInstance::OnGameInstanceSetuped_Implementation()
{
}

void UPlatformerGameInstance::CapturePlatformerProgress(UPlatformerSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return;
	}

	const APlatformerGameModeBase* PlatformerGameMode = Cast<APlatformerGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!PlatformerGameMode)
	{
		return;
	}

	const APlatformerCheckpoint* LastCheckpoint = PlatformerGameMode->GetLastCheckpoint();
	if (!IsValid(LastCheckpoint))
	{
		return;
	}

	SaveGame->LastCheckpoint.CheckpointTag = LastCheckpoint->CheckpointID;
	SaveGame->LastCheckpoint.CheckpointActorName = LastCheckpoint->GetFName();
	SaveGame->LastCheckpoint.LevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
}

bool UPlatformerGameInstance::ValidateLoadedPlatformerSave(UPlatformerSaveGame* SaveGame) const
{
	return SaveGame != nullptr;
}

bool UPlatformerGameInstance::InitializeSaveGames()
{
	return EnsureSettingsSave(SaveUserIndex) && EnsureProgressSave(SaveUserIndex);
}

bool UPlatformerGameInstance::EnsureProgressSave(int32 SlotIndex)
{
	if (LoadProgress(SlotIndex))
	{
		return true;
	}

	if (!PlatformerSaveClass)
	{
		return false;
	}

	CurrentSave = Cast<UPlatformerSaveGame>(UGameplayStatics::CreateSaveGameObject(PlatformerSaveClass));
	return CurrentSave != nullptr && SaveProgress();
}

bool UPlatformerGameInstance::EnsureSettingsSave(int32 SlotIndex)
{
	if (LoadSettings(SlotIndex))
	{
		return true;
	}

	if (!SettingsSaveClass)
	{
		return false;
	}

	SettingsSave = Cast<UPlatformerSaveGameSettings>(UGameplayStatics::CreateSaveGameObject(SettingsSaveClass));
	return SettingsSave != nullptr && SaveSettings();
}
