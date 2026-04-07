#pragma once

#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "SaveDeveloperSettings.generated.h"

UCLASS()
class COOKIEBROSPLATFORMER_API USaveDeveloperSettings : public USaveGame
{
	GENERATED_BODY()

public:
	static USaveDeveloperSettings* LoadDeveloperSettingsFromSlot(const UObject* DeveloperWorldContextObject);
	static USaveDeveloperSettings* LoadOrCreateDeveloperSettings(const UObject* DeveloperWorldContextObject);
	static bool WriteDeveloperSettingsToSlot(const UObject* DeveloperWorldContextObject, USaveDeveloperSettings* DeveloperSaveObject);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCharacterSettings DeveloperCharacterSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedDeveloperCombatSettings = false;
};
