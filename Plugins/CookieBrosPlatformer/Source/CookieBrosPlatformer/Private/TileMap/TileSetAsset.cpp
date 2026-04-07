// Copyright CookieBros. All Rights Reserved.

#include "TileMap/TileSetAsset.h"

namespace
{
	FString NormalizePaperTileImportName(FName InName)
	{
		FString Normalized = InName.ToString().ToLower();
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));
		Normalized.ReplaceInline(TEXT(" "), TEXT(""));
		return Normalized;
	}
}

const FPaperTileImportRule* UTileSetAsset::FindRuleByUserDataName(FName UserDataName) const
{
	const FString NormalizedUserDataName = NormalizePaperTileImportName(UserDataName);
	if (NormalizedUserDataName.IsEmpty())
	{
		return nullptr;
	}

	return Rules.FindByPredicate([&NormalizedUserDataName](const FPaperTileImportRule& Rule)
	{
		return NormalizePaperTileImportName(Rule.UserDataName) == NormalizedUserDataName;
	});
}

TArray<FName> UTileSetAsset::GetAllUserDataNames() const
{
	TArray<FName> Names;
	Names.Reserve(Rules.Num());
	for (const FPaperTileImportRule& Rule : Rules)
	{
		Names.Add(Rule.UserDataName);
	}
	return Names;
}

bool UTileSetAsset::ContainsUserDataName(FName UserDataName) const
{
	return FindRuleByUserDataName(UserDataName) != nullptr;
}

FPrimaryAssetId UTileSetAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("TileSet"), GetFName());
}

#if WITH_EDITOR
void UTileSetAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	TMap<FString, FName> SeenImportNames;
	for (int32 RuleIndex = 0; RuleIndex < Rules.Num(); ++RuleIndex)
	{
		const FPaperTileImportRule& Rule = Rules[RuleIndex];
		if (Rule.UserDataName.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("UTileSetAsset: Import rule at index %d has an empty UserDataName."), RuleIndex);
			continue;
		}

		const FString NormalizedName = NormalizePaperTileImportName(Rule.UserDataName);
		if (NormalizedName.IsEmpty())
		{
			continue;
		}

		if (const FName* ExistingUserDataName = SeenImportNames.Find(NormalizedName))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("UTileSetAsset: Duplicate PaperTile import UserDataName '%s' on rules '%s' and '%s'."),
				*Rule.UserDataName.ToString(),
				*ExistingUserDataName->ToString(),
				*Rule.UserDataName.ToString());
		}
		else
		{
			SeenImportNames.Add(NormalizedName, Rule.UserDataName);
		}

		if (!Rule.ImportActorClass)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("UTileSetAsset: Import rule '%s' has no ImportActorClass assigned."),
				*Rule.UserDataName.ToString());
		}
	}
}
#endif
