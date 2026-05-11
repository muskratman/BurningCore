#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlatformerCheckpointRegistry.generated.h"

class APlatformerCheckpoint;

UINTERFACE(MinimalAPI)
class UPlatformerCheckpointRegistry : public UInterface
{
	GENERATED_BODY()
};

/**
 * IPlatformerCheckpointRegistry
 * Service contract for runtime owners that track the active checkpoint.
 */
class COOKIEBROSPLATFORMER_API IPlatformerCheckpointRegistry
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Checkpoint")
	void RegisterCheckpoint(APlatformerCheckpoint* Checkpoint);
};
