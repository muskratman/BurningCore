#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PlatformerEnemyAnimDataAsset.generated.h"

class UAnimMontage;

/**
 * Single enemy animation entry mapping a GameplayTag to an AnimMontage.
 */
USTRUCT(BlueprintType)
struct FPlatformerEnemyAnimEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation", meta=(Categories="Anim.Enemy"))
	FGameplayTag AnimTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation", meta=(ClampMin="0.01"))
	float DefaultPlayRate = 1.0f;
};

/**
 * Data-driven enemy animation mapping.
 *
 * Enemy animation is intentionally separate from playable-character animation:
 * platformer enemies are commonly scripted around compact movement/combat states,
 * while player characters tend to carry richer traversal and input-driven state.
 */
UCLASS(BlueprintType)
class COOKIEBROSPLATFORMER_API UPlatformerEnemyAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy Animations")
	TArray<FPlatformerEnemyAnimEntry> EnemyAnimations;

	const FPlatformerEnemyAnimEntry* FindAnimEntry(const FGameplayTag& AnimTag) const;
	UAnimMontage* FindMontage(const FGameplayTag& AnimTag) const;
};
