// Copyright CookieBros. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Modules/ModuleManager.h"

/**
 * FCookieBrosLevelEditorModule
 * Editor-only module that exposes the Paper TileMap -> Level import tooling.
 */
class FCookieBrosLevelEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OpenPaperTileMapImportTab();
	void OpenPlatformerSettingsTab();
	TSharedRef<class SDockTab> SpawnPaperTileMapImportTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<class SDockTab> SpawnPlatformerSettingsTab(const class FSpawnTabArgs& SpawnTabArgs);
	bool HandleDeferredCommandLineImport(float DeltaTime);

	FTSTicker::FDelegateHandle DeferredImportTickerHandle;
};
