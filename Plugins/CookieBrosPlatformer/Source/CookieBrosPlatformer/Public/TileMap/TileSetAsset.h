// Copyright CookieBros. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TileSetAsset.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPaperTileImportRule
{
	GENERATED_BODY()

	/** UserDataName read from the source Paper2D TileSet tile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Import")
	FName UserDataName;

	/** Environment actor to spawn when this UserDataName is encountered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Import")
	TSubclassOf<AActor> ImportActorClass;
};

/**
 * UTileSetAsset
 * Legacy-named DataAsset used as a PaperTileMap import mapping.
 * Maps Paper2D TileSet UserDataName values to environment actor classes.
 */
UCLASS(BlueprintType, meta=(DisplayName="Paper Tile Import Mapping"))
class COOKIEBROSPLATFORMER_API UTileSetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Import rules evaluated against Paper2D TileSet UserDataName values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Import")
	TArray<FPaperTileImportRule> Rules;

	// ----- Lookup helpers -----

	/** Find an import rule by normalized Paper2D TileSet UserDataName. */
	const FPaperTileImportRule* FindRuleByUserDataName(FName UserDataName) const;

	/** Get all configured UserDataName values. */
	UFUNCTION(BlueprintPure, Category="Import")
	TArray<FName> GetAllUserDataNames() const;

	/** Check whether a given Paper2D UserDataName exists in the mapping. */
	UFUNCTION(BlueprintPure, Category="Import")
	bool ContainsUserDataName(FName UserDataName) const;

	// ----- UPrimaryDataAsset overrides -----
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
