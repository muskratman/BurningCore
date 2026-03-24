#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "DragonAbilitySet.generated.h"

class UGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDragonAbilitySet_Ability
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;
};

USTRUCT(BlueprintType)
struct FDragonAbilitySet_Effect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 * UDragonAbilitySet
 * DataAsset that holds a list of abilities and effects to grant to a character.
 */
UCLASS()
class BURNINGCORE_API UDragonAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Abilities")
	TArray<FDragonAbilitySet_Ability> Abilities;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TArray<FDragonAbilitySet_Effect> Effects;
};
