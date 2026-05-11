// Copyright CookieBros. All Rights Reserved.

#include "CookieBrosLevelEditor.h"

#include "Framework/Docking/TabManager.h"
#include "PaperTileMap/PaperTileMapLevelImporter.h"
#include "PlatformerSettings/SPlatformerSettingsWidget.h"
#include "PaperTileMap/SPaperTileMapImportWidget.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Visualizers/PlatformerPathComponentVisualizer.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FCookieBrosLevelEditorModule"

namespace
{
	static const FName PaperTileMapImportTabName(TEXT("CookieBros.PaperTileMapImport"));
	static const FName PlatformerSettingsTabName(TEXT("CookieBros.PlatformerSettings"));
}

void FCookieBrosLevelEditorModule::StartupModule()
{
	RegisterComponentVisualizers();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		PaperTileMapImportTabName,
		FOnSpawnTab::CreateRaw(this, &FCookieBrosLevelEditorModule::SpawnPaperTileMapImportTab))
		.SetDisplayName(LOCTEXT("PaperTileMapImportTabTitle", "Paper TileMap Import"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.PaperTileMap"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		PlatformerSettingsTabName,
		FOnSpawnTab::CreateRaw(this, &FCookieBrosLevelEditorModule::SpawnPlatformerSettingsTab))
		.SetDisplayName(LOCTEXT("PlatformerSettingsTabTitle", "Platformer Settings"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCookieBrosLevelEditorModule::RegisterMenus));

	if (FCookieBrosPaperTileMapLevelImporter::HasCommandLineRequest())
	{
		DeferredImportTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FCookieBrosLevelEditorModule::HandleDeferredCommandLineImport));
	}
}

void FCookieBrosLevelEditorModule::ShutdownModule()
{
	UnregisterComponentVisualizers();

	if (DeferredImportTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DeferredImportTickerHandle);
		DeferredImportTickerHandle.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PaperTileMapImportTabName);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PlatformerSettingsTabName);
	}
}

void FCookieBrosLevelEditorModule::RegisterComponentVisualizers()
{
	if (!GUnrealEd)
	{
		return;
	}

	const FName PathComponentClassName = UPlatformerPathComponent::StaticClass()->GetFName();
	TSharedPtr<FComponentVisualizer> PathVisualizer = MakeShared<FPlatformerPathComponentVisualizer>();
	GUnrealEd->RegisterComponentVisualizer(PathComponentClassName, PathVisualizer);
	PathVisualizer->OnRegister();
	RegisteredComponentVisualizerNames.Add(PathComponentClassName);
}

void FCookieBrosLevelEditorModule::UnregisterComponentVisualizers()
{
	if (!GUnrealEd)
	{
		RegisteredComponentVisualizerNames.Reset();
		return;
	}

	for (const FName ComponentClassName : RegisteredComponentVisualizerNames)
	{
		GUnrealEd->UnregisterComponentVisualizer(ComponentClassName);
	}
	RegisteredComponentVisualizerNames.Reset();
}

void FCookieBrosLevelEditorModule::RegisterMenus()
{
	UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");

	Section.AddMenuEntry(
		"CookieBrosPaperTileMapImport",
		LOCTEXT("OpenPaperTileMapImportLabel", "Paper TileMap Import"),
		LOCTEXT("OpenPaperTileMapImportTooltip", "Open the Paper TileMap to Level import panel."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.PaperTileMap"),
		FUIAction(FExecuteAction::CreateRaw(this, &FCookieBrosLevelEditorModule::OpenPaperTileMapImportTab)));

	Section.AddMenuEntry(
		"CookieBrosPlatformerSettings",
		LOCTEXT("OpenPlatformerSettingsLabel", "Platformer Settings"),
		LOCTEXT("OpenPlatformerSettingsTooltip", "Open quick settings for selected Platformer actors."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings"),
		FUIAction(FExecuteAction::CreateRaw(this, &FCookieBrosLevelEditorModule::OpenPlatformerSettingsTab)));
}

void FCookieBrosLevelEditorModule::OpenPaperTileMapImportTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(PaperTileMapImportTabName);
}

void FCookieBrosLevelEditorModule::OpenPlatformerSettingsTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(PlatformerSettingsTabName);
}

TSharedRef<SDockTab> FCookieBrosLevelEditorModule::SpawnPaperTileMapImportTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("PaperTileMapImportTabLabel", "Paper TileMap Import"))
		[
			SNew(SPaperTileMapImportWidget)
		];
}

TSharedRef<SDockTab> FCookieBrosLevelEditorModule::SpawnPlatformerSettingsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("PlatformerSettingsTabLabel", "Platformer Settings"))
		[
			SNew(SPlatformerSettingsWidget)
		];
}

bool FCookieBrosLevelEditorModule::HandleDeferredCommandLineImport(float /*DeltaTime*/)
{
	if ((GEditor == nullptr) || (GEditor->GetEditorWorldContext().World() == nullptr))
	{
		return true;
	}

	DeferredImportTickerHandle.Reset();
	FCookieBrosPaperTileMapLevelImporter::RunFromCommandLine();
	return false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCookieBrosLevelEditorModule, CookieBrosLevelEditor)
