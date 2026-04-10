#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "PlatformerTraversalConfigDataAsset.generated.h"

/**
 * Data-driven tuning asset for platformer traversal states.
 */
UCLASS(BlueprintType)
class COOKIEBROSPLATFORMER_API UPlatformerTraversalConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal")
	FPlatformerLedgeTraversalSettings LedgeSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal")
	FPlatformerSlideDashSettings SlideDashSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Traversal")
	FPlatformerWallTraversalSettings WallSettings;
};
