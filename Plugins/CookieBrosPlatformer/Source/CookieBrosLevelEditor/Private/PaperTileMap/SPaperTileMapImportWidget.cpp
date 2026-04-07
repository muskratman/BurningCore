// Copyright CookieBros. All Rights Reserved.

#include "PaperTileMap/SPaperTileMapImportWidget.h"

#include "AssetRegistry/AssetData.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "ObjectTools.h"
#include "PaperTileMap.h"
#include "PaperTileMap/PaperTileMapLevelImporter.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/AppStyle.h"
#include "TileMap/TileSetAsset.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SPaperTileMapImportWidget"

namespace
{
	static const FString GeneratedLevelsRoot(TEXT("/Game/PlatformerLevels"));
	static const TCHAR* DefaultImportMappingObjectPath = TEXT("/CookieBrosPlatformer/TilesData/DA_PlatformerTilesSet.DA_PlatformerTilesSet");
	static const TCHAR* PersistedSettingsSection = TEXT("CookieBrosLevelEditor.PaperTileMapImport");
	static const TCHAR* PersistedTileMapKey = TEXT("LastSelectedTileMap");
	static const TCHAR* PersistedBlockSizeXKey = TEXT("LastBlockSizeX");
	static const TCHAR* PersistedBlockSizeYKey = TEXT("LastBlockSizeY");
	static const TCHAR* PersistedBlockSizeZKey = TEXT("LastBlockSizeZ");
	static const TCHAR* PersistedGameModeKey = TEXT("LastGameModeOverride");
}

void SPaperTileMapImportWidget::Construct(const FArguments& InArgs)
{
	SelectedImportMappingAsset = LoadObject<UTileSetAsset>(nullptr, DefaultImportMappingObjectPath);
	LoadPersistedSettings();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 8.0f, 8.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Header", "Paper TileMap Import"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Description", "Generate a .umap in /Game/PlatformerLevels from a Paper TileMap using a Paper Tile import mapping asset, explicit block sizing, a lighting setup actor, and an optional GameMode override."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SelectTileMapLabel", "Select Tile Map"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UPaperTileMap::StaticClass())
			.ObjectPath_Lambda([this]() -> FString
			{
				return SelectedPaperTileMap.IsValid() ? SelectedPaperTileMap->GetPathName() : FString();
			})
			.OnObjectChanged_Lambda([this](const FAssetData& AssetData)
			{
				HandleSelectedTileMapChanged(AssetData);
			})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlockSizeLabel", "Block Size (X, Y, Z)"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SNumericEntryBox<float>)
				.LabelVAlign(VAlign_Center)
				.Label()
				[
					SNew(STextBlock).Text(LOCTEXT("BlockSizeXLabel", "X"))
				]
				.MinValue(1.0f)
				.AllowSpin(true)
				.Value_Lambda([this]() -> TOptional<float>
				{
					return ConfiguredBlockSize.X;
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					ConfiguredBlockSize.X = FMath::Max(NewValue, 1.0f);
				})
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SNumericEntryBox<float>)
				.LabelVAlign(VAlign_Center)
				.Label()
				[
					SNew(STextBlock).Text(LOCTEXT("BlockSizeYLabel", "Y"))
				]
				.MinValue(1.0f)
				.AllowSpin(true)
				.Value_Lambda([this]() -> TOptional<float>
				{
					return ConfiguredBlockSize.Y;
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					ConfiguredBlockSize.Y = FMath::Max(NewValue, 1.0f);
				})
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SNumericEntryBox<float>)
				.LabelVAlign(VAlign_Center)
				.Label()
				[
					SNew(STextBlock).Text(LOCTEXT("BlockSizeZLabel", "Z"))
				]
				.MinValue(1.0f)
				.AllowSpin(true)
				.Value_Lambda([this]() -> TOptional<float>
				{
					return ConfiguredBlockSize.Z;
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					ConfiguredBlockSize.Z = FMath::Max(NewValue, 1.0f);
				})
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlockSizeHelp", "Blocks and ramps use BlockSize.X/Z for layout and size. Their depth on Y stays fixed at 500. Ladder placement uses BlockSize.Y as the rear depth offset."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ImportMappingLabel", "Import Mapping"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UTileSetAsset::StaticClass())
			.ObjectPath_Lambda([this]() -> FString
			{
				return SelectedImportMappingAsset.IsValid() ? SelectedImportMappingAsset->GetPathName() : FString();
			})
			.OnObjectChanged_Lambda([this](const FAssetData& AssetData)
			{
				SelectedImportMappingAsset = Cast<UTileSetAsset>(AssetData.GetAsset());
			})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GameModeLabel", "GameMode Override"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SClassPropertyEntryBox)
			.AllowNone(true)
			.MetaClass(AGameModeBase::StaticClass())
			.SelectedClass(this, &SPaperTileMapImportWidget::GetSelectedGameModeClass)
			.OnSetClass(FOnSetClass::CreateSP(this, &SPaperTileMapImportWidget::HandleSelectedGameModeClass))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CreatedLevelNameLabel", "Created Level Name"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("CreatedLevelNameHint", "Will match the selected TileMap name"))
			.IsReadOnly(true)
			.Text_Lambda([this]() -> FText
			{
				return FText::FromString(CreatedLevelName);
			})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				const FString OutputLevelPackagePath = BuildOutputLevelPackagePath();
				return OutputLevelPackagePath.IsEmpty()
					? LOCTEXT("OutputLevelPathEmpty", "Output: /Game/PlatformerLevels/<LevelName>")
					: FText::Format(
						LOCTEXT("OutputLevelPathValue", "Output: {0}"),
						FText::FromString(OutputLevelPackagePath));
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("GenerateLevelButton", "Generate Level"))
			.IsEnabled_Lambda([this]()
			{
				return CanGenerateLevel();
			})
			.OnClicked_Lambda([this]()
			{
				return HandleGenerateLevelClicked();
			})
		]
	];
}

void SPaperTileMapImportWidget::HandleSelectedTileMapChanged(const FAssetData& AssetData)
{
	SelectedPaperTileMap = Cast<UPaperTileMap>(AssetData.GetAsset());
	ApplyLevelNameFromSelectedTileMap();
}

void SPaperTileMapImportWidget::ApplyLevelNameFromSelectedTileMap()
{
	CreatedLevelName = SelectedPaperTileMap.IsValid() ? SelectedPaperTileMap->GetName() : FString();
}

void SPaperTileMapImportWidget::LoadPersistedSettings()
{
	FString PersistedTileMapPath;
	if (GConfig->GetString(PersistedSettingsSection, PersistedTileMapKey, PersistedTileMapPath, GEditorPerProjectIni)
		&& !PersistedTileMapPath.IsEmpty())
	{
		SelectedPaperTileMap = LoadObject<UPaperTileMap>(nullptr, *PersistedTileMapPath);
	}

	float PersistedValue = 0.0f;
	if (GConfig->GetFloat(PersistedSettingsSection, PersistedBlockSizeXKey, PersistedValue, GEditorPerProjectIni))
	{
		ConfiguredBlockSize.X = FMath::Max(PersistedValue, 1.0f);
	}

	if (GConfig->GetFloat(PersistedSettingsSection, PersistedBlockSizeYKey, PersistedValue, GEditorPerProjectIni))
	{
		ConfiguredBlockSize.Y = FMath::Max(PersistedValue, 1.0f);
	}

	if (GConfig->GetFloat(PersistedSettingsSection, PersistedBlockSizeZKey, PersistedValue, GEditorPerProjectIni))
	{
		ConfiguredBlockSize.Z = FMath::Max(PersistedValue, 1.0f);
	}

	FString PersistedGameModePath;
	if (GConfig->GetString(PersistedSettingsSection, PersistedGameModeKey, PersistedGameModePath, GEditorPerProjectIni)
		&& !PersistedGameModePath.IsEmpty())
	{
		SelectedGameModeClass = LoadClass<AGameModeBase>(nullptr, *PersistedGameModePath);
	}

	ApplyLevelNameFromSelectedTileMap();
}

void SPaperTileMapImportWidget::SavePersistedSettings() const
{
	const FString TileMapPath = SelectedPaperTileMap.IsValid() ? SelectedPaperTileMap->GetPathName() : FString();
	const FString GameModePath = SelectedGameModeClass ? SelectedGameModeClass->GetPathName() : FString();

	GConfig->SetString(PersistedSettingsSection, PersistedTileMapKey, *TileMapPath, GEditorPerProjectIni);
	GConfig->SetFloat(PersistedSettingsSection, PersistedBlockSizeXKey, ConfiguredBlockSize.X, GEditorPerProjectIni);
	GConfig->SetFloat(PersistedSettingsSection, PersistedBlockSizeYKey, ConfiguredBlockSize.Y, GEditorPerProjectIni);
	GConfig->SetFloat(PersistedSettingsSection, PersistedBlockSizeZKey, ConfiguredBlockSize.Z, GEditorPerProjectIni);
	GConfig->SetString(PersistedSettingsSection, PersistedGameModeKey, *GameModePath, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

FPaperTileMapGenerationSettings SPaperTileMapImportWidget::BuildGenerationSettings() const
{
	FPaperTileMapGenerationSettings Settings;
	Settings.BlockSize = ConfiguredBlockSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	Settings.GameModeOverrideClass = SelectedGameModeClass;
	return Settings;
}

const UClass* SPaperTileMapImportWidget::GetSelectedGameModeClass() const
{
	return SelectedGameModeClass.Get();
}

void SPaperTileMapImportWidget::HandleSelectedGameModeClass(const UClass* InClass)
{
	SelectedGameModeClass = const_cast<UClass*>(InClass);
}

FString SPaperTileMapImportWidget::BuildOutputLevelPackagePath() const
{
	FString EffectiveLevelName = CreatedLevelName.TrimStartAndEnd();
	if (EffectiveLevelName.IsEmpty() && SelectedPaperTileMap.IsValid())
	{
		EffectiveLevelName = SelectedPaperTileMap->GetName();
	}

	const FString SanitizedLevelName = ObjectTools::SanitizeObjectName(EffectiveLevelName);
	if (SanitizedLevelName.IsEmpty())
	{
		return FString();
	}

	return FString::Printf(TEXT("%s/%s"), *GeneratedLevelsRoot, *SanitizedLevelName);
}

FReply SPaperTileMapImportWidget::HandleGenerateLevelClicked()
{
	if (!SelectedPaperTileMap.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MissingTileMapForGeneration", "Select a PaperTileMap before generating a level."));
		return FReply::Handled();
	}

	if (!SelectedImportMappingAsset.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MissingImportMappingForGeneration", "Select an Import Mapping asset before generating a level."));
		return FReply::Handled();
	}

	const FString OutputLevelPackagePath = BuildOutputLevelPackagePath();
	if (OutputLevelPackagePath.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MissingLevelNameForGeneration", "Select a valid TileMap so the generated level name can be resolved."));
		return FReply::Handled();
	}

	FString ErrorMessage;
	if (!FCookieBrosPaperTileMapLevelImporter::GenerateLevelFromTileMapAsset(
			SelectedPaperTileMap.Get(),
			SelectedImportMappingAsset.Get(),
			OutputLevelPackagePath,
			BuildGenerationSettings(),
			ErrorMessage))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("TileMapLevelGenerationFailed", "Failed to generate the level.\n\n{0}"),
				FText::FromString(ErrorMessage)));
		return FReply::Handled();
	}

	FMessageDialog::Open(
		EAppMsgType::Ok,
		FText::Format(
			LOCTEXT("TileMapLevelGenerationSucceeded", "Level generated successfully:\n\n{0}"),
			FText::FromString(OutputLevelPackagePath)));

	SavePersistedSettings();

	return FReply::Handled();
}

bool SPaperTileMapImportWidget::CanGenerateLevel() const
{
	return SelectedPaperTileMap.IsValid() && SelectedImportMappingAsset.IsValid() && !BuildOutputLevelPackagePath().IsEmpty();
}

#undef LOCTEXT_NAMESPACE
