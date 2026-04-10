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
	void SetSnapshot(const FPlatformerDeveloperSettingsSnapshot& InSnapshot);
	FPlatformerDeveloperSettingsSnapshot ResolveSnapshot() const;
	bool UsesSnapshotData() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	int32 DataVersion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FGuid SlotId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FPlatformerDeveloperSettingsSnapshot Snapshot;

	// Legacy single-slot fields kept to migrate older developer save payloads.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCharacterSettings DeveloperCharacterSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bHasSavedDeveloperCombatSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	bool bAutoRestartLevel = false;
};
