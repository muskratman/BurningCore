#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"

class AGameModeBase;
class UPaperTileMap;
class UTileSetAsset;

struct FPaperTileMapGenerationSettings
{
	FVector BlockSize = FVector(100.0f, 100.0f, 100.0f);
	TSubclassOf<AGameModeBase> GameModeOverrideClass;
};

class FCookieBrosPaperTileMapLevelImporter
{
public:
	static bool HasCommandLineRequest();
	static bool RunFromCommandLine();

	static bool GenerateLevelFromTileMapAsset(
		UPaperTileMap* TileMap,
		const UTileSetAsset* ImportMappingAsset,
		const FString& OutputLevelPackagePath,
		const FPaperTileMapGenerationSettings& GenerationSettings,
		FString& OutError);
};
