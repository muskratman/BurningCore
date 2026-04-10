#pragma once

#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "SaveDeveloperSettingsSlotIndex.generated.h"

UCLASS()
class COOKIEBROSPLATFORMER_API USaveDeveloperSettingsSlotIndex : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	int32 DataVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	TArray<FPlatformerDeveloperSettingsSlotDescriptor> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Developer")
	FGuid CurrentSlotId;
};
