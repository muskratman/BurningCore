#include "Core/SaveGame/SaveDeveloperSettings.h"

#include "Kismet/GameplayStatics.h"

namespace
{
const FString DeveloperSaveSlotName = TEXT("SaveDeveloperSettings_1");
constexpr int32 DeveloperSaveUserIndex = 0;
}

USaveDeveloperSettings* USaveDeveloperSettings::LoadDeveloperSettingsFromSlot(const UObject* DeveloperWorldContextObject)
{
	(void)DeveloperWorldContextObject;

	if (!UGameplayStatics::DoesSaveGameExist(DeveloperSaveSlotName, DeveloperSaveUserIndex))
	{
		return nullptr;
	}

	return Cast<USaveDeveloperSettings>(UGameplayStatics::LoadGameFromSlot(DeveloperSaveSlotName, DeveloperSaveUserIndex));
}

USaveDeveloperSettings* USaveDeveloperSettings::LoadOrCreateDeveloperSettings(const UObject* DeveloperWorldContextObject)
{
	if (USaveDeveloperSettings* LoadedDeveloperSave = LoadDeveloperSettingsFromSlot(DeveloperWorldContextObject))
	{
		return LoadedDeveloperSave;
	}

	return Cast<USaveDeveloperSettings>(UGameplayStatics::CreateSaveGameObject(StaticClass()));
}

bool USaveDeveloperSettings::WriteDeveloperSettingsToSlot(const UObject* DeveloperWorldContextObject, USaveDeveloperSettings* DeveloperSaveObject)
{
	(void)DeveloperWorldContextObject;
	return DeveloperSaveObject != nullptr
		&& UGameplayStatics::SaveGameToSlot(DeveloperSaveObject, DeveloperSaveSlotName, DeveloperSaveUserIndex);
}
