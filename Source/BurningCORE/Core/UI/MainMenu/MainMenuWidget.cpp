#include "MainMenuWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);
	}

	PopulateLevels();
}

void UMainMenuWidget::OnStartGameClicked()
{
	if (LevelComboBox && LevelComboBox->GetSelectedOption().Len() > 0)
	{
		FString SelectedLevel = LevelComboBox->GetSelectedOption();
		UGameplayStatics::OpenLevel(this, FName(*SelectedLevel));
	}
}

void UMainMenuWidget::PopulateLevels()
{
	if (!LevelComboBox)
	{
		return;
	}

	LevelComboBox->ClearOptions();

	// Find all map files in the levels directory
	FString LevelsDir = FPaths::ProjectContentDir() / TEXT("Levels");
	TArray<FString> MapFiles;
	
	IFileManager::Get().FindFilesRecursive(MapFiles, *LevelsDir, TEXT("*.umap"), true, false, false);

	for (const FString& MapFile : MapFiles)
	{
		FString MapName = FPaths::GetBaseFilename(MapFile);
		LevelComboBox->AddOption(MapName);
	}

	if (LevelComboBox->GetOptionCount() > 0)
	{
		LevelComboBox->SetSelectedIndex(0);
	}
}
