#include "AI/Data/PlatformerEnemyAnimDataAsset.h"

const FPlatformerEnemyAnimEntry* UPlatformerEnemyAnimDataAsset::FindAnimEntry(const FGameplayTag& AnimTag) const
{
	if (!AnimTag.IsValid())
	{
		return nullptr;
	}

	for (const FPlatformerEnemyAnimEntry& Entry : EnemyAnimations)
	{
		if (Entry.AnimTag == AnimTag)
		{
			return &Entry;
		}
	}

	return nullptr;
}

UAnimMontage* UPlatformerEnemyAnimDataAsset::FindMontage(const FGameplayTag& AnimTag) const
{
	const FPlatformerEnemyAnimEntry* Entry = FindAnimEntry(AnimTag);
	return Entry ? Entry->Montage.Get() : nullptr;
}
