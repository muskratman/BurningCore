#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PlatformerInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerInputActionBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> InputAction;
};

/**
 * Data-driven Enhanced Input setup for reusable platformer characters.
 * Input tags cover movement/custom project glue and route ability input
 * actions to granted GAS abilities through Input.Ability.* tags.
 */
UCLASS(BlueprintType)
class COOKIEBROSPLATFORMER_API UPlatformerInputConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	int32 MappingContextPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<FPlatformerInputActionBinding> InputBindings;

	UInputAction* FindInputAction(const FGameplayTag& InputTag) const;
};
