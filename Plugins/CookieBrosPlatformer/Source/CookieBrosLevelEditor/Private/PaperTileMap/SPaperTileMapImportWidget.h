// Copyright CookieBros. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperTileMap/PaperTileMapLevelImporter.h"
#include "Widgets/SCompoundWidget.h"

class AGameModeBase;
class UPaperTileMap;
class UTileSetAsset;
struct FAssetData;

class SPaperTileMapImportWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperTileMapImportWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void HandleSelectedTileMapChanged(const FAssetData& AssetData);
	void ApplyLevelNameFromSelectedTileMap();
	void LoadPersistedSettings();
	void SavePersistedSettings() const;
	FPaperTileMapGenerationSettings BuildGenerationSettings() const;
	const UClass* GetSelectedGameModeClass() const;
	void HandleSelectedGameModeClass(const UClass* InClass);
	FString BuildOutputLevelPackagePath() const;
	FReply HandleGenerateLevelClicked();
	bool CanGenerateLevel() const;

	TWeakObjectPtr<UPaperTileMap> SelectedPaperTileMap;
	TWeakObjectPtr<UTileSetAsset> SelectedImportMappingAsset;
	TSubclassOf<AGameModeBase> SelectedGameModeClass;
	FVector ConfiguredBlockSize = FVector(100.0f, 100.0f, 100.0f);
	FString CreatedLevelName;
};
